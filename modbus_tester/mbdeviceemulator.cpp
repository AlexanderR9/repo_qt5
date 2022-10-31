#include "mbdeviceemulator.h"
#include "lmath.h"

#include <QDebug>
#include <QTimer>
#include <QModbusDataUnit>

#define UPDATE_VALUES_INTERVAL      3333
#define UPDATE_CHANCE_FACTOR        0.5  //вероятность того, что сигнал попытается обновиться


/////////////////////MBDeviceEmulator//////////////////////////////
MBDeviceEmulator::MBDeviceEmulator(QObject *parent)
    :QObject(parent),
    m_timer(NULL),
    m_firstValues(false)
{
    setObjectName("mb_sacor_emulator");
    reset();

    m_timer = new QTimer(this);
    m_timer->setInterval(UPDATE_VALUES_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void MBDeviceEmulator::slotTimer()
{
    qDebug("update emulation values");
    QMap<quint16, quint16> need_update_regs;
    QMap<quint8, MBEmulDevice>::iterator it = m_devices.begin();
    while (it != m_devices.end())
    {
        it.value().updateSignals(need_update_regs, m_firstValues);
        it++;
    }

    m_firstValues = false;
    if (need_update_regs.isEmpty()) return;

    QMap<quint16, quint16>::const_iterator it2 = need_update_regs.constBegin();
    while (it2 != need_update_regs.constEnd())
    {
        qDebug()<<QString("emit signal to rewrite register pos=%1  value=%2").arg(it2.key()).arg(it2.value());
        emit signalSetRegisterValue(QModbusDataUnit::HoldingRegisters, it2.key(), it2.value());
        it2++;
    }
}
void MBDeviceEmulator::startEmulation()
{
    if (m_timer)
        if (!m_timer->isActive())
        {
            m_timer->start();
            //m_firstValues = true;
        }
}
void MBDeviceEmulator::stopEmulation()
{
    if (m_timer)
        if (m_timer->isActive()) m_timer->stop();
}
bool MBDeviceEmulator::activated() const
{
    return m_timer->isActive();
}
void MBDeviceEmulator::out()
{
    foreach (MBEmulDevice d, m_devices)
    {
        qDebug()<<QString("device addr: %1,  signals %2,  reserve registers %3").arg(d.address).arg(d.signalsCount()).arg(d.emulPosList().count());
        for (int i=0; i<d.emul_signals.count(); i++)
            qDebug()<<QString("     signal[%1]: id=%2  (%3)  reserve pos %4").arg(i).arg(d.emul_signals.at(i).id).arg(d.emul_signals.at(i).reg_info.toStr()).arg(d.regPos(i));
    }
}
int MBDeviceEmulator::allSignalsCount() const
{
    int n = 0;
    foreach (MBEmulDevice d, m_devices)
        n += d.signalsCount();
    return n;
}
int MBDeviceEmulator::deviceSignalsCount(quint8 addr) const
{
    if (m_devices.contains(addr))
        return m_devices[addr].emul_signals.count();
    return -1;
}
void MBDeviceEmulator::addDevice(quint8 address)
{
    m_devices.insert(address, MBEmulDevice(address));
    m_devices[address].internal_index = m_devices.count() - 1;
}
void MBDeviceEmulator::addDeviceSignal(quint8 address, const MBRegisterInfo &info)
{
    if (m_devices.contains(address))
        m_devices[address].addSignal(info);
}
void MBDeviceEmulator::processPDU(quint8 addr, QByteArray &ba)
{
    quint16 dev_pos = mbRegsCount() - 8;
    if (ba.size() < 4)
    {
        ba[1] = uchar(dev_pos);
        ba[0] = uchar(dev_pos>>8);
        return;
    }

    quint16 pos = quint16((quint8(ba.at(0)) << 8) | quint8(ba.at(1))); //извлекаем позиция в запросе
    qDebug()<<QString("received pos %1,  addr %2").arg(pos).arg(addr);


    if (m_devices.contains(addr))
    {
        const MBEmulDevice &d = m_devices[addr];
        bool not_find = true;
        for (int i=0; i<d.signalsCount(); i++)
        {
            if (d.emul_signals.at(i).reg_info.pos == pos)
            {
                pos = d.regPos(i, dev_pos);
                not_find = false;
                break;
            }
        }

        if (not_find) pos = dev_pos;
    }
    else pos = dev_pos;

    //replace pos to PDU
    qDebug()<<QString("real pos %1").arg(pos);
    ba[1] = uchar(pos);
    ba[0] = uchar(pos>>8);
}
void MBDeviceEmulator::setEmuValueSettings(quint8 addr, const EmulValueSettings &evs)
{
    if (m_devices.contains(addr))
        m_devices[addr].setEmuValueSettings(evs);
}
void MBDeviceEmulator::setEmuValueSettings(quint8 addr, const EmulValueSettings &evs, int sig_i)
{
    if (m_devices.contains(addr))
        m_devices[addr].setEmuValueSettings(sig_i, evs);
}


/////////////////////MBEmulDevice//////////////////////////////
void MBEmulDevice::addSignal(const MBRegisterInfo &info)
{
    emul_signals.append(MBEmulSignal(info));
    emul_signals.last().calcID(address);
}
void MBEmulDevice::updateSignal(int i, double v, double factor, int adder)
{
    if (i < 0 || i >= emul_signals.count()) return;
    emul_signals[i].updateValue(v, factor, adder);
}
void MBEmulDevice::updateSignals(QMap<quint16, quint16> &map, bool is_first)
{
    qDebug()<<QString("update signals for addr %1").arg(address);
    if (emul_signals.isEmpty()) return;

    //update double signals
    for (int i=0; i<signalsCount(); i++)
    {
        if (emul_signals.at(i).isBit()) continue;

        if (!is_first && (LMath::rnd() > UPDATE_CHANCE_FACTOR)) continue;

        const EmulValueSettings &evs = (emul_signals.at(i).hasEmulSettings() ? emul_signals.at(i).emul_settings : emul_settings);
        if (evs.disabled()) {qWarning()<<QString(" disabled: rack_addr=%1  i=%2").arg(address).arg(i); continue;}

        quint16 pos = regPos(i);
        updateSignal(i, evs.nextValue(), evs.factor, evs.adder);
        map.insert(pos, emul_signals.at(i).value);
    }


    //update bin signals
    for (int i=0; i<signalsCount(); i++)
    {
        if (!emul_signals.at(i).isBit()) continue;
        quint16 pos = regPos(i);

        const EmulValueSettings &evs = (emul_signals.at(i).hasEmulSettings() ? emul_signals.at(i).emul_settings : emul_settings);
        if (evs.disabled()) {qWarning()<<QString(" disabled: rack_addr=%1  i=%2").arg(address).arg(i); continue;}


        if (emul_signals.at(i).isBit())
        {
            quint8 b_index = emul_signals.at(i).reg_info.bit_index;
            double d_bit = evs.nextBitValue();
            updateSignal(i, d_bit, 0);
            int cur_value = map.value(pos, 0);

            if (emul_signals.at(i).value > 0) LMath::setBitOn(cur_value, b_index);
            else LMath::setBitOff(cur_value, b_index);

            if (b_index == 0 && emul_signals.at(i).reg_info.pos == 256)
            {
                qDebug()<<QString("rack_addr=%0 cur_value=%1  sig_value=%2  d_bit=%3  pos=%4").arg(address).arg(LMath::toStr(cur_value)).arg(emul_signals.at(i).value).arg(d_bit).arg(pos);
            }
            map.insert(pos, quint16(cur_value));
        }
    }

}
quint16 MBEmulDevice::regPos(int i, quint16 def_value) const
{
    if (i < 0 || i >= emul_signals.count()) return def_value;

    quint16 start_pos = internal_index * MBEmulDevice::maxSignals();
    quint16 i_emul_pos = emul_signals.at(i).reg_info.pos;
    int pos_index = emulPosList().indexOf(i_emul_pos);
    if (pos_index < 0) return def_value;
    return (start_pos + pos_index);
}
QList<quint16> MBEmulDevice::emulPosList() const
{
    QList<quint16> list;
    for (int i=0; i<signalsCount(); i++)
    {
        quint16 emul_pos = emul_signals.at(i).reg_info.pos;
        if (!list.contains(emul_pos)) list.append(emul_pos);
    }
    return list;
}
void MBEmulDevice::setEmuValueSettings(const EmulValueSettings &evs)
{
    emul_settings.base_value = evs.base_value;
    emul_settings.err = evs.err;
    emul_settings.factor = evs.factor;
    emul_settings.adder = evs.adder;
}
void MBEmulDevice::setEmuValueSettings(int i, const EmulValueSettings &evs)
{
    if (i < 0 || i >= emul_signals.count()) return;
    emul_signals[i].setEmuValueSettings(evs);
}



/////////////////////MBEmulSignal//////////////////////////////
void MBEmulSignal::calcID(quint8 addr)
{
    id = 1000000000;
    //if (signal_type == estBit_16) id *= 2;
    id += (1000000 * addr);
    id += 100*reg_info.pos;
    if (isBit()) id += reg_info.bit_index;
}
void MBEmulSignal::updateValue(double v, double factor, int adder)
{
    if (isBit()) value = (v > 0 ? 1 : 0);
    else value = quint16((v + adder)*factor);
}
void MBEmulSignal::setEmuValueSettings(const EmulValueSettings &evs)
{
    emul_settings.base_value = evs.base_value;
    emul_settings.err = evs.err;
    emul_settings.factor = evs.factor;
    emul_settings.adder = evs.adder;
}


/////////////////////EmulValueSettings//////////////////////////////
double EmulValueSettings::nextValue() const
{
    if (err <= 0 || err > 100) return base_value;
    double dv = base_value*err/100;
    dv *= (LMath::rnd()*LMath::rndSignum());
    return (base_value + dv);
}
double EmulValueSettings::nextBitValue() const
{
    if (err < 0.1 || err > 100) return base_value;

    bool need_invert = (double(LMath::rndInt(0, 10000))/double(100) > err);
    if (need_invert)
    {
        if (base_value == 0) return 1;
        return 0;
    }
    return base_value;
}
QString EmulValueSettings::toStr() const
{
    return QString("EmulValueSettings: base_value=%1  err=%2%  factor=%3  adder=%4").arg(base_value).arg(err).arg(factor).arg(adder);
}



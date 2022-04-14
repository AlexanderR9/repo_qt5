#include "mbdeviceemulator.h"
#include "lmath.h"

#include <QDebug>
#include <QTimer>
#include <QModbusDataUnit>

#define UPDATE_VALUES_INTERVAL      5000
#define UPDATE_CHANCE_FACTOR        0.1  //вероятность того, что сигнал попытается обновиться


/////////////////////MBDeviceEmulator//////////////////////////////
MBDeviceEmulator::MBDeviceEmulator(QObject *parent)
    :QObject(parent),
    m_timer(NULL)
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
        it.value().updateSignals(need_update_regs);
        it++;
    }

    if (need_update_regs.isEmpty()) return;

    qDebug("update emulation values 2");
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
        if (!m_timer->isActive()) m_timer->start();
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
        //qDebug("find device by addr");
        bool not_find = true;
        for (int i=0; i<d.signalsCount(); i++)
        {
            if (d.emul_signals.at(i).reg_info.pos == pos)
            {
                //qDebug("find signal by pos");
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
const MBEmulDevice* MBDeviceEmulator::deviceAt(quint8 addr) const
{
    if (m_devices.contains(addr))
    {
        const MBEmulDevice &d = m_devices[addr];
    //    return static_cast<const MBEmulDevice*>(d);
    }

    QMap<quint8, MBEmulDevice>::const_iterator it = m_devices.constBegin();
    while (it != m_devices.constEnd())
    {
        //if (it.key() == addr) return it*;
        it++;
    }


    return NULL;
}
void MBDeviceEmulator::setEmuValueSettings(quint8 addr, const EmulValueSettings &evs)
{
    if (deviceAt(addr))
        m_devices[addr].setEmuValueSettings(evs);
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
void MBEmulDevice::updateSignals(QMap<quint16, quint16> &map)
{
    qDebug()<<QString("update signals for addr %1").arg(address);
    if (emul_signals.isEmpty()) return;

    for (int i=0; i<signalsCount(); i++)
    {
        if (LMath::rnd() > UPDATE_CHANCE_FACTOR) continue;

        quint16 pos = regPos(i);
        if (emul_signals.at(i).isBit())
        {
            updateSignal(i, emul_settings.nextBitValue(), 0);
            int cur_value = map.value(pos, 0);
            if (emul_signals.at(i).value > 0) LMath::setBitOn(cur_value, emul_signals.at(i).reg_info.bit_index);
            else LMath::setBitOff(cur_value, emul_signals.at(i).reg_info.bit_index);
            map.insert(pos, quint16(cur_value));
        }
        else
        {
            updateSignal(i, emul_settings.nextValue(), 100, 0);
            map.insert(pos, emul_signals.at(i).value);
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
    int bv = ((base_value > 0) ? 1 : 0);
    if (err <= 0 || err > 100) return bv;

    bool need_invert = (double(LMath::rndInt(0, 10000))/double(100) > err);
    if (need_invert) return ((bv == 1) ? 0 : 1);
    return bv;
}



#include "mbserver.h"
#include "comparams_struct.h"
#include "mbadu.h"
#include "lstatic.h"
#include "lmath.h"
#include "mbdeviceemulator.h"


#include <QSerialPort>
#include <QModbusPdu>
#include <QDebug>
#include <QDateTime>
#include <QtMath>

#define REQ_DATE_TIME_CODE      0x15

////////////// MBServer //////////////////
MBServer::MBServer(QObject *parent)
    :MBSlaveServerBase(parent),
    m_maxReadingReg(0),
    m_maxWritingReg(0),
    m_invalidPass(0),
    emul_complex(NULL)
{
    reset();
    ComParams params;
    setPortParams(params);

    //for emul funcs
    initEmulComplex();
    //startTimer(3700);

}
void MBServer::timerEvent(QTimerEvent*)
{    
    /*
    if (emul_complex.hash_pos.isEmpty()) return;

    qDebug("MBServer::timerEvent");
    double base_len = 26.7;
    double d_value = 0;
    MBEmulDevice& rack11 = emul_complex.emul_devices[11];
    for (int i=0; i<rack11.emul_signals.count(); i++)
    {
        d_value = LMath::rndSignum()*LMath::rnd()*1.5;
        rack11.updateSignal(i, base_len+d_value, 100, 0);
    }

    base_len = 32.1;
    MBEmulDevice& rack12 = emul_complex.emul_devices[12];
    for (int i=0; i<rack12.emul_signals.count(); i++)
    {
        d_value = LMath::rndSignum()*LMath::rnd()*5.5;;
        rack12.updateSignal(i, base_len+d_value, 100, 0);
    }

    double base_temp = 280;
    MBEmulDevice& rack16 = emul_complex.emul_devices[16];
    for (int i=0; i<rack16.emul_signals.count(); i++)
    {
        d_value = LMath::rndSignum()*LMath::rnd()*3.5;
        rack16.updateSignal(i, base_temp+d_value, 32, 125);
    }


    MBEmulDevice& rack15 = emul_complex.emul_devices[15];
    bool was_1 = false;
    for (int i=0; i<rack15.emul_signals.count(); i++)
    {
        quint8 bit = ((LMath::factorOk(73.5) && !was_1) ? 1 : 0);
        rack15.updateSignal(i, bit, 0, 0);
        if (bit > 0) was_1 = true;
    }
//    rack15.updateSignal(2, 1, 0, 0);
//    rack15.updateSignal(10, 1, 0, 0);


    rewriteEmulData();
    //emul_complex.out();
    */

}
void MBServer::initEmulComplex()
{
    //ВМЕСТО ЭТОЙ ФУНКЦИИ ОБЪЕКТ emul_complex ДОЛЖЕН ЗАПОЛНИТЬСЯ ИЗ КОНФИГА!!! (в будущем)

    quint16 start_pos = 0x0100;
    emul_complex = new MBDeviceEmulator(this);

    //init racks
    for (quint8 addr=11; addr<=16; addr++)
        emul_complex->addDevice(addr);

    //init signals linear
    for (quint8 addr=11; addr<=14; addr++)
        for (int i=1; i<=8; i++)
            for (int j=0; j<2; j++)
            {
                MBRegisterInfo info(MBRegisterInfo::estBit_16, start_pos*i + j);
                emul_complex->addDeviceSignal(addr, info);
            }

    //init signals Din
    for (int i=1; i<=9; i++)
        for (int j=0; j<8; j++)
        {
            MBRegisterInfo info(MBRegisterInfo::estBit_1, start_pos*i, j);
            emul_complex->addDeviceSignal(15, info);
        }

    //init signals temp
    for (int i=1; i<=4; i++)
        for (int j=0; j<4; j++)
            emul_complex->addDeviceSignal(16, MBRegisterInfo(MBRegisterInfo::estBit_16, start_pos*i + j));


    connect(emul_complex, SIGNAL(signalSetRegisterValue(int, quint16, quint16)), this, SLOT(slotSetRegisterValue(int, quint16, quint16)));

    emul_complex->out();
    qDebug()<<QString("emul_complex: rack count %1,  all_signals_count %2,  reserve_reristers %3").
                    arg(emul_complex->deviceCount()).arg(emul_complex->allSignalsCount()).arg(emul_complex->mbRegsCount());

    emul_complex->setEmuValueSettings(11, EmulValueSettings(29.3, 1.5));

    emul_complex->startEmulation();


    /*
    //init racks
    emul_complex.reset();
    for (quint8 addr=11; addr<=16; addr++)
        emul_complex.addDevice(addr);

    quint16 pos = 0x0100;

    //init signals linear
    for (quint8 addr=11; addr<=14; addr++)
        for (int i=1; i<=8; i++)
            for (int j=0; j<2; j++)
                emul_complex.addDeviceSignal(addr, MBEmulSignal::estBit_16, pos*i + j);

    //init signals Din
    for (int i=1; i<=9; i++)
        for (int j=0; j<8; j++)
            emul_complex.addDeviceSignal(15, MBEmulSignal::estBit_1, pos*i);

    //init signals temp
    for (int i=1; i<=4; i++)
        for (int j=0; j<4; j++)
            emul_complex.addDeviceSignal(16, MBEmulSignal::estBit_16, pos*i+j);

    emul_complex.recalcRegisterPos();
    emul_complex.out();
    qDebug()<<QString("emul_complex: rack count %1    hash_pos size %2").arg(emul_complex.emul_devices.count()).arg(emul_complex.hash_pos.count());

    */
}
void MBServer::rewriteEmulData()
{
    /*
    qDebug("MBServer::rewriteEmulData()");
    if (!isConnected()) return;

    QModbusDataUnit::RegisterType reg_type = QModbusDataUnit::HoldingRegisters;

    QList<quint8> addrs = emul_complex.emul_devices.keys();
    for (int i=0; i<addrs.count(); i++)
    {
        const MBEmulDevice& rack = emul_complex.emul_devices.value(addrs.at(i));
        //qDebug()<<QString("MBServer::rewriteEmulData():  rack %1").arg(rack.address);

        //write bit values
        QList<quint32> writed_id;
        while (2 > 1)
        {
            int pos = -1;
            quint16 value = 0;
            quint32 f_id = 0;
            for (int j=0; j<rack.emul_signals.count(); j++)
            {
                const MBEmulSignal &sig = rack.emul_signals.at(j);
                if (sig.isBit() && !writed_id.contains(sig.id))
                {
                    if (pos < 0) {pos = emul_complex.hash_pos.value(sig.id); f_id = sig.id;}
                    if (emul_complex.hash_pos.value(sig.id) == pos)
                    {
                        if (sig.value > 0)
                        {
                            quint8 bit_index = sig.id - sig.baseID();
                            int a = value;
                            LMath::setBitOn(a, bit_index);
                            value = quint16(a);
                        }
                        writed_id.append(sig.id);
                    }
                }
            }

            if (pos > 0) //qDebug()<<QString("write reg, type bit: id_param=%1  pos=%2  reg_value=%3  sig_value=%4").arg(f_id).arg(pos).arg(value).arg(LMath::toStr(value));
                setData(reg_type, quint16(pos), value);
            else break;
        }


        //write quint16 values
        for (int j=0; j<rack.emul_signals.count(); j++)
        {
            const MBEmulSignal &sig = rack.emul_signals.at(j);
            if (!sig.isBit())
            {
                quint16 reg_pos = emul_complex.hash_pos.value(sig.id);
                //if (rack.address == 16) qDebug()<<QString("write reg: id_param=%1  pos=%2  value=%3").arg(sig.id).arg(reg_pos).arg(sig.value);
                setData(reg_type, reg_pos, sig.value);
            }
        }
    }
    */
}
void MBServer::slotSetRegisterValue(int reg_type, quint16 pos, quint16 value)
{
    if (isConnected())
    {
        qDebug()<<QString("rewrite register pos=%1  value=%2").arg(pos).arg(value);
        setData(QModbusDataUnit::RegisterType(reg_type), pos, value);
    }
}
void MBServer::initRegistersMap()
{
    quint16 start_pos_reg = 0; //стартовая позиция регистров
    quint16 reg_count = 5000; //количество регистров в буфере, т.е. для 16 битных регистров размер буфера будет reg_count*2
    QModbusDataUnit data_unit(QModbusDataUnit::HoldingRegisters, start_pos_reg, emul_complex->mbRegsCount());
    QModbusDataUnit data_coils(QModbusDataUnit::Coils, start_pos_reg, reg_count);

    QModbusDataUnitMap map;
    map.insert(data_unit.registerType(), data_unit);
    map.insert(data_coils.registerType(), data_coils);
    setMap(map);

}
void MBServer::reset()
{
    m_cmdCounter.clear(); //счетчик команд
    m_maxReadingReg = 0;
    m_maxWritingReg = 0;
    m_invalidPass = 0;
}
QString MBServer::cmdCounterToStr() const
{
    QString s("RECEIVED CMD COUNTER: ");
    if (m_cmdCounter.isEmpty()) s += "empty container!";
    else
    {
        QList<quint8> keys = m_cmdCounter.keys();
        for (int i=0; i<keys.count(); i++)
            s = QString("%1  cmd(%2)/count(%3) ").arg(s).arg(keys.at(i)).arg(m_cmdCounter.value(keys.at(i)));
    }
    return s;
}
QString MBServer::regPosToStr() const
{
    QString s("MAX REGISTERS POS: ");
    s = QString("%1    for reading: %2   for writing: %3").arg(s).arg(m_maxReadingReg).arg(m_maxWritingReg);
    return s;
}
void MBServer::parseCurrentBuffer()
{
    if (bufferSmall()) return;

    MBAdu adu(m_buffer);
    if (adu.invalid())
    {
        if (m_invalidPass < 5) //ждем еще один кусочек пакета, т.е. даем еще один шанс, с 1-го раза не бракуем
        {
            qDebug()<<QString("     m_invalidPass=%1").arg(m_invalidPass);
            m_invalidPass++;
            return;
        }

        qWarning() << QString("(RTU server) Invalid ADU, err_code %1: %2").arg(adu.errCode()).arg(adu.stringErr());
        qWarning() << m_buffer.toHex() << QString("    size=%1").arg(adu.rawSize());
        clearBuffer();
        m_invalidPass = 0;
        return;
    }
    qDebug()<<adu.rawDataToStr();

    int packet_size = adu.rawSize();
    if (bufferSize() == packet_size) clearBuffer();
    else m_buffer.remove(0, packet_size); //trim lieft bytes by packet_size

    m_invalidPass = 0;
    is_broadcast = (adu.serverAddress() == 0);
    tryParseAdu(adu);

    //если буффер не пустой, то вызвать эту функцию снова
    if (!bufferEmpty())
        parseCurrentBuffer();
}
void MBServer::transformPDU(QModbusPdu &pdu, quint8 rack_addr)
{
    switch (pdu.functionCode())
    {
        case QModbusPdu::ReadHoldingRegisters: //запрос на чтение группы регистров из таблицы HoldingRegisters
        {
            QByteArray ba(pdu.data()); //тело запроса (все что после кода команды и до CRC) т.е. 2 байта - позиция регистра, 2 байта - сколько регистров считать, начиная с указанной позиции
            emul_complex->processPDU(rack_addr, ba); //обработать запрос
            pdu.setData(ba); //установить модифицированный запрос обратно

        /*
            QByteArray ba(pdu.data());
            quint16 pos = quint16((quint8(ba.at(0)) << 8) | quint8(ba.at(1))); //позиция в запросе
            quint32 id = emul_complex.idFromRequest(rack_addr, pos);
            //qDebug()<<QString("pos from request, pos=%1,  must_id=%2").arg(pos).arg(id);
            if (!emul_complex.hash_pos.contains(id))
            {
               //qDebug("id not found");
               //pdu.setData(QByteArray());
               pos = emul_complex.hash_pos.count()-8;
            }
            else pos = emul_complex.hash_pos.value(id);
            //qDebug()<<QString("pos from hash, pos=%1").arg(pos);

            //replace pos to PDU
            ba[1] = uchar(pos);
            ba[0] = uchar(pos>>8);
            pdu.setData(ba);
            */

            break;
        }
        default: break;
    }
}
void MBServer::tryParseAdu(const MBAdu &adu)
{
    //пытаемся извлечь PDU из ADU
    QModbusPdu pdu;
    adu.getPduData(pdu);
    const int pduSizeWithoutFcode = pdu.size() - 1;
    if ((2 + pduSizeWithoutFcode + 2) != adu.rawSize())
    {
        qWarning()<<QString("(RTU server) Invalid PDU size %1, ADU size %2").arg(pdu.size()).arg(adu.rawSize());
        return;
    }


    /////////////////////////алгоритм реакции MBServer на запрос//////////////////////////////////////
    transformPDU(pdu, adu.serverAddress()); // скорректировать PDU для своей задачи
    if (pdu.data().isEmpty()) return;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    const QModbusRequest request(pdu);
    QModbusResponse response;
    makeResponseByRequest(request, response);

    if (response.isValid())
        trySendResponse(response); //запись ответ в COM
}
void MBServer::setPortParams(const ComParams &params)
{
    m_port->setPortName(params.port_name);
    m_port->setBaudRate(params.baud_rate, QSerialPort::AllDirections);
    m_port->setDataBits(QSerialPort::DataBits(params.data_bits));
    m_port->setStopBits(QSerialPort::StopBits(params.stop_bits));
    m_port->setParity(QSerialPort::Parity(params.parity));
}
QModbusResponse MBServer::exeptionRequest(const QModbusPdu &request) const
{
    quint8 meiType;
    request.decodeData(&meiType);
    qDebug()<<QString("processRequest: functionCode=%1,  meiType=%2").arg(QModbusRequest::EncapsulatedInterfaceTransport).arg(meiType);
    if (meiType == 0x0D)
    {
        return QModbusExceptionResponse(request.functionCode(), QModbusExceptionResponse::IllegalFunction);
    }
    else if (meiType == REQ_DATE_TIME_CODE)
    {
        qDebug("this request time command code");
    }
    else qWarning("******************INVALID SUB FUNCTION****************************");

    return QModbusResponse();
}
/*
QModbusResponse MBServer::dateTimeResponse(const QModbusPdu &request) const
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint8(REQ_DATE_TIME_CODE);
    stream << curDTPoint_double();
    //stream << curDTPoint();
    stream << quint16(0);
    return QModbusResponse(request.functionCode(), ba);
}
qint64 MBServer::curDTPoint() const
{
    //struct timespec tm;
    //int res = clock_gettime(CLOCK_REALTIME, &tm);
    //return (((qint64)tm.tv_sec)*1000000+((qint64)tm.tv_nsec)/1000);

    return 0;
}
*/



#include "mbserver.h"
#include "comparams_struct.h"
#include "mbadu.h"
#include "lstatic.h"
#include "lmath.h"
#include "mbdeviceemulator.h"
#include "mbconfigloader.h"



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
    emul_config_loader(NULL),
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

}
void MBServer::initEmulComplex()
{
    //ВМЕСТО ЭТОЙ ФУНКЦИИ ОБЪЕКТ emul_complex ДОЛЖЕН ЗАПОЛНИТЬСЯ ИЗ КОНФИГА!!! (в будущем)

    emul_complex = new MBDeviceEmulator(this);
    emul_config_loader = new MBConfigLoader(emul_complex);
    emul_complex->out();

    connect(emul_complex, SIGNAL(signalSetRegisterValue(int, quint16, quint16)), this, SLOT(slotSetRegisterValue(int, quint16, quint16)));

    emul_complex->startEmulation();

/*

    quint16 start_pos = 0x0100;
    quint8 addr_offset = 0;

    //init racks
    for (quint8 addr=11; addr<=16; addr++)
        emul_complex->addDevice(addr+addr_offset);

    //init signals linear
    for (quint8 addr=11; addr<=14; addr++)
        for (int i=1; i<=8; i++)
            for (int j=0; j<2; j++)
            {
                MBRegisterInfo info(MBRegisterInfo::estBit_16, start_pos*i + j);
                emul_complex->addDeviceSignal(addr+addr_offset, info);
            }

    //init signals Din
    for (int i=1; i<=9; i++)
        for (int j=0; j<8; j++)
        {
            MBRegisterInfo info(MBRegisterInfo::estBit_1, start_pos*i, j);
            emul_complex->addDeviceSignal(15+addr_offset, info);
        }

    //init signals temp
    for (int i=1; i<=4; i++)
        for (int j=0; j<4; j++)
            emul_complex->addDeviceSignal(16+addr_offset, MBRegisterInfo(MBRegisterInfo::estBit_16, start_pos*i + j));


    connect(emul_complex, SIGNAL(signalSetRegisterValue(int, quint16, quint16)), this, SLOT(slotSetRegisterValue(int, quint16, quint16)));

    //emul_complex->out();
    qDebug()<<QString("emul_complex: rack count %1,  all_signals_count %2,  reserve_reristers %3").arg(emul_complex->deviceCount()).arg(emul_complex->allSignalsCount()).arg(emul_complex->mbRegsCount());

    emul_complex->setEmuValueSettings(11+addr_offset, EmulValueSettings(46.3, 0.5, 100));
    emul_complex->setEmuValueSettings(12+addr_offset, EmulValueSettings(42.3, 0.5, 100));
    emul_complex->setEmuValueSettings(13+addr_offset, EmulValueSettings(68.3, 0.5, 100));
    emul_complex->setEmuValueSettings(14+addr_offset, EmulValueSettings(64.8, 0.1, 100));

    emul_complex->setEmuValueSettings(15+addr_offset, EmulValueSettings(0, 0, 1));
    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(218.3, 0.5, 32, 125));

    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(327, 0.5, 32, 125), 6);
    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(341, 0.5, 32, 125), 7);
    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(291, 0.2, 32, 125), 8);

    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(281, 0.2, 32, 125), 2);
    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(281, 0.2, 32, 125), 5);
    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(281, 0.2, 32, 125), 13);
    emul_complex->setEmuValueSettings(16+addr_offset, EmulValueSettings(281, 0.2, 32, 125), 14);

    emul_complex->startEmulation();
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
bool MBServer::open()
{
    if (isDisconnected()) emul_complex->setFirstValuesOn();
    return MBSlaveServerBase::open();
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

    if (emul_config_loader) setEmulConfig(params.emul_config);
}
void MBServer::setEmulConfig(const QString &f_name)
{
    if (isConnected()) return;

    if (f_name.trimmed() != emul_config_loader->currentConfig())
    {
        bool ok;
        emul_config_loader->setConfig(f_name.trimmed());
        emul_config_loader->tryLoadConfig(ok);
        if (!ok) qWarning()<<QString("MBServer::setEmulConfig WARNING cant't load emul config");
        else  emul_complex->out();

    }
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



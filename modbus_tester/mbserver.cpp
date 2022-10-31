#include "mbserver.h"
//#include "comparams_struct.h"
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
    :LMBSlaveServerBase(parent),
    m_maxReadingReg(0),
    m_maxWritingReg(0),
    m_invalidPass(0),
    emul_config_loader(NULL),
    emul_complex(NULL)
{
    reset();

    //for emul funcs
    initEmulComplex();
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
    return LMBSlaveServerBase::open();
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

    LMBAdu adu(m_buffer);
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
void MBServer::tryParseAdu(const LMBAdu &adu)
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






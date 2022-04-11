#include "mbserver.h"
#include "comparams_struct.h"
#include "mbadu.h"
#include "lstatic.h"

#include <QSerialPort>
#include <QModbusPdu>
#include <QDebug>
#include <QDateTime>
#include <QtMath>

#define REQ_DATE_TIME_CODE      0x15
//#define DT_START_POINT          4744694139655227477 //стартовая точка времи, соответствующая   09.03.2022  16:30:00


////////////// MBServer //////////////////
MBServer::MBServer(QObject *parent)
    :QModbusServer(parent),
    m_port(NULL),
    is_broadcast(false),
    m_maxReadingReg(0),
    m_maxWritingReg(0),
    m_invalidPass(0)
{
   m_port = new QSerialPort(this);

   ComParams params;
   setPortParams(params);

   connect(m_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
   connect(m_port, SIGNAL(aboutToClose()), this, SLOT(slotAboutToClose()));
   connect(m_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotError()));

   startTimer(3800);
}
void MBServer::timerEvent(QTimerEvent*)
{
    qDebug("MBServer::timerEvent");
    double base_len = 26.7; //mm
    quint16 pos = 0x100; //for rack 1
    quint16 value = 0; //reg_value
    double d_value = 0;

    int start_rack = 11;
    for (int rack=11; rack<=14; rack++)
    {
        int d_rack = rack - start_rack;
        double rack_factor = qPow(1.2, d_rack);
        for (int i=0; i<8; i++)
        {
            for (int j=0; j<2; j++)
            {
                int sign = ((qrand()%99 < 50) ? -1 : 1);
                d_value = sign*double(qrand()%300)/100;
                value = quint16((rack_factor*base_len + d_value)*100);
                setData(QModbusDataUnit::HoldingRegisters, (pos*(i+1) + j + d_rack*2), value);
            }
        }
    }


    //temperature params, rack=16
    double base_temp = 280;
    int d_rack = 16 - start_rack;
    for (int i=0; i<4; i++)
    {
        double group_factor = qPow(1.05, i);
        for (int j=0; j<4; j++)
        {
            //mul="0.03125" add="-125.0"
            int sign = ((qrand()%99 < 50) ? -1 : 1);
            d_value = sign*double(qrand()%300)/100;
            value = quint16((group_factor*base_temp + d_value + 125)*32);
            setData(QModbusDataUnit::HoldingRegisters, (pos*(i+1) + j + d_rack*2), value);
        }
    }


    //d_value = 3.6;
    //value = quint16((base_temp + d_value + 125)*32);
    //setData(QModbusDataUnit::HoldingRegisters, (pos + d_rack*2), value);


    //rack 1,  params 1 - 2
    //setData(QModbusDataUnit::HoldingRegisters, pos, value);
    //setData(QModbusDataUnit::HoldingRegisters, pos+1, quint16(value*1.5));
    //rack 1,  params 3 - 4
    //value = 3700;
    //setData(QModbusDataUnit::HoldingRegisters, 2*pos, value);
    //setData(QModbusDataUnit::HoldingRegisters, 2*pos+1, quint16(value*1.5));

    //pos += 2;
    //value = 2200;
    //setData(QModbusDataUnit::HoldingRegisters, pos, value);

}
void MBServer::initBuffers()
{
    quint16 start_pos_reg = 0; //стартовая позиция регистров
    quint16 reg_count = 5000; //количество регистров в буфере, т.е. для 16 битных регистров размер буфера будет reg_count*2
    QModbusDataUnit data_unit(QModbusDataUnit::HoldingRegisters, start_pos_reg, reg_count);
    QModbusDataUnit data_coils(QModbusDataUnit::Coils, start_pos_reg, reg_count);
    //QModbusDataUnit data_inputs(QModbusDataUnit::DiscreteInputs, start_pos_reg, reg_count);

    QModbusDataUnitMap map;
    map.insert(data_unit.registerType(), data_unit);
    //map.insert(data_inputs.registerType(), data_inputs);
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
void MBServer::slotError()
{
    int code = m_port->error();
    if (code == QSerialPort::NoError) return;
    qWarning()<<QString("MBServer::slotError() QSerialPort error, code=%1").arg(m_port->error());

    switch (code)
    {
        case QSerialPort::DeviceNotFoundError:
            setError(QModbusDevice::tr("Referenced serial device does not exist."), QModbusDevice::ConnectionError);
            break;
        case QSerialPort::PermissionError:
            setError(QModbusDevice::tr("Cannot open serial device due to permissions."), QModbusDevice::ConnectionError);
            break;
        case QSerialPort::OpenError:
        case QSerialPort::NotOpenError:
            setError(QModbusDevice::tr("Cannot open serial device."), QModbusDevice::ConnectionError);
            break;
        case QSerialPort::WriteError:
            setError(QModbusDevice::tr("Write error."), QModbusDevice::WriteError);
            break;
        case QSerialPort::ReadError:
            setError(QModbusDevice::tr("Read error."), QModbusDevice::ReadError);
            break;
        case QSerialPort::ResourceError:
            setError(QModbusDevice::tr("Resource error."), QModbusDevice::ConnectionError);
            break;
        case QSerialPort::UnsupportedOperationError:
            setError(QModbusDevice::tr("Device operation is not supported error."), QModbusDevice::ConfigurationError);
            break;
        case QSerialPort::TimeoutError:
            setError(QModbusDevice::tr("Timeout error."), QModbusDevice::TimeoutError);
            break;
        case QSerialPort::UnknownError:
            setError(QModbusDevice::tr("Unknown error."), QModbusDevice::UnknownError);
            break;
        default:
            qWarning() << "(RTU server) Unhandled QSerialPort error  " << code;
            break;
    }

    emit signalError(QString("com_port error, code=%1").arg(code));
}
void MBServer::slotAboutToClose()
{
    qDebug("MBServer::slotAboutToClose()");
    if (state() != QModbusDevice::ClosingState)
        setState(QModbusDevice::UnconnectedState);
}
bool MBServer::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    m_requestBuffer.clear();
    initBuffers();

    if (m_port->open(QIODevice::ReadWrite))
    {
        setState(QModbusDevice::ConnectedState);
        m_port->clear(); // only possible after open
        reset();
        qDebug("slave device opened!");
    }
    else setError(m_port->errorString(), QModbusDevice::ConnectionError);

    return (state() == QModbusDevice::ConnectedState);
}
void MBServer::close()
{
    if (state() == QModbusDevice::UnconnectedState)
        return;

    if (m_port->isOpen()) m_port->close();
    setState(QModbusDevice::UnconnectedState);
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
void MBServer::slotReadyRead()
{
    //qDebug()<<QString("ComObj::slotReadyRead()  %1").arg(m_port->bytesAvailable());
    const int size = m_port->size();
    m_requestBuffer += m_port->read(size);
    //qDebug() << QString("Current request buffer: ") << m_requestBuffer.toHex();
    parseCurrentBuffer();
}
void MBServer::parseCurrentBuffer()
{
    if (m_requestBuffer.size() < 4) return;

    MBAdu adu(m_requestBuffer);
    if (adu.invalid())
    {
        if (m_invalidPass < 5) //ждем еще один кусочек пакета, т.е. даем еще один шанс, с 1-го раза не бракуем
        {
            qDebug()<<QString("     m_invalidPass=%1").arg(m_invalidPass);
            m_invalidPass++;
            return;
        }

        qWarning() << QString("(RTU server) Invalid ADU, err_code %1: %2").arg(adu.errCode()).arg(adu.stringErr());
        qWarning() << m_requestBuffer.toHex() << QString("    size=%1").arg(adu.rawSize());
        m_requestBuffer.clear();
        m_invalidPass = 0;
        return;
    }
    qDebug()<<adu.rawDataToStr();

    int packet_size = adu.rawSize();
    if (m_requestBuffer.size() == packet_size) m_requestBuffer.clear();
    else
    {
        qDebug()<<QString("trim buffer:  requestBuffer %1 bytes / packet_size %2 bytes").arg(m_requestBuffer.size()).arg(packet_size);
        m_requestBuffer.remove(0, packet_size);
    }

    m_invalidPass = 0;
    is_broadcast = (adu.serverAddress() == 0);
    tryParseAdu(adu);

    if (!m_requestBuffer.isEmpty()) parseCurrentBuffer();
}
void MBServer::transformPDU(QModbusPdu &pdu, quint8 rack_addr)
{
    quint8 d_rack = rack_addr - 11;

    switch (pdu.functionCode())
    {
        case QModbusPdu::ReadHoldingRegisters:
        {
            if (d_rack > 0)
            {
                QByteArray ba(pdu.data());
                quint16 pos = quint16((quint8(ba.at(0)) << 8) | quint8(ba.at(1)));
               // qDebug()<<QString("pos=%1  d_rack=%2").arg(pos).arg(d_rack);
                pos += d_rack*2;
                ba[1] = uchar(pos);
                ba[0] = uchar(pos>>8);
                pdu.setData(ba);
                //qDebug()<<QString("PDU after transform, rack_addr=%1, pdu_data: ").arg(rack_addr) << ba.toHex();
            }
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

    // PDU ok

    /////////////////////////алгоритм реакции MBServer на запрос//////////////////////////////////////
    transformPDU(pdu, adu.serverAddress());
    /*
    if (!is_broadcast && (adu.serverAddress() != this->serverAddress())) //проверка адреса устройства в запросе
    {
        qWarning() << QString("(RTU server) Wrong server address %1 / %2").arg(serverAddress()).arg(adu.serverAddress());
        return;
    }
    */

    const QModbusRequest request(pdu);
    QModbusResponse response;
    if (value(QModbusServer::DeviceBusy).value<quint16>() == 0xffff)
    {
        qWarning() << QString("(RTU server) DeviceBusy");
        response = QModbusExceptionResponse(request.functionCode(),  QModbusExceptionResponse::ServerDeviceBusy);
    }
    else response = processRequest(request);

    if ((!response.isValid()) || processesBroadcast() || value(QModbusServer::ListenOnlyMode).toBool())
    {
        qWarning() << QString("(RTU server) Invalid response, exeption %1").arg(response.exceptionCode());
        qWarning() << response;
        return;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////


    trySendResponse(response); //запись ответ в COM
}
void MBServer::trySendResponse(const QModbusResponse &response)
{
    QByteArray result;
    getResponseBA(result, response); //convert QModbusResponse -> QByteArray

    if (!isConnected()) //по каким-то причинам произошел разрыв связи
    {
        qWarning() << "(RTU server) Requesting serial port has closed.";
        setError(QString("Requesting serial port is closed"), QModbusDevice::WriteError);
        m_port->clear(QSerialPort::Output);
        return;
    }

    //qDebug()<<QString("send response, size=%1").arg(result.size());
    int writtenBytes = m_port->write(result);   // запись ответа в COM порт
    if ((writtenBytes < 0) || (writtenBytes < result.size())) //проверка результат записи
    {
        qWarning() << "(RTU server) Cannot write requested response to serial port.";
        setError(QString("Could not write response to client"), QModbusDevice::WriteError);
        m_port->clear(QSerialPort::Output);
        return;
    }

    if (response.isException()) //некое исключение
        qWarning() << QString("WARNING: (RTU server) response exception %1 :  %2").arg(response.exceptionCode()).arg(LStatic::baToStr(result, 24));
}
void MBServer::getResponseBA(QByteArray &ba, const QModbusPdu &pdu)
{
    ba.clear();
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << quint8(serverAddress()) << pdu;
    quint16 crc = MBAdu::convertToLittleEndian(MBAdu::MB_CRC16(ba));
    stream << crc;
}
void MBServer::setPortParams(const ComParams &params)
{
    m_port->setPortName(params.port_name);
    m_port->setBaudRate(params.baud_rate, QSerialPort::AllDirections);
    m_port->setDataBits(QSerialPort::DataBits(params.data_bits));
    m_port->setStopBits(QSerialPort::StopBits(params.stop_bits));
    m_port->setParity(QSerialPort::Parity(params.parity));
}
QModbusResponse MBServer::processRequest(const QModbusPdu &request)
{
    if (request.functionCode() == QModbusRequest::EncapsulatedInterfaceTransport) //в запросе необычный код команды
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
            return dateTimeResponse(request);
        }
        else qWarning("******************INVALID SUB FUNCTION****************************");
    }

    return QModbusServer::processRequest(request); //стандартная базовая обработка QModbusPdu
}
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
double MBServer::curDTPoint_double() const
{
    struct timespec tm;
    int res = clock_gettime(CLOCK_REALTIME, &tm);

    double factor_6 = 1000000;
    double sec = tm.tv_sec;
    double mk_sec = -1000 + double(tm.tv_nsec)/double(1000);
    return (sec + mk_sec/factor_6);
}
qint64 MBServer::curDTPoint() const
{
    struct timespec tm;
    int res = clock_gettime(CLOCK_REALTIME, &tm);
    return (((qint64)tm.tv_sec)*1000000+((qint64)tm.tv_nsec)/1000);
}







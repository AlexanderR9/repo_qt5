#include "mbserver.h"
#include "comparams_struct.h"
#include "mbadu.h"

#include <QSerialPort>
#include <QModbusPdu>
#include <QDebug>
#include <QDateTime>

#define REQ_DATE_TIME_CODE      0x15
#define DT_START_POINT          4744694139655227477 //стартовая точка времи, соответствующая   09.03.2022  16:30:00



MBServer::MBServer(QObject *parent)
    :QModbusServer(parent),
    m_port(NULL),
    is_broadcast(false),
    m_maxReadingReg(0),
    m_maxWritingReg(0),
    m_invalidPass(0)
    //m_elapsedOffset(0)
{
   m_port = new QSerialPort(this);

   ComParams params;
   setPortParams(params);

   connect(m_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
   connect(m_port, SIGNAL(aboutToClose()), this, SLOT(slotAboutToClose()));
   connect(m_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotError()));

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
/*
void MBServer::recalcElapsedOffset()
{
    QDateTime cur_dt(QDateTime::currentDateTime());
    QDateTime start_dt(QDate(2022, 3, 9), QTime(16, 30));

    qint64 d = start_dt.msecsTo(cur_dt);
    m_elapsedOffset = quint64(d)*16.35;
    qDebug()<<QString("MBServer::recalcElapsedOffset()   m_elapsedOffset = %1").arg(m_elapsedOffset);
}
*/
bool MBServer::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;


    m_requestBuffer.clear();

    if (m_port->open(QIODevice::ReadWrite))
    {
        setState(QModbusDevice::ConnectedState);
        m_port->clear(); // only possible after open
        //recalcElapsedOffset();
        //m_elapsedTimer.start();
        reset();
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
        {
            s = QString("%1  cmd(%2)/count(%3) ").arg(s).arg(keys.at(i)).arg(m_cmdCounter.value(keys.at(i)));
        }
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

    MBAdu adu(m_requestBuffer);
    //qDebug() << QString("Current request buffer: ") << adu.rawData().toHex();

    //qDebug()<<adu.rawDataToStr();
    if (adu.invalid())
    {
        //qWarning() << m_requestBuffer.toHex() << QString("    size=%1").arg(adu.rawSize());
        if (m_invalidPass < 1)
        {
            m_invalidPass++;
            //qWarning()<<QString("invalid pass: %1").arg(m_invalidPass);
            return;
        }

        qWarning()<<QString("(RTU server) Invalid ADU, err_code %1: %2").arg(adu.errCode()).arg(adu.stringErr());
        qWarning() << m_requestBuffer.toHex() << QString("    size=%1").arg(adu.rawSize());
        m_requestBuffer.clear();
        m_invalidPass = 0;
        return;
    }

    m_invalidPass = 0;
    /*
    m_cmdCounter.insert(adu.cmdCode(), m_cmdCounter.value(adu.cmdCode(), 0) + 1);
    //qDebug()<<cmdCounterToStr();

    if (adu.readingCmd())
    {
        if (adu.startPosReg() > m_maxReadingReg) m_maxReadingReg = adu.startPosReg();
    }
    else if (adu.writingCmd())
    {
        if (adu.startPosReg() > m_maxWritingReg) m_maxWritingReg = adu.startPosReg();
    }
    else
    {
        qDebug() << QString("Current request buffer: ") << adu.rawData().toHex();
    }
    //qDebug()<<regPosToStr();
    */

    is_broadcast = (adu.serverAddress() == 0);
    QModbusPdu pdu;
    adu.getPduData(pdu);
    const int pduSizeWithoutFcode = pdu.size() - 1;
    if ((2 + pduSizeWithoutFcode + 2) != adu.rawSize())
    {
        qWarning()<<QString("(RTU server) Invalid PDU size %1, ADU size %2").arg(pdu.size()).arg(adu.rawSize());
        return;
    }

    m_requestBuffer.clear();
    if (!is_broadcast && (adu.serverAddress() != this->serverAddress()))
    {
        qWarning()<<QString("(RTU server) Wrong server address %1 / %2").arg(serverAddress()).arg(adu.serverAddress());
        return;
    }

    const QModbusRequest request(pdu);
    QModbusResponse response;
    if (value(QModbusServer::DeviceBusy).value<quint16>() == 0xffff)
    {
        qWarning()<<QString("(RTU server) DeviceBusy");
        response = QModbusExceptionResponse(request.functionCode(),  QModbusExceptionResponse::ServerDeviceBusy);
    }
    else
    {
        //incrementCounter(QModbusServerPrivate::Counter::ServerMessage);
        response = processRequest(request);
    }

    if ((!response.isValid()) || processesBroadcast() || value(QModbusServer::ListenOnlyMode).toBool())
    {
        qWarning()<<QString("(RTU server) Invalid response, exeption %1").arg(response.exceptionCode());
        qWarning()<<response;
        return;
    }

    QByteArray result;
    getResponseBA(result, response);
    if (adu.cmdCode() == 43)
        qDebug() << "(RTU server) Response ADU:" << result.toHex();

    if (!isConnected())
    {
        qWarning() << "(RTU server) Requesting serial port has closed.";
        setError(QString("Requesting serial port is closed"), QModbusDevice::WriteError);
        m_port->clear(QSerialPort::Output);
        return;
    }

    int writtenBytes = m_port->write(result);
    if ((writtenBytes < 0) || (writtenBytes < result.size()))
    {
        qWarning() << "(RTU server) Cannot write requested response to serial port.";
        setError(QString("Could not write response to client"), QModbusDevice::WriteError);
        m_port->clear(QSerialPort::Output);
        return;
    }

    if (response.isException())
    {
        qWarning() << QString("WARNING: (RTU server) response exception %1").arg(response.exceptionCode());
        qDebug() << QString("Current request buffer: ") << adu.rawData().toHex();
    }

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

    //setConnectionParameter(QModbusDevice::SerialPortNameParameter, params.port_name);
    // setConnectionParameter(QModbusDevice::SerialDataBitsParameter, params.data_bits);
    // setConnectionParameter(QModbusDevice::SerialStopBitsParameter, params.stop_bits);
    // setConnectionParameter(QModbusDevice::SerialBaudRateParameter, params.baud_rate);
     //setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud115200);
    // setConnectionParameter(QModbusDevice::SerialParityParameter, params.parity);

}
QModbusResponse MBServer::processRequest(const QModbusPdu &request)
{
    if (request.functionCode() == QModbusRequest::EncapsulatedInterfaceTransport)
    {
        quint8 meiType;
        request.decodeData(&meiType);
        qDebug()<<QString("processRequest: functionCode=%1,  meiType=%2").arg(QModbusRequest::EncapsulatedInterfaceTransport).arg(meiType);

        /*
        namespace EncapsulatedInterfaceTransport
        {
            enum SubFunctionCode {CanOpenGeneralReference = 0x0D, ReadDeviceIdentification = 0x0E};
        }
        */

        //if (meiType == EncapsulatedInterfaceTransport::CanOpenGeneralReference)

        if (meiType == 0x0D)
            return QModbusExceptionResponse(request.functionCode(), QModbusExceptionResponse::IllegalFunction);
        else if (meiType == REQ_DATE_TIME_CODE)
        {
            return dateTimeResponse(request);
        }
        else qWarning("******************INVALID SUB FUNCTION****************************");
    }
    return QModbusServer::processRequest(request);
}
QModbusResponse MBServer::dateTimeResponse(const QModbusPdu &request) const
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint8(REQ_DATE_TIME_CODE);
    /*
    //09.03.2022    16:30:00
    stream << quint8(0x41);
    stream << quint8(0xd8);
    stream << quint8(0x8a);
    stream << quint8(0x2e);
    stream << quint8(0x7a);
    stream << quint8(0x07);
    stream << quint8(0x08);
    stream << quint8(0x55);
    */

    stream << curDTPoint_double();
    //stream << curDTPoint();
    stream << quint16(0);

    return QModbusResponse(request.functionCode(), ba);
}
double MBServer::curDTPoint_double() const
{
    struct timespec tm;
    //int res = clock_gettime(CLOCK_MONOTONIC, &tm);
    int res = clock_gettime(CLOCK_REALTIME, &tm);

    //sys_time<milliseconds> utc_ms{round<milliseconds>(ms{my_utc_ts})};

    double factor_6 = 1000000;
    double sec = tm.tv_sec;
    double mk_sec = -1000 + double(tm.tv_nsec)/double(1000);
    return (sec + mk_sec/factor_6);

}

qint64 MBServer::curDTPoint() const
{
    //auto current_time = std::chrono::system_clock::now();
    //return quint64(std::chrono::system_clock::now());
    //double d = current_time;


    struct timespec tm;
    int res = clock_gettime(CLOCK_REALTIME, &tm);
    //qDebug()<<QString("clock %1").arg((int64_t)(((int64_t)tm.tv_sec)*1000000+((int64_t)tm.tv_nsec)/1000));
    return (((qint64)tm.tv_sec)*1000000+((qint64)tm.tv_nsec)/1000);



    //old code
    //quint64 s_point = quint64(DT_START_POINT) + m_elapsedOffset;
    //s_point += quint64(m_elapsedTimer.nsecsElapsed()/1000);
    //qDebug()<<QString("m_elapsedTimer microsecs: %1").arg(m_elapsedTimer.nsecsElapsed()/1000);
    //return s_point;
}







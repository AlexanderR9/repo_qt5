#include "mbslaveserverbase.h"
#include "mbadu.h"
#include "lstatic.h"

#include <QSerialPort>
#include <QModbusResponse>
#include <QModbusRequest>
#include <QDebug>


////////////// MBSlaveServerBase //////////////////
MBSlaveServerBase::MBSlaveServerBase(QObject *parent)
    :QModbusServer(parent),
    m_port(NULL),
    is_broadcast(false)
{
    initComPort();

    connect(m_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(m_port, SIGNAL(aboutToClose()), this, SLOT(slotAboutToClose()));
    connect(m_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotError()));

    m_buffer.clear();
}
void MBSlaveServerBase::initComPort()
{
    m_port = new QSerialPort(this);
    m_port->setPortName(QString("/dev/ttyS0"));
    m_port->setBaudRate(QSerialPort::Baud19200, QSerialPort::AllDirections);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::EvenParity);
}
bool MBSlaveServerBase::open()
{
    if (!isConnected())
    {
        m_buffer.clear();
        initRegistersMap();

        if (m_port->open(QIODevice::ReadWrite))
        {
            setState(QModbusDevice::ConnectedState);
            m_port->clear(); // only possible after open
        }
        else setError(m_port->errorString(), QModbusDevice::ConnectionError);
    }
    return isConnected();
}
void MBSlaveServerBase::close()
{
    if (isDisconnected()) return;
    if (m_port->isOpen()) m_port->close();
    setState(QModbusDevice::UnconnectedState);
}
bool MBSlaveServerBase::isConnected() const
{
    return (state() == QModbusDevice::ConnectedState);
}
bool MBSlaveServerBase::isDisconnected() const
{
    return (state() == QModbusDevice::UnconnectedState);
}
bool MBSlaveServerBase::isClosing() const
{
    return (state() == QModbusDevice::ClosingState);
}
void MBSlaveServerBase::slotAboutToClose()
{
    if (!isClosing())
        setState(QModbusDevice::UnconnectedState);
}
void MBSlaveServerBase::slotError()
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
void MBSlaveServerBase::slotReadyRead()
{
    const int size = m_port->size();
    m_buffer += m_port->read(size);
    parseCurrentBuffer();
}
void MBSlaveServerBase::convertResponseToBA(const QModbusResponse &response, QByteArray &ba)
{
    ba.clear();
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << quint8(serverAddress()) << response;
    quint16 crc = MBAdu::convertToLittleEndian(MBAdu::MB_CRC16(ba));
    stream << crc;
}
void MBSlaveServerBase::trySendResponse(const QModbusResponse &response)
{
    if (!isConnected()) //по каким-то причинам произошел разрыв связи
    {
        qWarning() << "(RTU server) Requesting serial port has closed.";
        setError(QString("Requesting serial port is closed"), QModbusDevice::WriteError);
        m_port->clear(QSerialPort::Output);
        return;
    }

    QByteArray reply_ba;
    convertResponseToBA(response, reply_ba); //convert QModbusResponse -> QByteArray
    int n_bytes = m_port->write(reply_ba);   // запись ответа в COM порт
    if ((n_bytes < 0) || (n_bytes < reply_ba.size())) //проверка результат записи
    {
        qWarning() << "(RTU server) Cannot write requested response to serial port.";
        setError(QString("Could not write response to client"), QModbusDevice::WriteError);
        m_port->clear(QSerialPort::Output);
        return;
    }

    if (response.isException()) //ответ содержит некое исключение протокола MB
        qWarning() << QString("WARNING: (RTU server) response exception %1 :  %2").arg(response.exceptionCode()).arg(LStatic::baToStr(reply_ba, 24));
}
QModbusResponse MBSlaveServerBase::processRequest(const QModbusPdu &request)
{
    if (request.functionCode() == QModbusRequest::EncapsulatedInterfaceTransport) //в запросе необычный код команды (исключение)
    {
        return exeptionRequest(request);
    }
    return QModbusServer::processRequest(request); //стандартная базовая обработка QModbusPdu
}
void MBSlaveServerBase::makeResponseByRequest(const QModbusRequest &request, QModbusResponse &response)
{
    if (value(QModbusServer::DeviceBusy).value<quint16>() == 0xffff)
    {
        qWarning() << QString("(RTU server) DeviceBusy");
        response = QModbusExceptionResponse(request.functionCode(),  QModbusExceptionResponse::ServerDeviceBusy);
        return;
    }

    response = processRequest(request);
    if (!response.isValid() || processesBroadcast() || value(QModbusServer::ListenOnlyMode).toBool())
    {
        qWarning() << QString("(RTU server) Invalid response, exeption %1").arg(response.exceptionCode());
        qWarning() << response;
    }
}




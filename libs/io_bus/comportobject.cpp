#include "comportobject.h"
#include "comparams.h"


/////////////// LComPortObj ///////////////////
LComPortObj::LComPortObj(QObject *parent)
    :QSerialPort(parent),
    m_autoClean(false),
    m_minReceivedBytes(-1)
{
    LComParams params;
    //setPortParams(params);

    connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotError()));

}
void LComPortObj::slotReadyRead()
{
    if (m_autoClean) m_buffer.clear();
    int cur_size = bufferSize();
    m_buffer.append(readAll());

    if (m_minReceivedBytes > 0 && bufferSize() < m_minReceivedBytes) return;
    emit signalDataReceived(bufferSize() - cur_size);
}
void LComPortObj::slotError()
{
    int code = error();
    if (code == QSerialPort::NoError) return;
    emit signalPortError(code);
}
void LComPortObj::setPortName(QString p_name)
{
    setPortName(p_name.trimmed());
}
void LComPortObj::setPortParams(const LComParams &params)
{
    setPortName(params.port_name);
    setBaudRate(params.baud_rate, QSerialPort::Direction(params.direction));
    setDataBits(QSerialPort::DataBits(params.data_bits));
    setStopBits(QSerialPort::StopBits(params.stop_bits));
    setFlowControl(QSerialPort::FlowControl(params.flow_control));
    setParity(QSerialPort::Parity(params.parity));
    m_direction = params.direction;
}
void LComPortObj::tryOpen(QString &err)
{
    err.clear();
    clearError();
    if (portOpened())
    {
        err = QString("port allready opened.");
        return;
    }
    bool ok = open(QIODevice::OpenMode(openModeByDirection()));
    if (!ok) err = QString("error opening port.");
}
void LComPortObj::tryClose(QString &err)
{
    err.clear();
    if (!portOpened())
    {
        err = QString("port allready closed.");
        return;
    }
    close();
}
void LComPortObj::tryWrite(const QByteArray &ba, QString &err)
{
    err.clear();
    if (!portOpened())
    {
        err = QString("port closed.");
        return;
    }
    if (m_direction == QSerialPort::Input)
    {
        err = QString("PORT_MODE=[%1]").arg(strDirection());
        return;
    }

    qint64 n = write(ba);
    if (n < 0) err = QString("error writing, writed bytes: %1").arg(n);
}
bool LComPortObj::portOpened() const
{
    return isOpen();
}
int LComPortObj::openModeByDirection() const
{
    switch (m_direction)
    {
        case QSerialPort::Input: return QIODevice::ReadOnly;
        case QSerialPort::Output: return QIODevice::WriteOnly;
        default: break;
    }
    return QIODevice::ReadWrite;
}
QString LComPortObj::strDirection() const
{
    switch (m_direction)
    {
        case QSerialPort::Input: return QString("Only read");
        case QSerialPort::Output: return QString("Only write");
        default: break;
    }
    return QString("Read/Write");
}






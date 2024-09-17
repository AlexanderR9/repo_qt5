#include "mbtcpobj.h"

#include <QDebug>
#include <QTimerEvent>

/////////////// MBTcpObj ///////////////////
MBTcpObj::MBTcpObj(QObject *parent)
    :LSimpleObject(parent)
     //m_port(NULL),
     //m_direction(-1)
{
    /*
    m_port = new QSerialPort(this);
    m_readedBuffer.clear();

    ComParams params;
    setPortParams(params);

    connect(m_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(m_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotError()));

    int t_id = startTimer(6*1000);
    qDebug()<<QString("ComObj timer started: id=%1").arg(t_id);
    */

}

/*
void ComObj::timerEvent(QTimerEvent *event)
{
    qDebug()<<QString("ComObj::timerEvent: timerId %1").arg(event->timerId());
    if (!portOpened()) return;

    if (readyBytes() > 0)
        qDebug()<<QString("ComObj::timerEvent()  %1").arg(readyBytes());
}
void ComObj::slotReadyRead()
{
    m_readedBuffer.clear();

    qDebug()<<QString("ComObj::slotReadyRead()  %1").arg(readyBytes());
    m_readedBuffer = m_port->readAll();
    emit signalMsg(QString("received data, readed %1 bytes").arg(m_readedBuffer.count()));

}
int ComObj::readyBytes() const
{
    return m_port->bytesAvailable();
}
void ComObj::slotError()
{
    int code = m_port->error();
    if (code == QSerialPort::NoError) return;
    qDebug()<<QString("ComObj::slotError()  code=%1").arg(m_port->error());

    emit signalError(QString("com_port error, code=%1").arg(code));
}
void ComObj::setPortParams(const ComParams &params)
{
    m_port->setPortName(params.port_name);
    m_port->setBaudRate(params.baud_rate, QSerialPort::Direction(params.direction));
    m_port->setDataBits(QSerialPort::DataBits(params.data_bits));
    m_port->setStopBits(QSerialPort::StopBits(params.stop_bits));
    m_port->setFlowControl(QSerialPort::FlowControl(params.flow_control));
    m_port->setParity(QSerialPort::Parity(params.parity));
    m_direction = params.direction;
}
QString ComObj::portName() const
{
    return m_port->portName();
}
void ComObj::tryOpen(bool &ok)
{
    m_port->clearError();

    ok = false;
    QString err;
    if (portOpened())
    {
        err = QString("port allready opened.");
        emit signalError(err);
        return;
    }

    ok = m_port->open(QIODevice::OpenMode(openModeByDirection()));
    if (!ok)
    {
        err = QString("error opening port.");
        emit signalError(err);
    }
    else emit signalMsg(QString("port opened, mode=[%1]").arg(strDirection()));
}
void ComObj::tryClose()
{
    QString err;
    if (!portOpened())
    {
        err = QString("port allready closed.");
        emit signalError(err);
        return;
    }
    m_port->close();
    emit signalMsg("port closed!");
}
void ComObj::tryWrite(const QByteArray &ba)
{
    QString err;
    if (!portOpened())
    {
        err = QString("port closed.");
        emit signalError(err);
        return;
    }

    qint64 n = m_port->write(ba);
    emit signalMsg(QString("writen to port %1 bytes").arg(n));
    if (n < 0 && m_direction == QSerialPort::Input)
        emit signalError("open mode: [Only read]");

}
bool ComObj::portOpened() const
{
    return m_port->isOpen();
}
int ComObj::openModeByDirection() const
{
    switch (m_direction)
    {
        case QSerialPort::Input: return QIODevice::ReadOnly;
        case QSerialPort::Output: return QIODevice::WriteOnly;
        default: break;
    }
    return QIODevice::ReadWrite;
}
QString ComObj::strDirection() const
{
    switch (m_direction)
    {
        case QSerialPort::Input: return QString("Only read");
        case QSerialPort::Output: return QString("Only write");
        default: break;
    }
    return QString("Read/Write");
}
*/





#include "tcpclientobj.h"
#include "ltime.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QTimer>



// LTcpClientObj
LTcpClientObj::LTcpClientObj(QObject *parent)
    :LSimpleObject(parent),
    m_clientSocket(NULL),
    m_host(QString()),
    m_port(9999),
    m_readOnly(false),
    m_connectTimeout(0),
    m_waitTimeoutTimer(NULL)
{
    setObjectName("l_tcp_client");
    setConnectTimeout();

    initClient();

    m_waitTimeoutTimer = new QTimer(this);
    m_waitTimeoutTimer->stop();
    connect(m_waitTimeoutTimer, SIGNAL(timeout()), this, SLOT(slotConnectionTimeout()));

}
void LTcpClientObj::slotConnectionTimeout()
{
    qDebug("LTcpClientObj::slotConnectionTimeout()  connection timeout is OVER!!!!");
    m_waitTimeoutTimer->stop();
    abortSocket();

    emit signalEvent("timeout");
}
void LTcpClientObj::setConnectionParams(QString host, quint16 port)
{
    if (!isDisconnected()) return;

    m_host = host;
    m_port = port;
}
void LTcpClientObj::setReadOnly(bool b)
{
    if (!isDisconnected()) return;

    m_readOnly = b;
}
void LTcpClientObj::resetConnectionParams()
{
    if (!isDisconnected()) return;

    m_host.clear();
    m_port = 0;
}
void LTcpClientObj::initClient()
{
    m_clientSocket = new QTcpSocket(this);
    m_clientSocket->setObjectName("client_socket");

    connect(m_clientSocket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(m_clientSocket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(m_clientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError()));
    connect(m_clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotSocketStateChanged()));
    connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyRead()));

}
void LTcpClientObj::slotSocketConnected()
{
    if (m_waitTimeoutTimer->isActive()) m_waitTimeoutTimer->stop();

    qDebug()<<QString("LTcpClientObj::slotSocketConnected()  sender=[%1]").arg(sender()->objectName());
    emit signalMsg(QString("%0: %1 conected").arg(name()).arg(sender()->objectName()));
    emit signalEvent("connected");
}
void LTcpClientObj::slotSocketDisconnected()
{
    if (m_waitTimeoutTimer->isActive()) m_waitTimeoutTimer->stop();

    qDebug()<<QString("LTcpClientObj::slotSocketDisconnected()  sender=[%1]").arg(sender()->objectName());
    emit signalMsg(QString("%0: %1 disconected").arg(name()).arg(sender()->objectName()));
    emit signalEvent("disconnected");

}
void LTcpClientObj::slotSocketError()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    int err_code = (socket ? socket->error() : -99);
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(err_code);
    qWarning()<<QString("LTcpClientObj::slotSocketError() WARNING  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    emit signalError(QString("%0: was error of %1, code=%2").arg(name()).arg(m_clientSocket->errorString()).arg(err_code));
    emit signalEvent("error");
}
void LTcpClientObj::slotSocketStateChanged()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString s_state = QString("state=%1").arg(socket ? QString::number(socket->state()) : "?");
    //qDebug()<<QString("LTcpClientObj::slotSocketStateChanged()  sender=[%1]  %2").arg(sender()->objectName()).arg(s_state);
}
void LTcpClientObj::slotSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) qWarning("LTcpClientObj::slotSocketReadyRead() - WARNING: socket is null");

    QString msg = QString("%1 ready read, bytes available %2").arg(socket->objectName()).arg(socket->bytesAvailable());
    emit signalMsg(QString("%0: %1").arg(name()).arg(msg));

    QByteArray ba = socket->readAll();
    emit signalMsg(QString("%0: readed bytes %1").arg(name()).arg(ba.count()));
    emit signalPackReceived(ba);
}
bool LTcpClientObj::isConnected() const
{
    return (m_clientSocket->state() == QAbstractSocket::ConnectedState);
}
bool LTcpClientObj::isDisconnected() const
{
    return (m_clientSocket->state() == QAbstractSocket::UnconnectedState);
}
bool LTcpClientObj::isConnecting() const
{
    return (m_clientSocket->state() == QAbstractSocket::ConnectingState);
}
void LTcpClientObj::tryConnect()
{
    // check busy
    if (isConnected())
    {
        emit signalError(QString("%0: client allready connected").arg(name()));
        return;
    }

    //check connection params
    m_host = m_host.trimmed();
    if (invalidConnectionParams())
    {
        emit signalError(QString("%0: conection params is invalid (%2)").arg(name()).arg(strConnectionParams()));
        return;
    }

    // try connect
    if (m_connectTimeout > 100) m_waitTimeoutTimer->start(m_connectTimeout);
    QIODevice::OpenMode mode = (isReadOnly() ? QIODevice::ReadOnly : QIODevice::ReadWrite);
    m_clientSocket->connectToHost(QHostAddress(m_host), m_port, mode);
}
void LTcpClientObj::abortSocket()
{
    if (m_waitTimeoutTimer->isActive()) m_waitTimeoutTimer->stop();
    if (isDisconnected()) return;

    if (m_clientSocket)
    {
         m_clientSocket->abort();
    }
}
QString LTcpClientObj::strState() const
{
    if (!m_clientSocket) return QString("null");
    if (isConnected()) return QString("connected");
    if (isDisconnected()) return QString("unconnected");

    switch (m_clientSocket->state())
    {
        case QAbstractSocket::ConnectingState: return QString("connecting");
        case QAbstractSocket::ClosingState: return QString("closing");
        case QAbstractSocket::ListeningState: return QString("listening");
        case QAbstractSocket::BoundState: return QString("bound");
        default: break;
    }
    return QString("unknown");
}
QString LTcpClientObj::strConnectionParams() const
{
    QString ro_mode = (isReadOnly() ? "true" : "false");
    QString s("LTcpClientObj CONNECTION_PARAMS:");
    s = QString("%1 host[%2] port[%3]  readonly_mode[%4]").arg(s).arg(hostValue()).arg(portValue()).arg(ro_mode);
    return s;
}
void LTcpClientObj::tryDisconnect()
{
    if (!isConnected())
    {
        emit signalError(QString("%0: client allready disconnected").arg(name()));
        return;
    }
    m_clientSocket->disconnectFromHost();
}
void LTcpClientObj::trySendPacket(const QByteArray &ba)
{
    if (ba.isEmpty())
    {
        emit signalError(QString("%0: packet size is empty").arg(name()));
        return;
    }

    qint64 n_bytes = m_clientSocket->write(ba);
    if (n_bytes == ba.size()) emit signalMsg(QString("%0: success sended packet (%1 bytes)").arg(name()).arg(n_bytes));
    else emit signalError(QString("%0: wrong sending packet(%1 bytes), writed bytes %2").arg(name()).arg(ba.size()).arg(n_bytes));
}





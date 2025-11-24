#include "localclientobj.h"
#include "ltime.h"

#include <QLocalSocket>
#include <QDebug>



// LLocalClientObj
LLocalClientObj::LLocalClientObj(QObject *parent)
    :LSimpleObject(parent),
    m_clientSocket(NULL),
    m_serverName("ltestserv"),
    m_blockSize(4)
{
    setObjectName("l_local_client");

    initClient();

}
void LLocalClientObj::initClient()
{
    m_clientSocket = new QLocalSocket(this);
    m_clientSocket->setObjectName("local_client_socket");

    connect(m_clientSocket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(m_clientSocket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(m_clientSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(slotSocketError()));
    connect(m_clientSocket, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), this, SLOT(slotSocketStateChanged()));
    connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyRead()));

}
void LLocalClientObj::tryConnect()
{
    // check busy
    if (isConnected())
    {
        emit signalError(QString("%0: client allready connected").arg(name()));
        return;
    }
    //check connection params
    if (fileServerName().isEmpty())
    {
        emit signalError(QString("%0: conection params is invalid, file_server_name is empty").arg(name()));
        return;
    }

    // try connect
    m_clientSocket->connectToServer(fileServerName());
}
void LLocalClientObj::tryDisconnect()
{
    if (!isConnected())
    {
        emit signalError(QString("%0: client allready disconnected").arg(name()));
        return;
    }
    m_clientSocket->disconnectFromServer();
}
void LLocalClientObj::slotSocketConnected()
{
    qDebug()<<QString("LLocalClientObj::slotSocketConnected()  sender=[%1]").arg(sender()->objectName());
    emit signalMsg(QString("%0: %1 conected").arg(name()).arg(sender()->objectName()));
    emit signalEvent("connected");
}
void LLocalClientObj::slotSocketDisconnected()
{
    qDebug()<<QString("LLocalClientObj::slotSocketDisconnected()  sender=[%1]").arg(sender()->objectName());
    emit signalMsg(QString("%0: %1 disconected").arg(name()).arg(sender()->objectName()));
    emit signalEvent("disconnected");
}
void LLocalClientObj::slotSocketError()
{
    const QLocalSocket *socket = qobject_cast<const QLocalSocket*>(sender());
    int err_code = (socket ? socket->error() : -99);
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(err_code);
    qWarning()<<QString("LLocalClientObj::slotSocketError() WARNING  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    emit signalError(QString("%0: was error of %1, code=%2").arg(name()).arg(m_clientSocket->errorString()).arg(err_code));
    emit signalEvent("error");
}
void LLocalClientObj::slotSocketStateChanged()
{
    const QLocalSocket *socket = qobject_cast<const QLocalSocket*>(sender());
    QString s_state = QString("state=%1").arg(socket ? QString::number(socket->state()) : "?");
    //qDebug()<<QString("LLocalClientObj::slotSocketStateChanged()  sender=[%1]  %2").arg(sender()->objectName()).arg(s_state);
}
void LLocalClientObj::slotSocketReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) qWarning("LLocalClientObj::slotSocketReadyRead() - WARNING: socket is null");

    quint32 n = socket->bytesAvailable();
    QString msg = QString("%1 ready read, bytes available %2").arg(socket->objectName()).arg(n);
    emit signalMsg(QString("%0: %1").arg(name()).arg(msg));
    if (n < dataBlockSize()) return;


    QByteArray ba = socket->readAll();
    emit signalMsg(QString("%0: readed bytes %1").arg(name()).arg(ba.count()));
    emit signalDataReceived(ba);
}
bool LLocalClientObj::isConnected() const
{
    return (m_clientSocket->state() == QLocalSocket::ConnectedState);
}
bool LLocalClientObj::isDisconnected() const
{
    return (m_clientSocket->state() == QLocalSocket::UnconnectedState);
}
bool LLocalClientObj::isConnecting() const
{
    return (m_clientSocket->state() == QLocalSocket::ConnectingState);
}
void LLocalClientObj::abortSocket()
{
    if (isDisconnected()) return;

    if (m_clientSocket)
    {
         m_clientSocket->abort();
    }
}
QString LLocalClientObj::strState() const
{
    if (!m_clientSocket) return QString("null");
    if (isConnected()) return QString("connected");
    if (isDisconnected()) return QString("unconnected");

    switch (m_clientSocket->state())
    {
        case QLocalSocket::ConnectingState: return QString("connecting");
        case QLocalSocket::ClosingState: return QString("closing");
        default: break;
    }
    return QString("unknown");
}
void LLocalClientObj::trySendData(const QByteArray &ba)
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



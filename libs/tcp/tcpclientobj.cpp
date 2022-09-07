#include "tcpclientobj.h"

#include <QTcpSocket>
#include <QHostAddress>



// LTcpClientObj
LTcpClientObj::LTcpClientObj(QObject *parent)
    :LSimpleObject(parent),
    m_clientSocket(NULL),
    m_host(QString()),
    m_port(9999),
    m_readOnly(false)
{
    setObjectName("l_tcp_client");

    initClient();

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
    qDebug()<<QString("LTcpClientObj::slotSocketConnected()  sender=[%1]").arg(sender()->objectName());
    emit signalMsg(QString("%0: %1 conected").arg(name()).arg(sender()->objectName()));

}
void LTcpClientObj::slotSocketDisconnected()
{
    qDebug()<<QString("LTcpClientObj::slotSocketDisconnected()  sender=[%1]").arg(sender()->objectName());
    emit signalMsg(QString("%0: %1 disconected").arg(name()).arg(sender()->objectName()));
}
void LTcpClientObj::slotSocketError()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(socket ? socket->error() : -99);
    qDebug()<<QString("LTcpClientObj::slotSocketError()  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    emit signalError(QString("%0: was error of %1, %2").arg(name()).arg(m_clientSocket->errorString()));
}
void LTcpClientObj::slotSocketStateChanged()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString s_state = QString("state=%1").arg(socket ? QString::number(socket->state()) : "?");
    qDebug()<<QString("LTcpClientObj::slotSocketStateChanged()  sender=[%1]  %2").arg(sender()->objectName()).arg(s_state);

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
bool LTcpClientObj::isConnecting() const
{
    return (m_clientSocket->state() == QAbstractSocket::ConnectingState);
}
void LTcpClientObj::tryConnect()
{
    if (isConnected())
    {
        emit signalError(QString("%0: client allready connected").arg(name()));
        return;
    }
    if (m_host.trimmed().isEmpty())
    {
        emit signalError(QString("%0: conection host is empty").arg(name()));
        return;
    }

     QIODevice::OpenMode mode = (m_readOnly ? QIODevice::ReadOnly : QIODevice::ReadWrite);
     m_clientSocket->connectToHost(QHostAddress(m_host), m_port, mode);
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





#include "lwebsocketserver.h"
#include "lstring.h"

#include <QWebSocket>
#include <QWebSocketServer>
#include <QDebug>


// LWebSocketServer constructor
LWebSocketServer::LWebSocketServer(quint16 port, QObject *parent)
    :LSimpleObject(parent),
    m_server(NULL),
    m_port(port)
{
    setObjectName("lwebsocket_server");

    // init ws_serv obj
    m_server = new QWebSocketServer(QStringLiteral("WS_Server"), QWebSocketServer::NonSecureMode, this);
    connect(m_server, SIGNAL(serverError(QWebSocketProtocol::CloseCode)), this, SLOT(slotServerError()));
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotServerNewConnection()));

}
void LWebSocketServer::closeServer()
{
    qDebug("LWebSocketServer::closeServer() 1");
    if (m_server->isListening()) m_server->close();
    if (m_clients.isEmpty()) return;

    foreach (QWebSocket *socket, m_clients)
    {
        // Отключите соединение с клиентом
        if(socket->state() != QAbstractSocket::UnconnectedState)
            socket->abort();
    }
    qDeleteAll(m_clients);
    m_clients.clear();
    qDebug("LWebSocketServer::closeServer() 4");
}
void LWebSocketServer::slotServerNewConnection()
{
    emit signalMsg(QString("%0: was new connection").arg(name()));


    QWebSocket *socket = m_server->nextPendingConnection();
    socket->setObjectName(nextSocketName());
    m_clients << socket;
    qDebug() << "Was new connection, current clients: " << m_clients.size();


    connect(socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError()));
    //connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotSocketStateChanged()));
    connect(socket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(slotTextMessageReceived(const QString&)));
}
void LWebSocketServer::slotServerError()
{
    int err = m_server->error();
    QString s_err = m_server->errorString();

    emit signalError(QString("%0: %1  (code=%2)").arg(name()).arg(s_err).arg(err));
    //m_errCounter++;
}
void LWebSocketServer::slotTextMessageReceived(const QString &msg)
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) qWarning("LWebSocketServer::slotSocketReadyRead() - WARNING: socket is null");

    QString text = QString("%1: text msg received, msg_size=%2").arg(socket->objectName()).arg(msg.length());
    emit signalMsg(QString("%0: %1").arg(name()).arg(text));

    int i_socket = socketIndexOf(socket->objectName());
    emit signalTextMsgReceived(i_socket, msg);
}
int LWebSocketServer::socketIndexOf(const QString &s_name) const
{
    if (m_clients.isEmpty() || s_name.trimmed().isEmpty()) return -1;
    for (int i=0; i<m_clients.count(); i++)
    {
        if (!m_clients.at(i)) continue;
        if (m_clients.at(i)->objectName() == s_name) return i;
    }
    return -1;
}
void LWebSocketServer::slotSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket)
    {
        QString s_name = socket->objectName().trimmed();
        m_clients.removeAll(socket);
        socket->deleteLater();
        qDebug() << QString("Отключился клиент(%1), осталось: %2").arg(s_name).arg(m_clients.size());
        emit signalError(QString("%0: %1 disconected,  client_index=%2").arg(name()).arg(s_name).arg(socketIndexOf(s_name)));
    }
}
void LWebSocketServer::slotSocketError()
{
    const QWebSocket *socket = qobject_cast<const QWebSocket*>(sender());
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(socket ? socket->error() : -99);
    qDebug()<<QString("LWebSocketServer::slotSocketError()  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    emit signalError(QString("%0: (SOCKET_ERR)  sender=[%1]  ERR: %2").arg(name()).arg(sender()->objectName()).arg(err));
    //m_errCounter++;
}
void LWebSocketServer::startListening(quint8 max_clients)
{
    if (isListening())
    {
        emit signalError(QString("%0: server allready listening").arg(name()));
        return;
    }

    m_server->setMaxPendingConnections(max_clients);
    qDebug()<<QString("LWebSocketServer::startListening()  host=%1  port=%2").arg("ANY").arg(m_port);
    bool result = m_server->listen(QHostAddress::Any, m_port);

    QString text = QString("%0: started listening").arg(name());
    text = QString("%1 (HOST=%2  PORT=%3)").arg(text).arg("any").arg(m_port);
    emit signalMsg(text);

    if (!result)
    {
        emit signalError(QString("%0: RESULT=[fault],  err: %1").arg(name()).arg(m_server->errorString()));
        //m_errCounter++;
    }
}
void LWebSocketServer::stopListening()
{
    if (!m_server->isListening())
    {
        emit signalError(QString("%0: server is stoped").arg(name()));
        return;
    }

    qDebug("LWebSocketServer::stopListening() 1");
    closeServer();
    emit signalMsg(QString("%0: stoped listening").arg(name()));
    qDebug("LWebSocketServer::stopListening() 2");
}
bool LWebSocketServer::isListening() const
{
    return m_server->isListening();
}
bool LWebSocketServer::hasConnectedClients() const
{
    if (m_clients.isEmpty()) return false;
    foreach (QWebSocket *socket, m_clients)
    {
        if (!socket) continue;
        if(socket->state() == QAbstractSocket::ConnectedState) return true;
    }
    return false;
}



//private funcs
quint8 LWebSocketServer::nextSocketNumber() const
{
    if (m_clients.isEmpty()) return 1;

    const QWebSocket *socket = m_clients.last();
    if (!socket) return 1;
    QString s_name = socket->objectName().trimmed();
    int pos = s_name.lastIndexOf("_");
    if (pos < 0) return 1;

    bool ok;
    quint8 n = LString::strTrimLeft(s_name, pos+1).trimmed().toUInt(&ok);
    if (!ok || n < 1) return 1;
    return (n + 1);
}
QString LWebSocketServer::nextSocketName() const
{
    return QString("ws_client_%1").arg(nextSocketNumber());
}


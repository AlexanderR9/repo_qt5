#include "tcpserverobj.h"
#include "lstring.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>


// LTcpServerObj constructor
LTcpServerObj::LTcpServerObj(QObject *parent)
    :LSimpleObject(parent),
    m_server(NULL),
    m_errCounter(0)
{
    setObjectName("ltcp_server");
    resetParams();
    initServer();

}
void LTcpServerObj::initServer()
{
    m_server = new QTcpServer(this);

    connect(m_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(slotServerError()));
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotServerNewConnection()));
}
void LTcpServerObj::resetParams()
{
    m_listenHost = QString();
    m_listenPort = 0;
    m_maxConnections = 5;
}
void LTcpServerObj::slotServerNewConnection()
{
    emit signalMsg(QString("%0: was new connection").arg(name()));

    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket)
    {
        emit signalError(QString("%0: connected socket is null").arg(name()));
        m_errCounter++;
    }
    else
    {
        addConnectedSocket(socket);
        emit signalMsg(QString("%0: connected socket_name: [%1]").arg(name()).arg(socket->objectName()));
    }
}
void LTcpServerObj::slotServerError()
{
    int err = m_server->serverError();
    QString s_err = m_server->errorString();

    emit signalError(QString("%0: %1  (code=%2)").arg(name()).arg(s_err).arg(err));
    m_errCounter++;
}
void LTcpServerObj::startListening()
{
    if (m_server->isListening())
    {
        emit signalError(QString("%0: server allready listening").arg(name()));
        return;
    }

    m_server->setMaxPendingConnections(m_maxConnections);
    qDebug()<<QString("LTcpServerObj::startListening()  host=%1  port=%2").arg(m_listenHost).arg(m_listenPort);
    //emit signalMsg(QString("TcpServer starting listen - host=%1  port=%2").arg(m_listenHost).arg(m_listenPort));

    bool result = false;

    result = m_server->listen(QHostAddress::Any, m_listenPort);
    //if (m_listenHost.trimmed().isEmpty()) result = m_server->listen(QHostAddress::Any, m_listenPort);
    //else result = m_server->listen(QHostAddress(m_listenHost), m_listenPort);

    QString msg = QString("%0: started listening").arg(name());
    msg = QString("%1 (HOST=%2  PORT=%3)").arg(msg).arg(m_listenHost.trimmed().isEmpty()?"any":m_listenHost).arg(m_listenPort);
    emit signalMsg(msg);

    if (!result)
    {
        emit signalError(QString("%0: RESULT=[fault],  err: %1").arg(name()).arg(m_server->errorString()));
        m_errCounter++;
    }
}
void LTcpServerObj::stopListening()
{
    if (!m_server->isListening())
    {
        emit signalError(QString("%0: server is stoped").arg(name()));
        return;
    }

    qDebug("LTcpServerObj::stopListening() 1");
    closeServer();
    emit signalMsg(QString("%0: stoped listening").arg(name()));

    //m_server->deleteLater();
    qDebug("LTcpServerObj::stopListening() 2");
    QTest::qWait(200);
    qDebug("LTcpServerObj::stopListening() 3");
    //m_server = NULL;
    qDebug("LTcpServerObj::stopListening() 4");
    //initServer();
    qDebug("LTcpServerObj::stopListening() 5");
}
quint8 LTcpServerObj::nextSocketNumber() const
{
    if (m_sockets.isEmpty()) return 1;
    const QTcpSocket *socket = m_sockets.last();
    if (!socket) return 1;
    QString s_name = socket->objectName().trimmed();
    int pos = s_name.lastIndexOf("_");
    if (pos < 0) return 1;

    bool ok;
    quint8 n = LString::strTrimLeft(s_name, pos+1).trimmed().toUInt(&ok);
    if (!ok || n < 1) return 1;
    return (n + 1);
}
QString LTcpServerObj::nextSocketName() const
{
    return QString("client_%1").arg(nextSocketNumber());
}
int LTcpServerObj::socketIndexOf(const QString &s_name) const
{
    if (m_sockets.isEmpty() || s_name.trimmed().isEmpty()) return -1;
    for (int i=0; i<m_sockets.count(); i++)
    {
        if (!m_sockets.at(i)) continue;
        if (m_sockets.at(i)->objectName() == s_name) return i;
    }
    return -1;
}
void LTcpServerObj::addConnectedSocket(QTcpSocket *socket)
{
    socket->setObjectName(nextSocketName());
    m_sockets.append(socket);

    //connect(socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotSocketStateChanged()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyRead()));

}
/*
void LTcpServerObj::slotSocketConnected()
{
    qDebug()<<QString("LTcpServerObj::slotSocketConnected()  sender=[%1]").arg(sender()->objectName());

}
*/
void LTcpServerObj::slotSocketDisconnected()
{
    qDebug()<<QString("LTcpServerObj::slotSocketDisconnected()  sender=[%1]").arg(sender()->objectName());

    QString s_name = sender()->objectName().trimmed();
    int pos = socketIndexOf(s_name);
    emit signalError(QString("%0: %1 disconected,  client_index=%2").arg(name()).arg(s_name).arg(pos));

    if (pos >= 0)
    {
        if (m_sockets.at(pos) && m_sockets.at(pos)->state() == QAbstractSocket::ConnectedState)
        {
            qDebug()<<QString("----------------ATTANTION socket was connected----------------------");
            m_sockets[pos]->close();
            QTest::qWait(200);
        }
        /*
        if (m_sockets.at(pos)->isOpen())
            m_sockets[pos]->close();
        QTest::qWait(200);
        */


        QTcpSocket *socket = m_sockets.takeAt(pos);
        if (socket) socket->deleteLater();
        emit signalMsg(QString("%0: current clients %1").arg(name()).arg(clientsCount()));
    }
}
void LTcpServerObj::slotSocketError()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(socket ? socket->error() : -99);
    qDebug()<<QString("LTcpServerObj::slotSocketError()  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    emit signalError(QString("%0: (SOCKET_ERR)  sender=[%1]  ERR: %2").arg(name()).arg(sender()->objectName()).arg(err));
    m_errCounter++;
}
void LTcpServerObj::slotSocketStateChanged()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString s_state = QString("state=%1").arg(socket ? QString::number(socket->state()) : "?");
    qDebug()<<QString("LTcpServerObj::slotSocketStateChanged()  sender=[%1]  %2").arg(sender()->objectName()).arg(s_state);

}
void LTcpServerObj::slotSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) qWarning("LTcpServerObj::slotSocketReadyRead() - WARNING: socket is null");

    QString msg = QString("%1 ready read, bytes available %2").arg(socket->objectName()).arg(socket->bytesAvailable());
    emit signalMsg(QString("%0: %1").arg(name()).arg(msg));

    QByteArray ba = socket->readAll();
    emit signalMsg(QString("%0: readed bytes %1").arg(name()).arg(ba.count()));

    emit signalPackReceived(ba);
}
bool LTcpServerObj::hasConnectedClients() const
{
    if (m_sockets.isEmpty()) return false;
    foreach (QTcpSocket *socket, m_sockets)
    {
        if (!socket) continue;
        if(socket->state() == QAbstractSocket::ConnectedState)
            return true;
    }
    return false;
}
QString LTcpServerObj::connectedHostAt(int i) const
{
    if (i < 0 || i >= m_sockets.count()) return "??";

    QHostAddress ip4(m_sockets.at(i)->peerAddress().toIPv4Address());
    return ip4.toString();
}
void LTcpServerObj::trySendPacketToClients(const QByteArray &ba, bool &ok)
{
    ok = false;
    if (ba.isEmpty())
    {
        emit signalError(QString("%0: packet size is empty").arg(name()));
        return;
    }
    if (clientsCount() == 0)
    {
        emit signalError(QString("%0: connected clients is empty").arg(name()));
        return;
    }

    //try send packet
    foreach (QTcpSocket *v, m_sockets)
    {
        if (v)
        {
            qint64 n_bytes = v->write(ba);
            if (n_bytes == ba.size())
            {
                emit signalMsg(QString("%0: success sended packet (%1 bytes) to %2").arg(name()).arg(n_bytes).arg(v->objectName()));
                ok = true;
            }
            else
            {
                emit signalError(QString("%0: wrong sending packet(%1 bytes) to %2, writed bytes %3").arg(name()).arg(ba.size()).arg(v->objectName()).arg(n_bytes));
                m_errCounter++;
            }
        }
    }
}
void LTcpServerObj::trySendPacketToClient(quint8 socket_number, const QByteArray &ba, bool &ok)
{
    ok = false;
    if (ba.isEmpty())
    {
        emit signalError(QString("%0: packet size is empty").arg(name()));
        return;
    }

    QString s_name = QString("client_%1").arg(socket_number);
    int pos = socketIndexOf(s_name);
    if (pos < 0)
    {
        emit signalError(QString("%0: socket by number %1 not found").arg(name()).arg(socket_number));
        return;
    }

    if (!m_sockets.at(pos))
    {
        emit signalError(QString("%0: socket by number %1 is null").arg(name()).arg(socket_number));
        return;
    }

    qint64 n_bytes = m_sockets[pos]->write(ba);
    if (n_bytes == ba.size())
    {
        emit signalMsg(QString("%0: success sended packet (%1 bytes) to %2").arg(name()).arg(n_bytes).arg(s_name));
        ok = true;
    }
    else
    {
        emit signalError(QString("%0: wrong sending packet(%1 bytes) to %2, writed bytes %3").arg(name()).arg(ba.size()).arg(s_name).arg(n_bytes));
        m_errCounter++;
    }
}
bool LTcpServerObj::isListening() const
{
    return m_server->isListening();
}
void LTcpServerObj::closeServer()
{
    qDebug("LTcpServerObj::closeServer() 1");
    if (m_server->isListening()) m_server->close();
    qDebug("LTcpServerObj::closeServer() 2");
    if (m_sockets.isEmpty()) return;

    foreach (QTcpSocket *socket, m_sockets)
    {
        // Отключите соединение с клиентом
        socket->disconnectFromHost();
        if(socket->state() != QAbstractSocket::UnconnectedState)
            socket->abort();
    }
    qDebug("LTcpServerObj::closeServer() 3");
    qDeleteAll(m_sockets);
    m_sockets.clear();
    qDebug("LTcpServerObj::closeServer() 4");
}


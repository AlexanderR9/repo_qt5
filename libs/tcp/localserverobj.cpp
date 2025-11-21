#include "localserverobj.h"
#include "lstring.h"

#include <QLocalServer>
#include <QLocalSocket>
//#include <QTest>

#include <QFile>


// LLocalServerObj constructor
LLocalServerObj::LLocalServerObj(QObject *parent)
    :LSimpleObject(parent),
    m_server(NULL),
    m_errCounter(0),
    m_serverName("ltestserv"),
    m_maxConnections(2),
    m_blockSize(4)
{
    setObjectName("llocal_server");

    m_server = new QLocalServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotServerNewConnection()));


}
void LLocalServerObj::slotServerNewConnection()
{
    emit signalMsg(QString("%0: was new connection").arg(name()));
    emit signalEvent("new_client_connected");


    QLocalSocket *socket = m_server->nextPendingConnection();
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
void LLocalServerObj::addConnectedSocket(QLocalSocket *socket)
{
    qDebug()<<QString("LLocalServerObj::addConnectedSocket  QLocalSocket name [%1]").arg(socket->objectName());
    //socket->setObjectName(nextSocketName());
    m_sockets.append(socket);

    //connect(socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(slotSocketError()));
    connect(socket, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), this, SLOT(slotSocketStateChanged()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyRead()));

}
void LLocalServerObj::startListening()
{
    if (m_server->isListening())
    {
        emit signalError(QString("%0: server allready listening").arg(name()));
        return;
    }
    if (fileServerName().isEmpty())
    {
        emit signalError(QString("%0: file_server_name is empty").arg(name()));
        return;
    }

    // На всякий случай удалим старый сокет, если остался
    QFile::remove(fileServerName());

    //try listening ON
    m_server->setMaxPendingConnections(m_maxConnections);
    qDebug()<<QString("LLocalServerObj::startListening()  file_server_name[%1]").arg(fileServerName());
    bool result = m_server->listen(fileServerName());
    if (!result)
    {
        QString msg = QString("%0: RESULT=[fault],  err: %1").arg(name()).arg(m_server->errorString());
        emit signalError(msg);
        qCritical()<<QString("[LOCAL_SERVER] Can't listen : ")<<msg;
        m_errCounter++;
    }
    else qDebug() << QString("[LOCAL_SERVER] Listening on file_server_name[%1]").arg(fileServerName());

}
void LLocalServerObj::stopListening()
{
    if (!m_server->isListening())
    {
        emit signalError(QString("%0: server is stoped already").arg(name()));
        return;
    }

    qDebug("LLocalServerObj::stopListening() 1");
    closeServer();
    emit signalMsg(QString("%0: stoped listening").arg(name()));
    qDebug("LLocalServerObj::stopListening() 2");



    /*

    //m_server->deleteLater();
    qDebug("LTcpServerObj::stopListening() 2");
    QTest::qWait(200);
    qDebug("LTcpServerObj::stopListening() 3");
    //m_server = NULL;
    qDebug("LTcpServerObj::stopListening() 4");
    //initServer();
    qDebug("LTcpServerObj::stopListening() 5");
    */
}
bool LLocalServerObj::isListening() const
{
    return m_server->isListening();
}





/*
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
*/

int LLocalServerObj::socketIndexOf(const QString &s_name) const
{
    if (m_sockets.isEmpty() || s_name.trimmed().isEmpty()) return -1;
    for (int i=0; i<m_sockets.count(); i++)
    {
        if (!m_sockets.at(i)) continue;
        if (m_sockets.at(i)->objectName() == s_name) return i;
    }
    return -1;
}
void LLocalServerObj::slotSocketDisconnected()
{
    qDebug()<<QString("LLocalServerObj::slotSocketDisconnected()  sender=[%1]").arg(sender()->objectName());

    QString s_name = sender()->objectName().trimmed();
    int pos = socketIndexOf(s_name);
    emit signalError(QString("%0: %1 disconected,  client_index=%2").arg(name()).arg(s_name).arg(pos));
    emit signalEvent("client_disconected");

    if (pos >= 0)
    {
        /*
        if (m_sockets.at(pos) && m_sockets.at(pos)->state() == QAbstractSocket::ConnectedState)
        {
            qDebug()<<QString("----------------ATTANTION socket was connected----------------------");
            m_sockets[pos]->close();
            //QTest::qWait(200);

        }
        */


        QLocalSocket *socket = m_sockets.takeAt(pos);
        if (socket) socket->deleteLater();
        emit signalMsg(QString("%0: current clients %1").arg(name()).arg(clientsCount()));
    }
}
void LLocalServerObj::slotSocketError()
{
    const QLocalSocket *socket = qobject_cast<const QLocalSocket*>(sender());
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(socket ? socket->error() : -99);
    qDebug()<<QString("LLocalServerObj::slotSocketError()  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    emit signalError(QString("%0: (LOCAL_SOCKET_ERR)  sender=[%1]  ERR: %2").arg(name()).arg(sender()->objectName()).arg(err));
    emit signalEvent("client_err");

    m_errCounter++;
}
void LLocalServerObj::slotSocketStateChanged()
{
    const QLocalSocket *socket = qobject_cast<const QLocalSocket*>(sender());
    QString s_state = QString("state=%1").arg(socket ? QString::number(socket->state()) : "?");
    qDebug()<<QString("LLocalServerObj::slotSocketStateChanged()  sender=[%1]  %2").arg(sender()->objectName()).arg(s_state);

}
void LLocalServerObj::slotSocketReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) qWarning("LLocalServerObj::slotSocketReadyRead() - WARNING: socket is null");

    quint32 n = socket->bytesAvailable();
    QString msg = QString("%1 ready read, bytes available %2").arg(socket->objectName()).arg(n);
    emit signalMsg(QString("%0: %1").arg(name()).arg(msg));
    if (n < dataBlockSize()) return;

    QByteArray ba = socket->readAll();
    emit signalMsg(QString("%0: readed bytes %1").arg(name()).arg(ba.count()));
    if (ba.isEmpty()) return;

    int client_index = socketIndexOf(socket->objectName());
    emit signalDataReceived(client_index, ba);

}
bool LLocalServerObj::hasConnectedClients() const
{
    if (m_sockets.isEmpty()) return false;
    return true;

    /*
    foreach (QTcpSocket *socket, m_sockets)
    {
        if (!socket) continue;
        if(socket->state() == QAbstractSocket::ConnectedState)
            return true;
    }
    return false;
    */
}
/*
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
*/


void LLocalServerObj::closeServer()
{
    qDebug("LLocalServerObj::closeServer() 1");
    if (isListening()) m_server->close();
    qDebug("LLocalServerObj::closeServer() 2");
    if (m_sockets.isEmpty()) return;

    foreach (QLocalSocket *socket, m_sockets)
    {
        if (socket->state() == QLocalSocket::UnconnectedState) continue;

        // Отключите соединение с клиентом
        socket->disconnectFromServer();
        if (socket->waitForDisconnected(500)) qDebug("QLocalSocket Disconnected!");
        if (socket->state() != QLocalSocket::UnconnectedState) socket->abort();
    }
    qDebug("LLocalServerObj::closeServer() 3");
    qDeleteAll(m_sockets);
    m_sockets.clear();
    qDebug("LLocalServerObj::closeServer() 4");
}





#include "httpserverobj.h"
#include "lstring.h"
#include "lstatic.h"

#include <QDebug>
#include <QByteArray>
#include <QTcpSocket>
#include <QFile>
#include <QDir>



// LHttpServerObj constructor
LHttpServerObj::LHttpServerObj(quint16 port, QObject *parent)
    :LTcpServerObj(parent),
      m_wwwPath("www")
{
    setObjectName("lhttp_server");
    qDebug("LHttpServerObj created!");

    setMaxServerClients(1);
    setConnectionParams(QString(), port);
}
/*
LHttpServerObj::LHttpServerObj(QObject *parent)
    :LTcpServerObj(parent),
      m_wwwPath("www")
{
    setObjectName("lhttp_server");
    qDebug("LHttpServerObj created!");

}
*/
void LHttpServerObj::httpReqReceived(int i_socket, const QByteArray &req_data)
{
    if (i_socket < 0)
    {
        qWarning("LHttpServerObj::httpReqReceived WARNING: invalid socket index (-1)");
        emit signalError("LHttpServerObj: invalid socket index (-1)");
        return;
    }

    QTcpSocket *s = m_sockets[i_socket];
    qDebug()<<QString("LHttpServerObj::slotHttpReqReceived  req_data size %1").arg(req_data.size());
    if (!req_data.startsWith("GET "))
    {
        qWarning()<<QString("WARNING: 404, invalid req");
        QByteArray notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        s->write(notFound);
        return;
    }

    QString str_req = QString::fromUtf8(req_data).trimmed().toLower();
    qDebug()<<QString("STR_REQ: %1").arg(str_req);
    QStringList headers = LString::trimSplitList(str_req, "\r\n"); //разделить заголовки запроса
    if (headers.count() < 3)
    {
        qWarning()<<QString("WARNING: 404, invalid req");
        QByteArray notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 headers too small";
        s->write(notFound);
        return;
    }

    //parsing headers
    LHttpReqParams params;
    params.socket_name = s->objectName();
    parseHttpHeaders(headers, params);

    //diag debug
    qDebug()<<QString("req headers %1").arg(headers.count());
    foreach (const QString &v, headers) qDebug()<<"   "<<v;

    processHttpReq(params);
}
void LHttpServerObj::parseHttpHeaders(const QStringList &headers, LHttpReqParams &params)
{
    QString h = headers.first().trimmed();
    if (h.indexOf("get") == 0) params.kind = "get";
    h = LString::strTrimLeft(h, 3).trimmed();

    int pos = h.indexOf(" ");
    if (pos > 0) h = h.left(pos).trimmed();
    params.path = h;

    foreach (const QString &v, headers)
    {
        if (v.trimmed().indexOf("host:") == 0)
        {
            params.host = LString::strTrimLeft(v, 5).trimmed();
            break;
        }
    }
}
void LHttpServerObj::processHttpReq(const LHttpReqParams &params)
{
    qDebug("---------------processHttpReq--------------------");
    qDebug()<<params.toStr();

    QString page = LString::strTrimLeft(params.path, 1).trimmed();
    QString contentType = "text/html";
    if (page == "" || page == "index") page = "index.html";
    else contentType = params.replyContentType();

    qDebug()<<QString("REPLY: page[%1]  contentType[%2]").arg(page).arg(contentType);

    int i_socket = socketIndexOf(params.socket_name);
    QTcpSocket *s = m_sockets[i_socket];


    QFile f(QString("%1%2%3").arg(m_wwwPath).arg(QDir::separator()).arg(page));
    if (!f.exists())
    {
        qWarning()<<QString("WARNING: 404, invalid req, not found page [%1]").arg(f.fileName());
        QByteArray notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        s->write(notFound);
        return;
    }

    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning()<<QString("WARNING: 404, invalid req");
        QByteArray notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Can't read page file";
        s->write(notFound);
        return;
    }

    QByteArray content = f.readAll();
    f.close();

    //prepare reply
    QByteArray reply = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType.toUtf8();
    reply += ("\r\nContent-Length: " + QByteArray::number(content.size()) + "\r\n\r\n");
    reply += content;

    //send reply to browser
    s->write(reply);
}
void LHttpServerObj::slotServerNewConnection()
{
    LTcpServerObj::slotServerNewConnection();
    qDebug()<<QString("LHttpServerObj::slotServerNewConnection()  sockets %1").arg(clientsCount());
}
void LHttpServerObj::slotServerError()
{
    LTcpServerObj::slotServerError();
    qDebug("LHttpServerObj::slotServerError()");
}
void LHttpServerObj::slotSocketDisconnected()
{
    LTcpServerObj::slotSocketDisconnected();
    qDebug()<<QString("LHttpServerObj::slotSocketDisconnected()  sockets %1").arg(clientsCount());

}
void LHttpServerObj::slotSocketError()
{
    LTcpServerObj::slotSocketError();
    qDebug("LHttpServerObj::slotSocketError()");
}
void LHttpServerObj::slotSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) qWarning("LHttpServerObj::slotSocketReadyRead() - WARNING: socket is null");

    QByteArray ba = socket->readAll();
    qDebug()<<QString("LHttpServerObj::slotSocketReadyRead(), socket[%1], data_size %2 bytes").arg(socket->objectName()).arg(ba.size());
    int i_socket = socketIndexOf(socket->objectName());
    //emit signalMsg(QString("%0: readed bytes %1").arg(name()).arg(ba.count()));
    //emit signalPackReceived(ba);

    httpReqReceived(i_socket, ba);
}





/////////////////////////////////////////////////////////
QString LHttpReqParams::replyContentType() const
{
    QString contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".json")) contentType = "application/json";
    else if (path.endsWith(".png")) contentType = "image/png";
    else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) contentType = "image/jpeg";
    return contentType;
}









#ifndef HTTPSERVEROBJ_H
#define HTTPSERVEROBJ_H

#include "tcpserverobj.h"

class QByteArray;
class QTcpSocket;


//структура содержащая параметры пришедшего http запроса от браузера
struct LHttpReqParams
{
    LHttpReqParams() {reset();}

    QString kind; // get or post
    QString path; // example: some page.html or js file
    QString host;
    QString socket_name; // who from req

    void reset() {kind.clear(); path.clear(); host.clear(); socket_name.clear();}
    QString toStr() const {return QString("LHttpReqParams: kind[%1]  path[%2]  host[%3] socket[%4]").arg(kind).arg(path).arg(host).arg(socket_name);}
    QString replyContentType() const;

};




//класс LHttpServerObj нужен для обработки запросов http получаемых от обычного браузера для просмотра web страниц.
//по сути реализует web сервер, который отдает страницы сайта для просмотра.
//перед запуском прослушивания необходимо задать порт и переменную m_wwwPath (папка где лежит весь контент сайта).
//processHttpReq - функция где происходит отбработка запроса и выдача ответа подключенному сокету, ее можно переопределить под свои задачи.
//соединение между браузером и сервером строится на базе привычной связки  QTcpServer/QTcpSocket


// LHttpServerObj
class LHttpServerObj : public LTcpServerObj
{
    Q_OBJECT
public:
    //LHttpServerObj(QObject *parent = NULL);
    LHttpServerObj(quint16, QObject *parent = NULL);
    virtual ~LHttpServerObj() {}

    void setWebPath(QString wpath) {m_wwwPath = wpath.trimmed();}

protected:
    QString m_wwwPath; //путь к папке где хранятся все web файлы типа js, html, путь указывается относительно точки запуска ПО

    virtual void httpReqReceived(int, const QByteArray&); //пришел запрос от браузера(от сокета с указанным индексом) на выдачу страницы
    virtual void parseHttpHeaders(const QStringList&, LHttpReqParams&); //обработать заголовки запроса и записать необходимые поля в структуру
    virtual void processHttpReq(const LHttpReqParams&); //обработать заголовки запроса и отправить страницу обратно в браузер по указанному сокету


protected slots:
    virtual void slotServerNewConnection(); //произошло новое подключение к серверу
    virtual void slotServerError(); //произошла сетевая ошибка
    virtual void slotSocketDisconnected(); //выполняется когда клиент отключился
    virtual void slotSocketError(); //выполняется когда произошла сетевая ошибка у подключенного сокета
    virtual void slotSocketReadyRead(); //выполняется когда в сокет пришли данные от одного(любого) из подключенных сокетов



};

#endif // HTTPSERVEROBJ_H

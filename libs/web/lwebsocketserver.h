#ifndef LWEBSOCKETSERVER_H
#define LWEBSOCKETSERVER_H


#include "lsimpleobj.h"


class QWebSocketServer;
class QWebSocket;

//класс для взаимодействия с веб браузером, а точнее со скриптами js работающими на страницах html.
//при приходе сигнала disconnect от сокета, сокет удаляется из контейнера m_clients, и сам объект сокета тоже удаляется.

//LWebSocketServer
class LWebSocketServer : public LSimpleObject
{
    Q_OBJECT
public:
    LWebSocketServer(quint16, QObject *parent = NULL);
    virtual ~LWebSocketServer() {closeServer();}

    inline int clientsCount() const {return m_clients.count();} //количество подключенных сокетов(клиентов)
    inline quint16 listeningPort() const {return m_port;}

    virtual void startListening(quint8 max_clients = 5); //запустить прослушивание
    virtual void stopListening(); //остановить прослушивание
    virtual bool isListening() const;
    virtual bool hasConnectedClients() const; // признак наличия подключенных клиентов в текущий момент

protected:
    QWebSocketServer *m_server;
    QList<QWebSocket*> m_clients; //подключившиеся клиенты
    quint16 m_port; //порт на котором слушает сервер

    virtual void closeServer(); //сервер закрывает прослушку, разрывается соединение со всеми клиентами, очищается контейнер m_sockets
    int socketIndexOf(const QString&) const; // поиск сокета в контейнере m_clients по имени, вернет индекс элемента или -1

private:
    quint8 nextSocketNumber() const; //возвращает следующий номер подключенного сокета(клиента) 1..N
    QString nextSocketName() const; //возвращает следующеее назначенное имя подключенного сокета(клиента)

protected slots:
    virtual void slotServerNewConnection(); //произошло новое подключение к серверу
    virtual void slotServerError(); //произошла сетевая ошибка
    virtual void slotTextMessageReceived(const QString&); //пришло текстовое сообщение от одного из клиентов (от браузера)
    virtual void slotSocketDisconnected(); //выполняется когда клиент отключился
    virtual void slotSocketError(); //выполняется когда произошла сетевая ошибка у подключенного сокета


signals:
    void signalTextMsgReceived(int, const QString&); //имитится когда пришло текстовое сообщение от сокета с указанным индексом


};



#endif // LWEBSOCKETSERVER_H




#ifndef TCP_SERVER_OBJECT_H
#define TCP_SERVER_OBJECT_H

#include "lsimpleobj.h"


class QTcpServer;
class QTcpSocket;


//LTcpServerObj
class LTcpServerObj : public LSimpleObject
{
    Q_OBJECT
public:
    LTcpServerObj(QObject *parent = NULL);
    virtual ~LTcpServerObj() {closeServer();}

    inline void setConnectionParams(QString host, quint16 port = 0) {m_listenHost = host; m_listenPort = port;} //задать параметры для прослушивания сервера
    inline void setMaxServerClients(quint8 n) {m_maxConnections = n;} //задать максимально количество подключенных клиентов
    inline int clientsCount() const {return m_sockets.count();} //количество подключенных сокетов(клиентов)

    virtual void startListening(); //запустить прослушивание
    virtual void stopListening(); //остановить прослушивание
    virtual bool isListening() const;
    virtual void trySendPacketToClient(quint8, const QByteArray&); //отправить пакет клиенту с заданным номером
    virtual bool hasConnectedClients() const; // признак наличия подключенных клиентов в текущий момент

    virtual QString name() const {return QString("LTcpServer");}

protected:
    QTcpServer          *m_server;
    QList<QTcpSocket*>   m_sockets; // список подключившихся сокетов(клиентов)

    //сетевые настройки сервера
    QString     m_listenHost;
    quint16     m_listenPort;
    quint8      m_maxConnections;

    virtual void initServer();  //инициализация сервера
    virtual void resetParams(); //сброс настроек сервера
    virtual void addConnectedSocket(QTcpSocket*); //добавить подключившийся сокет в m_sockets
    virtual void closeServer(); //сервер закрывает прослушку, разрывается соединение со всеми клиентами, очищается контейнер m_sockets

    int socketIndexOf(const QString&) const; // поиск сокета в контейнере m_sockets по имени, вернет индекс элемента или -1

protected slots:
    virtual void slotServerNewConnection(); //произошло новое подключение к серверу
    virtual void slotServerError(); //произошла сетевая ошибка
    //virtual void slotSocketConnected(); //выполняется когда клиент подключился
    virtual void slotSocketDisconnected(); //выполняется когда клиент отключился
    virtual void slotSocketError(); //выполняется когда произошла сетевая ошибка у подключенного сокета
    virtual void slotSocketStateChanged(); //выполняется когда меняется состояние сокета
    virtual void slotSocketReadyRead(); //выполняется когда в сокет пришли данные

private:
    quint8 nextSocketNumber() const; //возвращает следующий номер подключенного сокета(клиента) 1..N
    QString nextSocketName() const; //возвращает следующеее назначенное имя подключенного сокета(клиента)

signals:
    void signalPackReceived(const QByteArray&);

};



#endif //TCP_SERVER_OBJECT_H



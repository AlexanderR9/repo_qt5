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

    void startListening(); //запустить прослушивание
    void stopListening(); //остановить прослушивание
    bool isListening() const;
    void trySendPacketToClient(quint8, const QByteArray&); //отправить пакет клиенту с заданным номером
    bool hasConnectedClients() const;

    virtual QString name() const {return QString("LTcpServer");}

protected:
    QTcpServer          *m_server;
    QList<QTcpSocket*>   m_sockets; // список подключившихся сокетов(клиентов)

    QString     m_listenHost;
    quint16     m_listenPort;
    quint8      m_maxConnections;

    void initServer();  //инициализация сервера
    void resetParams();
    void addConnectedSocket(QTcpSocket*);
    void closeServer();

protected slots:
    void slotServerNewConnection(); //произошло новое подключение к серверу
    void slotServerError(); //произошла сетевая ошибка
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketError();
    void slotSocketStateChanged();
    void slotSocketReadyRead();

private:
    quint8 nextSocketNumber() const; //возвращает следующий номер подключенного сокета(клиента) 1..N
    QString nextSocketName() const; //возвращает следующеее назначенное имя подключенного сокета(клиента)
    int socketIndexOf(const QString&) const; // поиск сокета в контейнере m_sockets по имени, вернет индекс элемента или -1

signals:
    void signalPackReceived(const QByteArray&);

};



#endif //TCP_SERVER_OBJECT_H



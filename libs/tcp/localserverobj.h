#ifndef LOCAL_SERVER_OBJECT_H
#define LOCAL_SERVER_OBJECT_H

#include "lsimpleobj.h"


class QLocalServer;
class QLocalSocket;


//LLocalServerObj
class LLocalServerObj : public LSimpleObject
{
    Q_OBJECT
public:
    LLocalServerObj(QObject *parent = NULL);
    virtual ~LLocalServerObj() {closeServer();}


    virtual QString name() const {return QString("LLocalServer");}


    virtual void startListening(); //запустить прослушивание
    virtual void stopListening(); //остановить прослушивание
    virtual bool isListening() const;
    virtual bool hasConnectedClients() const; // признак наличия подключенных клиентов в текущий момент

    /*
    virtual void trySendPacketToClient(quint8, const QByteArray&, bool&); //отправить пакет клиенту с заданным номером [1..N]
    virtual void trySendPacketToClients(const QByteArray&, bool&); //отправить пакет все подключившимся клиента

    */


    inline void setFileServerName(QString fname) {m_serverName = fname.trimmed();}
    inline QString fileServerName() const {return m_serverName.trimmed();}
    inline quint32 errCount() const {return m_errCounter;}
    inline void resetErrCounter() {m_errCounter = 0;}
    inline int clientsCount() const {return m_sockets.count();} //количество подключенных сокетов(клиентов)
    inline void setMaxClients(quint8 n) {m_maxConnections = n;} //задать максимально количество подключенных клиентов
    inline quint8 maxClientCount() const {return m_maxConnections;}
    inline void setBlockSize(quint16 a) {m_blockSize = a;} //задать минимальное количество байт в сокете которое можно читать
    inline quint16 dataBlockSize() const {return m_blockSize;}


protected:
    QLocalServer           *m_server;
    QList<QLocalSocket*>    m_sockets; // список подключившихся сокетов(клиентов)
    quint32                 m_errCounter; //счетчи ошибок
    QString                 m_serverName; // файловый дискриптор, по которому слушает сервер
    quint8                  m_maxConnections; // максимально допустимое количество подключенных клиентов
    quint16                 m_blockSize; // минимальное количество байт в сокете на которое реагировать, по умолчанию 4 байта

    virtual void addConnectedSocket(QLocalSocket*); //добавить подключившийся сокет в m_sockets
    virtual void closeServer(); //сервер закрывает прослушку, разрывается соединение со всеми клиентами, очищается контейнер m_sockets

    int socketIndexOf(const QString&) const; // поиск сокета в контейнере m_sockets по его имени, вернет индекс элемента или -1

protected slots:
    virtual void slotServerNewConnection(); //произошло новое подключение к серверу
    //virtual void slotServerError(); //произошла сетевая ошибка
    //virtual void slotSocketConnected(); //выполняется когда клиент подключился
    virtual void slotSocketDisconnected(); //выполняется когда клиент отключился, этот сокет удаляется из m_sockets
    virtual void slotSocketError(); //выполняется когда произошла сетевая ошибка у подключенного сокета
    virtual void slotSocketStateChanged(); //выполняется когда меняется состояние сокета
    virtual void slotSocketReadyRead(); //выполняется когда в сокет пришли данные

    /*
private:
    quint8 nextSocketNumber() const; //возвращает следующий номер подключенного сокета(клиента) 1..N
    QString nextSocketName() const; //возвращает следующеее назначенное имя подключенного сокета(клиента)
    */

signals:
    // выполняется когда пришли данные от любого из подключенных клиентов.
    // данные у сокет полностью вычитываются в QByteArray.
    // 1-й параметр это индекс сокета в m_sockets, или -1 если не был найден в с таким именем (sender objectName)
    void signalDataReceived(int, const QByteArray&);

    void signalEvent(QString); // msg example: new_client_connected/client_disconected

};



#endif // LOCAL_SERVER_OBJECT_H



#ifndef LOCAL_CLIENT_OBJECT_H
#define LOCAL_CLIENT_OBJECT_H


#include "lsimpleobj.h"

class QLocalSocket;
class QTimer;



//LLocalClientObj
class LLocalClientObj : public LSimpleObject
{
    Q_OBJECT
public:
    LLocalClientObj(QObject *parent = NULL);
    virtual ~LLocalClientObj() {}

    virtual QString name() const {return QString("LLocalClient");}


    bool isConnected() const; //вернет true если m_clientSocket подключен в текущий момент
    bool isDisconnected() const; //вернет true если m_clientSocket подключен в текущий момент
    bool isConnecting() const; //вернет true если m_clientSocket пытается подключиться в текущий момент

    virtual void tryConnect(); //попытаться подключиться к local_серверу
    virtual void tryDisconnect();
    virtual void trySendData(const QByteArray&); // попытаться записать данные в сокет, т.е. отправить пакет серверу

    //грубое прерывание соединения (если сокет подключен или в процессе подключения),
    //функция немедленно закрывает сокет, удаляя все данные в буфере
    virtual void abortSocket();


    //debug funcs
    QString strState() const; //текущее состояние сокета

    // изменения параметров соедиения,
    inline void setFileServerName(QString fname) {m_serverName = fname.trimmed();}
    inline QString fileServerName() const {return m_serverName.trimmed();}
    inline void setBlockSize(quint16 a) {m_blockSize = a;} //задать минимальное количество байт в сокете которое можно читать
    inline quint16 dataBlockSize() const {return m_blockSize;}

protected:
    QLocalSocket  *m_clientSocket; //сокет клиента
    QString        m_serverName; // файловый дискриптор, по которому слушает сервер
    quint16        m_blockSize; // минимальное количество байт в сокете на которое реагировать, по умолчанию 4 байта

    void initClient(); //инициализировать клиента

protected slots:
    virtual void slotSocketConnected();
    virtual void slotSocketDisconnected();
    virtual void slotSocketError();
    virtual void slotSocketStateChanged();
    virtual void slotSocketReadyRead();

signals:
    void signalDataReceived(const QByteArray&);
    void signalEvent(QString); // msg example: timeout/connected/disconnected/error


};



#endif // LOCAL_CLIENT_OBJECT_H



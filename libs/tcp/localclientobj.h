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

    void tryConnect(); //попытаться подключиться к local_серверу
    void tryDisconnect();
    void trySendData(const QByteArray&); // попытаться записать данные в сокет, т.е. отправить пакет серверу

    //грубое прерывание соединения (если сокет подключен или в процессе подключения),
    //функция немедленно закрывает сокет, удаляя все данные в буфере
    void abortSocket();


    //inline bool isReadOnly() const {return m_readOnly;}
    //void setReadOnly(bool); // задать режим обмена с хостом
    //void setConnectTimeout(quint32 interval = 5000) {m_connectTimeout = interval;}

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

    /*
    bool        m_readOnly; //если true, то режим обмена для клиента QIODevice::ReadOnly
    quint32     m_connectTimeout; //допустимое время подключения, если 0 то не отслеживается таймаут, отдается на откуп ОС
    // таймер ожидания подключения, нужен для прерывания подключения если сокет не смог подключится в течении m_connectTimeout
    QTimer *m_waitTimeoutTimer;
    */


protected slots:
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketError();
    void slotSocketStateChanged();
    void slotSocketReadyRead();
    //void slotConnectionTimeout(); // выполняется когда процесс подключения превысил время ожидания m_connectTimeout

signals:
    void signalDataReceived(const QByteArray&);
    void signalEvent(QString); // msg example: timeout/connected/disconnected/error


};



#endif // LOCAL_CLIENT_OBJECT_H



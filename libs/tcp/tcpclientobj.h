#ifndef TCP_CLIENT_OBJECT_H
#define TCP_CLIENT_OBJECT_H


#include "lsimpleobj.h"

class QTcpSocket;
class QTimer;



//LTcpClientObj
class LTcpClientObj : public LSimpleObject
{
    Q_OBJECT
public:
    LTcpClientObj(QObject *parent = NULL);
    virtual ~LTcpClientObj() {}

    virtual QString name() const {return QString("LTcpClient");}

    // изменения параметров соедиения,
    // значения применятся только если в текущий момент сокет не подключен
    void setConnectionParams(QString host, quint16 port = 0); // задать параметры для сетевого подключения
    void setReadOnly(bool); // задать режим обмена с хостом
    void resetConnectionParams();

    bool isConnected() const; //вернет true если m_clientSocket подключен в текущий момент
    bool isDisconnected() const; //вернет true если m_clientSocket подключен в текущий момент
    bool isConnecting() const; //вернет true если m_clientSocket пытается подключиться в текущий момент

    void tryConnect(); //попытаться подключиться к хосту
    void tryDisconnect();
    void trySendPacket(const QByteArray&); // попытаться отправить пакет подключенному хосту
    void setConnectTimeout(quint32 interval = 5000) {m_connectTimeout = interval;}

    //грубое прерывание соединения (если сокет подключен или в процессе подключения),
    //функция немедленно закрывает сокет, удаляя все данные в буфере
    void abortSocket();


    inline QString hostValue() const {return m_host;}
    inline quint16 portValue() const {return m_port;}
    inline bool isReadOnly() const {return m_readOnly;}
    inline bool invalidConnectionParams() const {return (m_host.trimmed().isEmpty() || (m_port == 0));}

    //debug funcs
    QString strState() const; //текущее состояние сокета
    QString strConnectionParams() const; // настройки соединения

protected:
    QTcpSocket  *m_clientSocket; //сокет клиента

    QString     m_host; // хост к которому клиент должен подключиться
    quint16     m_port; // порт по которому клиент должен подключиться
    bool        m_readOnly; //если true, то режим обмена для клиента QIODevice::ReadOnly
    quint32     m_connectTimeout; //допустимое время подключения, если 0 то не отслеживается таймаут, отдается на откуп ОС

    // таймер ожидания подключения, нужен для прерывания подключения если сокет не смог подключится в течении m_connectTimeout
    QTimer *m_waitTimeoutTimer;

    void initClient(); //инициализировать клиента

protected slots:
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketError();
    void slotSocketStateChanged();
    void slotSocketReadyRead();
    void slotConnectionTimeout(); // выполняется когда процесс подключения превысил время ожидания m_connectTimeout

signals:
    void signalPackReceived(const QByteArray&);
    void signalEvent(QString); // msg example: timeout/connected/disconnected/error


};



#endif //TCP_CLIENT_OBJECT_H



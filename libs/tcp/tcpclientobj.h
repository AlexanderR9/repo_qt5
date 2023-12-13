#ifndef TCP_CLIENT_OBJECT_H
#define TCP_CLIENT_OBJECT_H


#include "lsimpleobj.h"

class QTcpSocket;

//LTcpClientObj
class LTcpClientObj : public LSimpleObject
{
    Q_OBJECT
public:
    LTcpClientObj(QObject *parent = NULL);
    virtual ~LTcpClientObj() {}

    inline void setConnectionParams(QString host, quint16 port = 0) {m_host = host; m_port = port;} //задать параметры для сетевого подключения
    inline void setReadOnly(bool b) {m_readOnly = b;} //задать режим обмена с хостом

    bool isConnected() const; //вернет true если m_clientSocket подключен в текущий момент
    bool isDisconnected() const; //вернет true если m_clientSocket подключен в текущий момент
    bool isConnecting() const; //вернет true если m_clientSocket пытается подключиться в текущий момент

    void tryConnect(); //попытаться подключиться к хосту
    void tryDisconnect();
    void trySendPacket(const QByteArray&); // попытаться отправить пакет подключенному хосту
    void setConnectTimeout(quint32 interval = 5000) {m_connectTimeout = interval;}
    QString strState() const; //текущее состояние сокета
    void abortSocket(); //грубое прерывание соединения (если сокет подключен или в процессе подключения), функция немедленно закрывает сокет, удаляя все данные в буфере

    virtual QString name() const {return QString("LTcpClient");}

protected:
    QTcpSocket  *m_clientSocket; //сокет клиента

    QString     m_host; // хост к которому клиент должен подключиться
    quint16     m_port; // порт по которому клиент должен подключиться
    bool        m_readOnly; //если true, то режим обмена для клиента QIODevice::ReadOnly
    quint32     m_connectTimeout; //допустимое время подключения (пока не работает)

    void initClient(); //инициализировать клиента

protected slots:
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketError();
    void slotSocketStateChanged();
    void slotSocketReadyRead();

signals:
    void signalPackReceived(const QByteArray&);
    void signalEvent(QString);


};



#endif //TCP_CLIENT_OBJECT_H



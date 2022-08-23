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
    void tryConnect(); //попытаться подключиться к хосту
    void tryDisconnect();
    void trySendPacket(const QByteArray&); // попытаться отправить пакет подключенному хосту

    virtual QString name() const {return QString("LTcpClient");}

protected:
    QTcpSocket  *m_clientSocket; //сокет клиента

    QString     m_host; // хост к которому клиент должен подключиться
    quint16     m_port; // порт по которому клиент должен подключиться
    bool        m_readOnly; //если true, то режим обмена для клиента QIODevice::ReadOnly

    void initClient(); //инициализировать клиента

protected slots:
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketError();
    void slotSocketStateChanged();
    void slotSocketReadyRead();

signals:
    void signalPackReceived(const QByteArray&);


};



#endif //TCP_CLIENT_OBJECT_H



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
    virtual ~LTcpServerObj() {}

protected:
    QTcpServer          *m_server;
    QList<QTcpSocket*>   m_sockets;

    QString     m_listenHost;
    quint16     m_listenPort;
    quint8      m_maxConnections;


    void initServer();

};





#endif //TCP_SERVER_OBJECT_H



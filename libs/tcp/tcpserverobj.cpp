#include "tcpserverobj.h"

#include <QTcpServer>
#include <QTcpSocket>


// LTcpServerObj constructor
LTcpServerObj::LTcpServerObj(QObject *parent)
    :LSimpleObject(parent),
    m_server(NULL)
{
    setObjectName("ltcp_server");
    initServer();

}
void LTcpServerObj::initServer()
{
    m_server = new QTcpServer(this);

    //connect(m_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(slotServerError()));
    //connect(m_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}



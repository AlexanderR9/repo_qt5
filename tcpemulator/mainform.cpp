#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "lfile.h"


#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QProgressBar>
#include <QGroupBox>
#include <QStyleFactory>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>


//#define LISTEN_HOST     QString("192.168.10.6")
//uint LTcpServer::socketNumber = 1001;


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL)
    //m_server(NULL)
{
    setObjectName("main_form_tcpemulator");


//    QTimer *timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
//    timer->start(30000);

    //LTcpServer::initSocketNumber();


    initServer();
}
void MainForm::initServer()
{
    //LTcpServer::initSocketNumber();
/*
    m_server = new LTcpServer(this);

    connect(m_server, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_server, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
*/
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        case LMainWidget::atStop: {stop(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    m_protocol = new LProtocolBox(false, this);
    addWidget(m_protocol, 0, 0);

    /*
    QGroupBox *view_box = NULL;
    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter = new QSplitter(Qt::Horizontal, this);
    v_splitter->addWidget(html_box);
    v_splitter->addWidget(m_protocol);
    h_splitter->addWidget(v_splitter);

    if (view_box)
        h_splitter->addWidget(view_box);

    addWidget(h_splitter, 0, 0);
    */
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));
    
    QString key = QString("host");
    lCommonSettings.addParam(QString("Listening host"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString());

    key = QString("port");
    lCommonSettings.addParam(QString("Listening port"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("0"));

    key = QString("mode");
    lCommonSettings.addParam(QString("Emulator mode"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Server" << "Client";
    lCommonSettings.setComboList(key, combo_list);
    //lCommonSettings.setDefValue(key, QString("0"));

}
void MainForm::save()
{
    LMainWidget::save();

//    QSettings settings(companyName(), projectName());
//    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());
//    settings.setValue(QString("%1/h_splitter/state").arg(objectName()), h_splitter->saveState());
}
void MainForm::load()
{
    LMainWidget::load();

//    QSettings settings(companyName(), projectName());
//    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
//    if (!ba.isEmpty()) v_splitter->restoreState(ba);
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::start()
{
    /*
    if (m_server->emulatorStarted())
    {
        m_protocol->addText("TCP emulator already started, need stop", LProtocolBox::ttWarning);
        return;
    }

    QString mode = lCommonSettings.paramValue("mode").toString().trimmed().toLower();
    qDebug()<<QString("MODE=%1").arg(mode);

    m_server->setEmulatorMode((mode == "client") ? LTcpServer::emClient : LTcpServer::emServer);

    QString host = lCommonSettings.paramValue("host").toString().trimmed();
    quint16 port = lCommonSettings.paramValue("port").toUInt();
    m_server->setConnectionParams(host, port);
    m_server->startEmulator();
    */
}
void MainForm::stop()
{
   // m_server->stopEmulator();
}


/*

//LTcpServer
LTcpServer::LTcpServer(QObject *parent)
    :LSimpleObject(parent),
      m_server(NULL),
      m_client(NULL),
      m_host(QString()),
      m_port(0),
      m_maxConnections(3),
      m_mode(emServer)
{
    setObjectName("ltcp_server");
    initServer();
    initClient();

}
void LTcpServer::startListening()
{
    if (m_server->isListening())
    {
        emit signalError(QString("LTcpServer: server allready listening"));
        return;
    }

    bool result = false;
    if (m_host.trimmed().isEmpty()) result = m_server->listen(QHostAddress::Any, m_port);
    else result = m_server->listen(QHostAddress(m_host), m_port);

    QString msg = QString("LTcpServer: started listening");
    msg = QString("%1 (HOST=%2  PORT=%3)").arg(msg).arg(m_host.trimmed().isEmpty()?"any":m_host).arg(m_port);
    emit signalMsg(msg);

    if (!result)
        emit signalError("LTcpServer: RESULT=[fault]");
}
void LTcpServer::stopListening()
{
    if (!m_server->isListening())
    {
        emit signalError(QString("LTcpServer: server is stoped"));
        return;
    }

    m_server->close();
    qDeleteAll(m_sockets);
    m_sockets.clear();
    m_server->deleteLater();

    emit signalMsg(QString("LTcpServer: stoped listening"));

    QTest::qWait(200);
    m_server = NULL;

    initServer();
}
void LTcpServer::startEmulator()
{
    if (isClient()) startClient();
    else if (isServer()) startListening();
    else emit signalError(QString("LTcpServer: emulator is INVALID_MODE,  mode=(%1)"));
}
void LTcpServer::stopEmulator()
{
    if (isClient()) stopClient();
    else if (isServer()) stopListening();
    else emit signalError(QString("LTcpServer: emulator is INVALID_MODE,  mode=(%1)"));
}
bool LTcpServer::emulatorStarted() const
{
    if (isServer())
    {
        if (m_server->isListening()) return true;
    }
    else
    {
        if (m_client->isOpen()) return true;
    }
    return false;
}
void LTcpServer::stopClient()
{
    if (m_client->isOpen()) m_client->close();



}
void LTcpServer::startClient()
{
    if (!isClient())
    {
        emit signalError(QString("LTcpServer: emulator is not CLIENT_MODE,  mode=(%1)"));
        return;
    }

    tryCloseAllSockets();
    qDeleteAll(m_sockets);
    m_sockets.clear();

    if (m_host.trimmed().isEmpty())
    {
        emit signalError(QString("LTcpClient: host is empty"));
        return;
    }

    emit signalMsg("LTcpClient: try connecting ..............");
    m_client->connectToHost(QHostAddress(m_host), m_port, QIODevice::ReadOnly);

}
void LTcpServer::setEmulatorMode(int m)
{
    m_mode = m;
    if (m_mode != emServer && m_mode != emClient) m_mode = emServer;
}
void LTcpServer::initServer()
{
    m_server = new QTcpServer(this);

    connect(m_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(slotServerError()));
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}
void LTcpServer::initClient()
{
    m_client = new QTcpSocket(this);
    m_client->setObjectName("client");

    connect(m_client, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(m_client, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(m_client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError()));
    connect(m_client, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotSocketStateChanged()));

    connect(m_client, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}
void LTcpServer::slotReadyRead()
{
    qDebug()<<QString("LTcpServer::slotReadyRead() buffer: %1 bytes").arg(m_client->bytesAvailable());
    QByteArray ba(m_client->readAll());
}
void LTcpServer::tryCloseAllSockets()
{
    for (int i=0; i<m_sockets.count(); i++)
    {
        if (m_sockets.at(i))
            if (m_sockets.at(i)->isOpen())
                m_sockets.at(i)->close();
    }
}
void LTcpServer::slotNewConnection()
{
    emit signalMsg("LTcpServer: was new connection");

    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket)
    {
        emit signalError(QString("LTcpServer: connected socket is null"));
        return;
    }

    socket->setObjectName(QString("client_%1").arg(socketNumber));
    socketNumber++;
    emit signalMsg(QString("LTcpServer: connected socket_name: [%1]").arg(socket->objectName()));

    addConnectedSocket(socket);
}
void LTcpServer::slotServerError()
{
    int err = m_server->serverError();
    QString s_err = m_server->errorString();

    emit signalError(QString("LTcpServer: %1  (code=%2)").arg(s_err).arg(err));
}
void LTcpServer::addConnectedSocket(QTcpSocket *socket)
{
    m_sockets.append(socket);


    connect(socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotSocketStateChanged()));

}
void LTcpServer::slotSocketConnected()
{
    qDebug()<<QString("LTcpServer::slotSocketConnected()  sender=[%1]").arg(sender()->objectName());

    if (isClient())
    {
        emit signalMsg(QString("LTcpClient: connected"));
    }

}
void LTcpServer::slotSocketDisconnected()
{
    qDebug()<<QString("LTcpServer::slotSocketDisconnected()  sender=[%1]").arg(sender()->objectName());

    emit signalError(QString("LTcpServer: %1  disconected").arg(sender()->objectName()));

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) socket->close();
}
void LTcpServer::slotSocketError()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString err = QString("%1 (code=%2)").arg(socket ? socket->errorString() : "?????").arg(socket ? socket->error() : -99);
    qDebug()<<QString("LTcpServer::slotSocketError()  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err);

    if (isClient())
    {
        emit signalError(QString("LTcpServer::slotSocketError()  sender=[%1]  ERR: %2").arg(sender()->objectName()).arg(err));
    }
}
void LTcpServer::slotSocketStateChanged()
{
    const QTcpSocket *socket = qobject_cast<const QTcpSocket*>(sender());
    QString s_state = QString("state=%1").arg(socket ? QString::number(socket->state()) : "?");
    qDebug()<<QString("LTcpServer::slotSocketStateChanged()  sender=[%1]  %2").arg(sender()->objectName()).arg(s_state);

}
*/










#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"
#include "lsimpleobj.h"

class LProtocolBox;
class QSplitter;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class QTcpServer;
class QTcpSocket;


/*
// LTcpServer
class LTcpServer : public LSimpleObject
{
    Q_OBJECT
public:
    enum EmulatorMode {emServer = 0, emClient};


    LTcpServer(QObject*);
    virtual ~LTcpServer() {}

    inline void setConnectionParams(QString host, quint16 port = 0) {m_host = host; m_port = port;}
    inline void setMaxServerClients(quint8 n) {m_maxConnections = n;} //only server
    inline bool isClient() const {return (m_mode == emClient);}
    inline bool isServer() const {return (m_mode == emServer);}
    void setEmulatorMode(int);

    void startEmulator();
    void stopEmulator();
    bool emulatorStarted() const;


    static uint socketNumber;

protected:
    QTcpServer          *m_server;
    QList<QTcpSocket*>   m_sockets;

    QTcpSocket          *m_client;

    QString     m_host;
    quint16     m_port;
    quint8      m_maxConnections;
    int         m_mode;

    //static int socketNumber;

    void initServer();
    void initClient();
    void addConnectedSocket(QTcpSocket*);
    void tryCloseAllSockets();


    void startListening();
    void stopListening();
    void startClient();
    void stopClient();


protected slots:
    //for tcp_server
    void slotNewConnection();
    void slotServerError();

    //for tcp_sockets
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketError();
    void slotSocketStateChanged();
    void slotReadyRead();


};
*/



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox        *m_protocol;
    //LTcpServer          *m_server;

    QString projectName() const {return "tcpemulator";}
    QString mainTitle() const {return QString("TCP emulator (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void initServer();
    void start();
    void stop();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);



};




#endif


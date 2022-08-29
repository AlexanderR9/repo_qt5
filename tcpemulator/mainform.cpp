#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "lfile.h"
#include "tcpclientobj.h"
#include "tcpserverobj.h"

#include <stdio.h>
//#include "winsock2.h"


#pragma GCC diagnostic ignored "-Wsign-compare"

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
    m_protocol(NULL),
    m_server(NULL),
    m_client(NULL),
    m_mode(emServer),
    m_counter(0)
{
    setObjectName("main_form_tcpemulator");
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    timer->start(1000);

    initTcpObjects();
    PackHeader p_header;
    QByteArray ba;
    p_header.toByteArray(&ba);
    qDebug()<<QString("size PackHeader %1,   time_size %2,  ba size %3").arg(sizeof(p_header)).arg(sizeof(p_header.time)).arg(ba.size());


    PortalPackHeader h1;
    ARecord rec;
    h1.toByteArray(&ba);
    qDebug()<<QString("size PortalPackHeader %1,   ARecord %2,  ba size %3").arg(h1.size2()).arg(rec.size2()).arg(ba.size());


    prepareFloatPacket(ba);

}
void MainForm::slotTimer()
{
    //qDebug("MainForm::slotTimer() tick");
    //qDebug()<<QString("connected lients %1").arg(m_server->clientsCount());
    //return;

    if (!autoSendPack()) return;

    if (isServer())
    {
        sendPack();
        /*
        if (m_server->hasConnectedClients())
        {
            ba_header.clear();
            QDataStream stream(&ba_header, QIODevice::WriteOnly);
            for (int i=0; i<18; i++)
            {
                quint16 a = qrand()%100;
                stream << a;
            }
            qDebug()<<QString("MainForm::slotTimer()  ba_header size %1").arg(ba_header.size());
            m_server->trySendPacketToClient(1, ba_header);
        }
        */
    }
    if (isClient())
    {
        sendPack();
    }
}
void MainForm::initTcpObjects()
{
    m_server = new LTcpServerObj(this);
    connect(m_server, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_server, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(m_server, SIGNAL(signalPackReceived(const QByteArray&)), this, SLOT(slotServerPackReceived(const QByteArray&)));

    m_client = new LTcpClientObj(this);
    connect(m_client, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_client, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(m_client, SIGNAL(signalPackReceived(const QByteArray&)), this, SLOT(slotClientPackReceived(const QByteArray&)));
}
void MainForm::slotServerPackReceived(const QByteArray &ba)
{
    qDebug("MainForm::slotServerPackReceived");
    ba_header.clear();
    ba_header.append(ba);
    slotMsg(LStatic::baToStr(ba, 12));

}
void MainForm::slotClientPackReceived(const QByteArray &ba)
{
    qDebug("MainForm::slotClientPackReceived");
    ba_header.clear();
    ba_header.append(ba);
    slotMsg(LStatic::baToStr(ba, 12));
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atSendMsg);
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
        case LMainWidget::atSendMsg: {sendPack(); break;}
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
    lCommonSettings.addParam(QString("Host"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString());

    key = QString("port");
    lCommonSettings.addParam(QString("Port"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("0"));

    key = QString("mode");
    lCommonSettings.addParam(QString("Emulator mode"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Server" << "Client";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("autosendpack");
    lCommonSettings.addParam(QString("Auto send packet"), LSimpleDialog::sdtBool, key);

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
    m_protocol->addSpace();
    if (m_server->isListening())
    {
        m_protocol->addText("Server is listening now", LProtocolBox::ttWarning);
        return;
    }
    if (m_client->isConnected())
    {
        m_protocol->addText("Client connected now", LProtocolBox::ttWarning);
        return;
    }

    qDebug("///////////////////////////////////////");
    qDebug()<<QString("MainForm::start()  mode=%1").arg(m_mode);

    QString host = lCommonSettings.paramValue("host").toString().trimmed();
    quint16 port = lCommonSettings.paramValue("port").toUInt();
    m_mode = ((lCommonSettings.paramValue("mode").toString().trimmed().toLower() == "client") ? emClient : emServer);

    if (isClient())
    {
        m_client->setConnectionParams(host, port);
        m_client->tryConnect();
    }
    else
    {
        m_server->setConnectionParams(host, port);
        m_server->startListening();
    }
}
void MainForm::stop()
{
    if (m_server->isListening())
    {
        m_server->stopListening();
    }
    if (m_client->isConnected())
    {
        m_client->tryDisconnect();
    }
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    foreach (QString key, keys)
    {
        if (key == "mode")
            m_mode = ((lCommonSettings.paramValue("mode").toString().trimmed().toLower() == "client") ? emClient : emServer);
    }
}
bool MainForm::autoSendPack() const
{
    return lCommonSettings.paramValue("autosendpack").toBool();
}
void MainForm::sendPack()
{
    if (isServer() && m_server->hasConnectedClients())
    {
        //m_server->trySendPacketToClient(1, ba_header);

        QByteArray ba;
        prepareFloatPacket(ba);
        if (!ba.isEmpty())
            m_server->trySendPacketToClient(1, ba);

        prepareDiscretePacket(ba);
        if (!ba.isEmpty())
            m_server->trySendPacketToClient(1, ba);

    }
    if (isClient() && m_client->isConnected())
    {
        m_client->trySendPacket(ba_header);
    }
}
void MainForm::prepareFloatPacket(QByteArray &ba)
{
    ba.clear();
    if (QTime::currentTime().second()%2 == 1) return;

    PortalPackHeader header;
    header.subSys_id = QChar('I').toLatin1();
    header.dataType_id = QChar('A').toLatin1();
    header.counter = m_counter++;
    header.time.setCurrentTime();

    QByteArray ba_rec;
    QDataStream stream(&ba_rec, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    int n_group = 374;
    for (int i=0; i<n_group; i++)
    {
        ARecord f_rec(12.3+float(i)/10);
        f_rec.toByteArray(stream);
    }
    qDebug()<<QString("ba_rec %1").arg(ba_rec.size());


    //header.len = n_group*ARecord::size() + header.size();
    header.len = n_group;
    header.toByteArray(&ba);
    qDebug()<<QString("ba_header %1").arg(ba.size());
    ba.append(ba_rec);

    qDebug()<<header.toStr();
}
void MainForm::prepareDiscretePacket(QByteArray &ba)
{
    ba.clear();
    if (QTime::currentTime().second()%2 == 0) return;

    PortalPackHeader header;
    header.subSys_id = QChar('I').toLatin1();
    header.dataType_id = QChar('D').toLatin1();
    header.counter = m_counter++;
    header.time.setCurrentTime();

    QByteArray ba_rec;
    QDataStream stream(&ba_rec, QIODevice::WriteOnly);
    int n_group = 93;
    for (int i=0; i<n_group; i++)
    {
        DRecord d_rec(qrand()%2);
        d_rec.toByteArray(stream);
    }
    qDebug()<<QString("ba_rec(discrete) %1").arg(ba_rec.size());

    header.len = n_group;
    header.toByteArray(&ba);
    ba.append(ba_rec);
    qDebug()<<header.toStr();

}




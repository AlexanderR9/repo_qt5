#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "lfile.h"
#include "tcpclientobj.h"
#include "tcpserverobj.h"
//#include "dtsstructs.h"
#include "tcpstatuswidget.h"

#include <ctime>

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



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_statusWidget(NULL),
    m_server(NULL),
    m_client(NULL),
    m_mode(emServer),
    m_counter(101)
{
    setObjectName("main_form_tcpemulator");
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    timer->start(1000);

    initTcpObjects();
}
void MainForm::slotTimer()
{
    updateStatusWidget();
    updateButtonsState();

    if (!autoSendPack()) return;

    if (isServer())
    {
        sendPack();
    }
    if (isClient())
    {
        //sendPack();
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
    //ba_header.clear();
    //ba_header.append(ba);
    slotMsg(LStatic::baToStr(ba, 12));

}
void MainForm::slotClientPackReceived(const QByteArray &ba)
{
    qDebug("MainForm::slotClientPackReceived");
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
    addWidget(m_protocol, 0, 0, 1, 1);

    m_statusWidget = new LTCPStatusWidget(this);
    addWidget(m_statusWidget, 1, 0, 1, 1);

    updateButtonsState();

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
    lCommonSettings.setDefValue(key, QString("127.0.0.1"));

    key = QString("port");
    lCommonSettings.addParam(QString("Port"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("12345"));

    key = QString("qual");
    lCommonSettings.addParam(QString("Quality of signals"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("48"));

    key = QString("mode");
    lCommonSettings.addParam(QString("Emulator mode"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Server" << "Client";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("autosendpack");
    lCommonSettings.addParam(QString("Auto send packet"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, true);

    key = QString("f_recs");
    lCommonSettings.addParam(QString("Records count of group (float)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("374"));

    key = QString("d_recs");
    lCommonSettings.addParam(QString("Records count of group (discrete)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("93"));

    key = QString("byteorder");
    lCommonSettings.addParam(QString("DataStream bytes order"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "BigEndian" << "LitleEndian";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString("LitleEndian"));


}
void MainForm::save()
{
    LMainWidget::save();
}
void MainForm::load()
{
    LMainWidget::load();
    updateStatusWidget();
}
void MainForm::updateStatusWidget()
{
    if (m_server->isListening())
    {
        m_statusWidget->setTextMode("SERVER");
        if (m_server->hasConnectedClients())
        {
            m_statusWidget->setConnectedState();
            m_statusWidget->setTextMode(QString("SERVER  (connected %1 clients)").arg(m_server->clientsCount()));
        }
        else m_statusWidget->setListeningState();
        return;
    }

    if (m_client->isConnected())
    {
        m_statusWidget->setTextMode("CLIENT");
        m_statusWidget->setConnectedState();
        return;
    }

    m_statusWidget->setOffState();
    if (isClient()) m_statusWidget->setTextMode("CLIENT");
    else if (isServer()) m_statusWidget->setTextMode("SERVER");
    else m_statusWidget->setTextMode("INVALID MODE!!!");

}
void MainForm::updateButtonsState()
{
    bool started = (m_server->isListening() || m_client->isConnected() || m_client->isConnecting());
    getAction(atStart)->setEnabled(!started);
    getAction(atStop)->setEnabled(started);
    getAction(atSendMsg)->setEnabled(started);
    getAction(atSettings)->setEnabled(!started);
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

    QString host = lCommonSettings.paramValue("host").toString().trimmed();
    quint16 port = lCommonSettings.paramValue("port").toUInt();
    m_mode = ((lCommonSettings.paramValue("mode").toString().trimmed().toLower() == "client") ? emClient : emServer);

    updateStatusWidget();
    if (isClient())
    {
        m_client->setConnectionParams(host, port);
        m_client->tryConnect();
        m_statusWidget->setConnectingState();
    }
    else
    {
        m_server->setConnectionParams(host, port);
        m_server->startListening();
    }
    updateButtonsState();
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
    QTest::qWait(200);
    updateStatusWidget();
    updateButtonsState();
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    LMainWidget::slotAppSettingsChanged(keys);

    foreach (QString key, keys)
    {
        if (key == "mode")
        {
            m_mode = ((lCommonSettings.paramValue("mode").toString().trimmed().toLower() == "client") ? emClient : emServer);
            updateStatusWidget();
        }
    }
}
bool MainForm::autoSendPack() const
{
    return lCommonSettings.paramValue("autosendpack").toBool();
}
int MainForm::floatRecsCount() const
{
    return lCommonSettings.paramValue("f_recs").toInt();
}
int MainForm::discreteRecsCount() const
{
    return lCommonSettings.paramValue("d_recs").toInt();
}
int MainForm::byteOrder() const
{
    if (lCommonSettings.paramValue("byteorder").toString().toLower().contains("big")) return QDataStream::BigEndian;
    return QDataStream::LittleEndian;
}
quint16 MainForm::qualSig() const
{
    return lCommonSettings.paramValue("qual").toUInt();
}
void MainForm::sendPack()
{
    if (isServer() && m_server->hasConnectedClients())
    {
        /*
        QByteArray ba;
        prepareFloatPacket(ba);
        if (!ba.isEmpty())
            m_server->trySendPacketToClient(1, ba);

        prepareDiscretePacket(ba);
        if (!ba.isEmpty())
            m_server->trySendPacketToClient(1, ba);
            */

    }
    if (isClient() && m_client->isConnected())
    {
        //m_client->trySendPacket(ba_header);
    }
}


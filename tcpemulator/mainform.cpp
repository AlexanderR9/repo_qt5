#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "lfile.h"
#include "lstring.h"
#include "tcpclientobj.h"
#include "tcpserverobj.h"
#include "tcpstatuswidget.h"


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
    //qDebug("MainForm::slotServerPackReceived");
    receivedPackToProtolol(ba);
    m_protocol->addSpace();

}
void MainForm::slotClientPackReceived(const QByteArray &ba)
{
   // qDebug("MainForm::slotClientPackReceived");
    receivedPackToProtolol(ba);
    m_protocol->addSpace();
}
void MainForm::receivedPackToProtolol(const QByteArray &pack)
{
    if (pack.isEmpty())
    {
        slotError("received packed is empty");
        return;
    }

    int hs = shineHeaderSize();
    if (hs > 0)
    {
        if (pack.size() < hs) hs = pack.size();

        m_protocol->addText(QString("PACKET HEADER: %1 bytes").arg(hs), LProtocolBox::ttData);
        slotMsg(LStatic::baToStr(pack.left(hs), outLinePack()));
        m_protocol->addText(LString::symbolString(QChar('-'), 100), LProtocolBox::ttData);
        if (pack.size() > hs)
            slotMsg(LStatic::baToStr(pack.right(pack.size()-hs), outLinePack()));
    }
    else slotMsg(LStatic::baToStr(pack, outLinePack()));
}

void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atClear);
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
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
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

    key = QString("mode");
    lCommonSettings.addParam(QString("Emulator mode"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Server" << "Client";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("autosendpack");
    lCommonSettings.addParam(QString("Auto send packet"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, true);

    key = QString("byteorder");
    lCommonSettings.addParam(QString("DataStream bytes order"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "BigEndian" << "LitleEndian";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString("LitleEndian"));

    key = QString("shine_header");
    lCommonSettings.addParam(QString("Need shine header, bytes size"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "-1" << "16" << "20" << "24" << "32" << "36" << "40" << "48" << "64";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("out_line_size");
    lCommonSettings.addParam(QString("Need shine header, bytes size"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    for (int i=1; i<=16; i++) combo_list << QString::number(i*4);
    lCommonSettings.setComboList(key, combo_list);



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
int MainForm::byteOrder() const
{
    if (lCommonSettings.paramValue("byteorder").toString().toLower().contains("big")) return QDataStream::BigEndian;
    return QDataStream::LittleEndian;
}
int MainForm::shineHeaderSize() const
{
    return lCommonSettings.paramValue("shine_header").toInt();
}
qint16 MainForm::outLinePack() const
{
    return lCommonSettings.paramValue("out_line_size").toUInt();
}
void MainForm::sendPack()
{
    if (isServer() && m_server->hasConnectedClients())
    {
        QByteArray ba;
        // to do prepare BA

    }
    if (isClient() && m_client->isConnected())
    {
        //m_client->trySendPacket(ba_header);
    }
}


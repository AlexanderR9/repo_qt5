#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lstatic.h"
#include "mbtcpobj.h"
#include "mbtcpcentralwidget.h"

#include <QDebug>
//#include <QSerialPort>
#include <QByteArray>
#include <QSplitter>
#include <QModbusRequest>


MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_centralWidget(NULL),
    m_mbtcpObj(NULL)
{
    setObjectName("main_form_mbtcptest");

    initMBObj();
}
void MainForm::initMBObj()
{
    m_mbtcpObj = new MBTcpObj(this);

    connect(m_mbtcpObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_mbtcpObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart); //open port
    addAction(LMainWidget::atStop); //close port
    addAction(LMainWidget::atSendMsg); //write data to port
    addAction(LMainWidget::atClear); //clear protocol
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {startExchange(); break;}
        case LMainWidget::atStop: {stopExchange(); break;}
        case LMainWidget::atSendMsg: {sendReq(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_centralWidget = new MBTcpCentralWidget(this);
    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(m_mbtcpObj, SIGNAL(signalUpdateState(const QStringList&)), m_centralWidget, SLOT(slotUpdateState(const QStringList&)));
    connect(m_mbtcpObj, SIGNAL(signalFillReq(QModbusRequest&, quint8&, QString&)), m_centralWidget, SLOT(slotFillReq(QModbusRequest&, quint8&, QString&)));
    connect(m_mbtcpObj, SIGNAL(signalUpdateReqRespTable(const QStringList&, const QStringList&)), m_centralWidget, SLOT(slotUpdateReqRespTable(const QStringList&, const QStringList&)));
    connect(m_mbtcpObj, SIGNAL(signalUpdateRegTable(const QModbusResponse&)), m_centralWidget, SLOT(slotUpdateRegTable(const QModbusResponse&)));

    updateToolbar(false);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    combo_list << "Master" << "Slave";

    QString key = QString("mode");
    lCommonSettings.addParam(QString("Mode"), LSimpleDialog::sdtStringCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.last());

    key = QString("port");
    lCommonSettings.addParam(QString("TCP port"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, "1502");

    key = QString("ip");
    lCommonSettings.addParam(QString("IP address"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, "127.0.0.1");

    key = QString("do_count");
    lCommonSettings.addParam(QString("Registers count (DO)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);
    key = QString("di_count");
    lCommonSettings.addParam(QString("Registers count (DI)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);
    key = QString("ao_count");
    lCommonSettings.addParam(QString("Registers count (AO)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);
    key = QString("ai_count");
    lCommonSettings.addParam(QString("Registers count (AI)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);

}
void MainForm::startExchange()
{
    if (m_mbtcpObj)
    {
        m_mbtcpObj->reinit(mode());
        m_mbtcpObj->setNetworkParams(host(), port());
        m_mbtcpObj->start();
    }
    updateToolbar(true);
}
void MainForm::stopExchange()
{
    if (m_mbtcpObj)
    {
        m_mbtcpObj->stop();
    }
    updateToolbar(false);
}
void MainForm::sendReq()
{
    qDebug("MainForm::sendReq()");
    if (m_mbtcpObj) m_mbtcpObj->sendReq();
}
void MainForm::slotAppSettingsChanged(QStringList list)
{
    foreach (const QString &v, list) qDebug()<<QString("slotAppSettingsChanged: changed setting key: %1").arg(v);

    if (list.contains("mode"))
    {
        m_centralWidget->reinitWidget(mode());
    }
}
void MainForm::updateToolbar(bool is_start)
{
    getAction(LMainWidget::atSendMsg)->setEnabled(is_start && m_mbtcpObj->isMaster());
    getAction(LMainWidget::atStart)->setEnabled(!is_start);
    getAction(LMainWidget::atStop)->setEnabled(is_start);
    getAction(LMainWidget::atSettings)->setEnabled(!is_start);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text);
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    m_centralWidget->save(settings);
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_centralWidget->reinitWidget(mode());
    m_centralWidget->load(settings);

}

//private
int MainForm::mode() const
{
    QString s = lCommonSettings.paramValue("mode").toString().trimmed().toLower();
    return ((s == "master") ? 0 : 1);
}
quint16 MainForm::port() const
{
    return lCommonSettings.paramValue("port").toUInt();
}
QString MainForm::host() const
{
    return lCommonSettings.paramValue("ip").toString();
}
quint32 MainForm::do_count() const
{
    return lCommonSettings.paramValue("do_count").toUInt();
}
quint32 MainForm::di_count() const
{
    return lCommonSettings.paramValue("di_count").toUInt();
}
quint32 MainForm::ao_count() const
{
    return lCommonSettings.paramValue("ao_count").toUInt();
}
quint32 MainForm::ai_count() const
{
    return lCommonSettings.paramValue("ai_count").toUInt();
}



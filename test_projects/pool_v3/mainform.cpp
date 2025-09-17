#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "centralwidget_v3.h"
#include "deficonfig.h"
#include "deficonfigloader.h"
#include "appcommonsettings.h"

#include <QDebug>
#include <QSettings>
#include <QSplitter>


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_centralWidget(NULL),
    m_configLoader(NULL)
{
    setObjectName("poolsv3_mainwindow");

    m_configLoader = new DefiConfigLoader(this);
    connect(m_configLoader, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_configLoader, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
}
void MainForm::initActions()
{
    qDebug("MainForm::initActions()");
    //addAction(LMainWidget::atStart);
    addAction(LMainWidget::atRefresh);
    //addAction(LMainWidget::atMonitoring);
    //addAction(LMainWidget::atStop);
    addAction(LMainWidget::atClear);
    //addAction(LMainWidget::atSave);
    //addAction(LMainWidget::atLoadData);

    addToolBarSeparator();
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);

    this->setActionTooltip(atRefresh, "Update page data from chain");
    this->setActionTooltip(atClear, "Clear protocol");
    this->setActionTooltip(atSettings, "Application common settings");
    //this->setActionTooltip(atMonitoring, "Get state of positions with liquitydy");

}
void MainForm::slotAction(int type)
{
    switch (type)
    {  
        //case LMainWidget::atStart: {actStart(); break;}
        //case LMainWidget::atStop: {actStop(); break;}
        case LMainWidget::atRefresh: {actStartUpdating(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        //case LMainWidget::atSave: {actSaveData(); break;}
        //case LMainWidget::atLoadData: {actLoadData(); break;}
        //case LMainWidget::atMonitoring: {m_centralWidget->getStateLiqPos(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    qDebug("MainForm::initWidgets()");
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_centralWidget = new CentralWidgetV3(this);
    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(m_centralWidget, SIGNAL(signalEnableControls(bool)), this, SLOT(slotEnableControls(bool)));
    connect(m_centralWidget, SIGNAL(signalTabPageChanged(int)), this, SLOT(slotVisibleActionsUpdate(int)));

    //qDebug("MainForm::initWidgets() 1");
    slotEnableControls(true);
    //qDebug("MainForm::initWidgets() 2");
}
void MainForm::initCommonSettings()
{
    qDebug("MainForm::initCommonSettings()");
    QStringList combo_list;

    QString key = QString("defi_config");
    lCommonSettings.addParam(QString("Defi config file"), LSimpleDialog::sdtFilePath, key);

    key = QString("nodejs_path");
    lCommonSettings.addParam(QString("Path to Node_JS scripts"), LSimpleDialog::sdtDirPath, key);


    /*
    QString key = QString("req_delay");
    lCommonSettings.addParam(QString("Request delay"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "500" << "1000" << "2000" << "3000" << "5000" << "10000";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("view_expand");
    lCommonSettings.addParam(QString("View expand level"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "1" << "2" << "3" << "4" << "5";
    lCommonSettings.setComboList(key, combo_list);



    key = QString("req_interval");
    lCommonSettings.addParam(QString("Request sending interval, ms"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "500" << "1000" << "1500" << "2000" << "3000" << "5000" << "10000" << "20000";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(3));

    key = QString("delay_after_tx");
    lCommonSettings.addParam(QString("Delay after TX, secs"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    for (int i=1; i<10; i++) combo_list << QString::number(i*5);
    lCommonSettings.setComboList(key, combo_list);

*/
}
void MainForm::slotVisibleActionsUpdate(int p_kind)
{
    qDebug()<<QString("MainForm::slotVisibleActionsUpdate  p_kind=%1").arg(p_kind);
    switch(p_kind)
    {
        case dpkWallet:
        {
            getAction(atRefresh)->setVisible(true);
            break;
        }
        case dpkTx:
        {
            getAction(atRefresh)->setVisible(false);
            break;
        }
        case dpkApproved:
        {
            getAction(atRefresh)->setVisible(false);
            break;
        }
        default: break;
    }
}
/*
void MainForm::actStop()
{
    m_centralWidget->breakUpdating();
    m_protocol->addSpace();
    m_protocol->addText("breaked updating data of page!", LProtocolBox::ttWarning);
}
*/

void MainForm::actStartUpdating()
{
    m_protocol->addSpace();
    m_protocol->addText("Run updating data of page .......", LProtocolBox::ttFile);
    m_centralWidget->startUpdating();
}
void MainForm::save()
{
    qDebug("MainForm::save()");
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());
    m_centralWidget->save(settings);
}
void MainForm::load()
{
    qDebug("MainForm::load()");
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    AppCommonSettings::setNodejsPath(nodejsPath());

    //try load config
    m_configLoader->loadDefiConfiguration(defiConfig());

    //init central widget
    m_centralWidget->load(settings);
}
void MainForm::slotEnableControls(bool b)
{    
    //getAction(atStart)->setEnabled(b);
    //getAction(atStop)->setEnabled(!b);
    getAction(atRefresh)->setEnabled(b);
    //getAction(atMonitoring)->setEnabled(b);
    getAction(atSettings)->setEnabled(b);
    getAction(atClear)->setEnabled(b);

    m_centralWidget->setEnableControl(b);
}
void MainForm::slotAppSettingsChanged(QStringList list)
{
    qDebug("MainForm::slotAppSettingsChanged");
    LMainWidget::slotAppSettingsChanged(list);

    if (list.contains("nodejs_path"))
        AppCommonSettings::setNodejsPath(nodejsPath());

//    updateWindowTitle();
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
    m_protocol->moveScrollDown();
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
    m_protocol->moveScrollDown();
}


//private
QString MainForm::defiConfig() const
{
    return lCommonSettings.paramValue("defi_config").toString();
}
QString MainForm::nodejsPath() const
{
    return lCommonSettings.paramValue("nodejs_path").toString();
}

/*

int MainForm::expandLevel() const
{
    return lCommonSettings.paramValue("view_expand").toInt();
}
quint16 MainForm::pageUpdatingInterval() const
{
    return lCommonSettings.paramValue("page_updating_interval").toInt();
}
double MainForm::minTVL() const
{
    return lCommonSettings.paramValue("min_tvl").toDouble();
}
quint16 MainForm::minPoolAge() const
{
    return lCommonSettings.paramValue("min_age").toUInt();
}
quint16 MainForm::minPoolRatio() const
{
    return lCommonSettings.paramValue("min_ratio").toUInt();
}
quint16 MainForm::reqSize() const
{
    return lCommonSettings.paramValue("req_size").toUInt();
}
quint16 MainForm::reqInterval() const
{
    return lCommonSettings.paramValue("req_interval").toUInt();
}
quint8 MainForm::viewPrecision() const
{
    return lCommonSettings.paramValue("view_precision").toInt();
}
QString MainForm::apiKey() const
{
    return lCommonSettings.paramValue("apikey").toString();
}
QString MainForm::subgraphID() const
{
    QString chain = lCommonSettings.paramValue("subgraph_id").toString();
    int pos = chain.indexOf("(");
    if (pos > 0) chain = chain.left(pos).trimmed();
    foreach (const SubGraph_CommonSettings::SGFactory &f, sub_commonSettings.factories)
        if (f.chain == chain) return f.sub_id;
    return QString();
}
bool MainForm::usePreferTokens() const
{
    return lCommonSettings.paramValue("use_prefer_tokens").toBool();
}
QString MainForm::walletAddr() const
{
    return lCommonSettings.paramValue("wallet").toString();
}
quint16 MainForm::delayAfterTX() const
{
    return lCommonSettings.paramValue("delay_after_tx").toUInt();
}


*/


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
    addAction(LMainWidget::atRefresh);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atData);
    addToolBarSeparator();
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);

    this->setActionTooltip(atRefresh, "Update page data from chain");
    this->setActionTooltip(atClear, "Clear protocol");
    this->setActionTooltip(atSettings, "Application common settings");
    this->setActionTooltip(atData, "Mint position");
    this->setActionIcon(atData, QString("%1/modbus.png").arg(AppCommonSettings::commonIconsPath()));


}
void MainForm::slotAction(int type)
{
    switch (type)
    {  
        case LMainWidget::atRefresh: {actStartUpdating(); break;}
        case LMainWidget::atData: {actMintPos(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
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

    slotEnableControls(true);
}
void MainForm::initCommonSettings()
{
    qDebug("MainForm::initCommonSettings()");
    QStringList combo_list;

    QString key = QString("defi_config");
    lCommonSettings.addParam(QString("Defi config file"), LSimpleDialog::sdtFilePath, key);

    key = QString("nodejs_path");
    lCommonSettings.addParam(QString("Path to Node_JS scripts"), LSimpleDialog::sdtDirPath, key);

    key = QString("update_wallet");
    lCommonSettings.addParam(QString("Update wallet at startup"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);
}
void MainForm::slotVisibleActionsUpdate(int p_kind)
{
    qDebug()<<QString("MainForm::slotVisibleActionsUpdate  p_kind=%1").arg(p_kind);
    getAction(atData)->setVisible(false);
    switch(p_kind)
    {
        case dpkPositions:
        {
            getAction(atRefresh)->setVisible(true);
            getAction(atData)->setVisible(true);
            break;
        }
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
        case dpkBalance:
        case dpkPool:
        case dpkApproved:
        {
            getAction(atRefresh)->setVisible(false);
            break;
        }
        default: break;
    }
}
void MainForm::actStartUpdating()
{
    m_protocol->addSpace();
    m_protocol->addText("Run updating data of page .......", LProtocolBox::ttFile);
    m_centralWidget->startUpdating();
}
void MainForm::actMintPos()
{
    m_protocol->addSpace();
    m_centralWidget->mintPos();
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

    if (updateWalletAtStart())
        actStartUpdating();
}
void MainForm::slotEnableControls(bool b)
{    
    getAction(atRefresh)->setEnabled(b);
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
bool MainForm::updateWalletAtStart() const
{
    return lCommonSettings.paramValue("update_wallet").toBool();
}



#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "ug_centralwidget.h"
#include "ug_apistruct.h"
#include "subcommonsettings.h"

#include <QDebug>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QDateTime>



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_centralWidget(NULL)
{
    setObjectName("unigraph_mainwindow");

    quint32 a = 1622550974;
    QDateTime dt = QDateTime::fromSecsSinceEpoch(a);
    qDebug()<<dt.toString("yyyy.MM.dd  hh:mm:ss");

    QString err;
    sub_commonSettings.loadConfig(err);
    if (!err.isEmpty())
    {
        slotError(err);
        return;
    }
}
void MainForm::initActions()
{
    qDebug("MainForm::initActions()");
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atRefresh);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSave);
    addAction(LMainWidget::atLoadData);

    addToolBarSeparator();
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);

    this->setActionTooltip(atStart, "Send free query");
    this->setActionTooltip(atSave, "Save page data to file");
    this->setActionTooltip(atLoadData, "Load page data from file");

}
void MainForm::slotAction(int type)
{
    switch (type)
    {  
        case LMainWidget::atStart: {actStart(); break;}
        case LMainWidget::atStop: {actStop(); break;}
        case LMainWidget::atRefresh: {actStartUpdating(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        case LMainWidget::atSave: {actSaveData(); break;}
        case LMainWidget::atLoadData: {actLoadData(); break;}
        default: break;
    }
}
void MainForm::slotSetFilterParams(quint16 &req_limit, double &min_tvl)
{
    req_limit = reqSize();
    min_tvl = minTVL();
}
void MainForm::slotVisibleActionsUpdate(int p_type)
{
    switch(p_type)
    {
        case rtJsonView:
        {
            getAction(atStart)->setVisible(true);
            getAction(atRefresh)->setVisible(false);
            getAction(atLoadData)->setVisible(false);
            getAction(atSave)->setVisible(false);
            break;
        }
        case rtPools:
        {
            getAction(atStart)->setVisible(false);
            getAction(atRefresh)->setVisible(true);
            getAction(atLoadData)->setVisible(true);
            getAction(atSave)->setVisible(true);
            break;
        }
        case rtTokens:
        {
            getAction(atStart)->setVisible(false);
            getAction(atRefresh)->setVisible(true);
            getAction(atLoadData)->setVisible(true);
            getAction(atSave)->setVisible(true);
            break;
        }
        case rtDaysData:
        {
            getAction(atStart)->setVisible(false);
            getAction(atRefresh)->setVisible(true);
            getAction(atLoadData)->setVisible(true);
            getAction(atSave)->setVisible(false);
            break;
        }
        case rtPositions:
        {
            getAction(atStart)->setVisible(false);
            getAction(atRefresh)->setVisible(true);
            getAction(atLoadData)->setVisible(false);
            getAction(atSave)->setVisible(false);
            break;
        }

        default: break;
    }
}
void MainForm::actStart()
{
    m_protocol->addSpace();
    m_protocol->addText("Send free query .......", LProtocolBox::ttFile);

    if (m_centralWidget)
    {
        m_centralWidget->setViewPrecision(viewPrecision());
        m_centralWidget->setExpandLevel(expandLevel());
        m_centralWidget->updateDataPage();
    }
}
void MainForm::actStartUpdating()
{
    m_protocol->addSpace();
    m_protocol->addText("Run updatating data of page .......", LProtocolBox::ttFile);

    if (m_centralWidget)
    {
        m_centralWidget->setViewPrecision(viewPrecision());
        m_centralWidget->setExpandLevel(expandLevel());
        m_centralWidget->updateDataPage(reqInterval());
    }
}
void MainForm::actSaveData()
{
    m_protocol->addSpace();
    m_protocol->addText("Try save page data to file .......", LProtocolBox::ttFile);
    m_centralWidget->actSaveData();
}
void MainForm::actLoadData()
{
    m_protocol->addSpace();
    m_protocol->addText("Try load page data from file .......", LProtocolBox::ttFile);
    m_centralWidget->actLoadData();
}
void MainForm::actStop()
{
    if (m_centralWidget)
    {
        m_centralWidget->slotStopUpdating();
    }

    qDebug("MainForm::actStop()");
    slotMsg("Stoped by user");
    //slotEnableControls(true);
}
void MainForm::initWidgets()
{
    qDebug("MainForm::initWidgets()");
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_centralWidget = new UG_CentralWidget(this);
    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(m_centralWidget, SIGNAL(signalPageChanged(int)), this, SLOT(slotVisibleActionsUpdate(int)));
    connect(m_centralWidget, SIGNAL(signalEnableControls(bool)), this, SLOT(slotEnableControls(bool)));
    connect(m_centralWidget, SIGNAL(signalGetFilterParams(quint16&, double&)), this, SLOT(slotSetFilterParams(quint16&, double&)));

    slotEnableControls(true);
}
void MainForm::initCommonSettings()
{
    qDebug("MainForm::initCommonSettings()");
    QStringList combo_list;

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

    key = QString("view_precision");
    lCommonSettings.addParam(QString("View precision of floating numeric"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    for (int i=0; i<7; i++) combo_list.append(QString::number(i));
    lCommonSettings.setComboList(key, combo_list);


    key = QString("graphserv");
    lCommonSettings.addParam(QString("The graph server domain"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString(""));        

    key = QString("apikey");
    lCommonSettings.addParam(QString("API key"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("subgraph_id");
    lCommonSettings.addParam(QString("The graph subgraph_id"), LSimpleDialog::sdtStringCombo, key);
    lCommonSettings.setComboList(key, sub_commonSettings.factoryTitles());

    key = QString("wallet");
    lCommonSettings.addParam(QString("Wallet address"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("min_tvl");
    lCommonSettings.addParam(QString("Min pools TVL, USDT"), LSimpleDialog::sdtDoubleLine, key);
    lCommonSettings.setDefValue(key, QString("50000.0"));

    key = QString("use_prefer_tokens");
    lCommonSettings.addParam(QString("Use only prefer tokens"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);


    key = QString("req_size");
    lCommonSettings.addParam(QString("Request elements size"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "1" << "2" << "3" << "5" << "10" << "20" << "50" << "100" << "200" << "500" << "700" << "900";
    lCommonSettings.setComboList(key, combo_list);


    key = QString("req_interval");
    lCommonSettings.addParam(QString("Request sending interval, ms"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "500" << "1000" << "1500" << "2000" << "3000" << "5000" << "10000" << "20000";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(3));

}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    m_centralWidget->save(settings);
    //api_config.saveFavorTickers();
}
void MainForm::load()
{
    qDebug("MainForm::load()");

    if (sub_commonSettings.invalid())
    {
        slotMsg(QString("sub_commonSettings is invalid"));
        return;
    }

    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_centralWidget->load(settings);
    m_centralWidget->setExpandLevel(expandLevel());
    m_centralWidget->setUpdatingInterval(pageUpdatingInterval());
    m_centralWidget->setApiServer(graphDomain());
    m_centralWidget->setApiKeys(apiKey(), subgraphID());

    sub_commonSettings.setCurFactory(subgraphID());
    sub_commonSettings.only_prefer_tokens = usePreferTokens();
    sub_commonSettings.wallet = walletAddr();
}
void MainForm::slotEnableControls(bool b)
{    
    getAction(atStart)->setEnabled(b);
    getAction(atStop)->setEnabled(!b);
    getAction(atRefresh)->setEnabled(b);
    getAction(atSettings)->setEnabled(b);
}
void MainForm::slotAppSettingsChanged(QStringList list)
{
    LMainWidget::slotAppSettingsChanged(list);

    if (list.contains("apikey") || list.contains("subgraph_id"))
    {
        m_centralWidget->setApiKeys(apiKey(), subgraphID());
        sub_commonSettings.setCurFactory(subgraphID());
    }

    if (list.contains("graphserv"))
        m_centralWidget->setApiServer(graphDomain());

    if (list.contains("use_prefer_tokens"))
        sub_commonSettings.only_prefer_tokens = usePreferTokens();

    updateWindowTitle();
}
QString MainForm::mainTitle() const
{
    QString s("Unigraph Qt5");
    if (sub_commonSettings.cur_factory >= 0)
        s = QString("%1  (CHAIN: %2)").arg(s).arg(sub_commonSettings.curChain());
    return s;
}




//private
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
QString MainForm::graphDomain() const
{
    return lCommonSettings.paramValue("graphserv").toString();
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





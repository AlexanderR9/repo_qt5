#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lhtmlpagerequester.h"
#include "cfdpage.h"
#include "logpage.h"
#include "chartpage.h"
#include "divpage.h"
#include "htmlpage.h"
#include "configpage.h"
#include "cfdconfigobj.h"
#include "tgbot.h"
#include "cfdcalcobj.h"
#include "lfile.h"

#include <QDebug>
#include <QTimer>
#include <QIcon>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QWebEngineView>
#include <QProgressBar>
#include <QGroupBox>
#include <QStyleFactory>
#include <QTabWidget>
#include <QLibraryInfo>


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_tab(NULL),
    m_configObj(NULL),
    m_bot(NULL),
    m_timer(NULL),
    m_calcObj(NULL)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
    qDeleteAll(m_pages);
    m_pages.clear();

    initPages();
    initTab();
    initSplitter();

    updateActionsEnable(true);

    //////////////////////////////////
    LogStruct log(amtMainWindow, 0);
    log.msg = "Application started";
    emit signalSendLog(log);
}
void MainForm::initPages()
{
    CFDPage *cfd_page = new  CFDPage(this);
    m_pages.insert(BasePage::ptCFDStat, cfd_page);

    ConfigPage *config_page = new  ConfigPage(this);
    m_pages.insert(BasePage::ptConfig, config_page);

    LogPage *log_page = new  LogPage(this);
    m_pages.insert(BasePage::ptLog, log_page);
    connect(this, SIGNAL(signalSendLog(const LogStruct&)), log_page, SLOT(slotNewLog(const LogStruct&)));

    HtmlPage *html_page = new  HtmlPage(this);
    m_pages.insert(BasePage::ptHtml, html_page);
    connect(html_page, SIGNAL(signalGetUrlByTicker(const QString&, QString&)), config_page, SLOT(slotSetUrlByTicker(const QString&, QString&)));

    ChartPage *chart_page = new  ChartPage(this);
    m_pages.insert(BasePage::ptChart, chart_page);
    connect(chart_page, SIGNAL(signalGetSource(QStringList&)), config_page, SLOT(slotSetChartSource(QStringList&)));
    connect(this, SIGNAL(signalPointsSizeChanged(quint8)), chart_page, SLOT(slotPointsSizeChanged(quint8)));

    DivPage *div_page = new  DivPage(this);
    m_pages.insert(BasePage::ptDiv, div_page);
    connect(div_page, SIGNAL(signalGetSource(QStringList&)), config_page, SLOT(slotSetChartSource(QStringList&)));
    connect(div_page, SIGNAL(signalGetDivData(const QString&)), html_page, SLOT(slotGetDivData(const QString&)));
    connect(html_page, SIGNAL(signalDivDataReceived(const QString&)), div_page, SLOT(slotDivDataReceived(const QString&)));
    connect(div_page, SIGNAL(signalGetCurrentPrices(QMap<QString, double>&)), cfd_page, SLOT(slotSetCurrentPrices(QMap<QString, double>&)));

    foreach (BasePage *page, m_pages)
    {
        connect(page, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
        connect(page, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
        connect(page, SIGNAL(signalSendLog(const LogStruct&)), this, SIGNAL(signalSendLog(const LogStruct&)));
    }
}
void MainForm::initTab()
{
    if (m_tab) {delete m_tab; m_tab = NULL;}
    m_tab = new QTabWidget(this);
    m_tab->setIconSize(QSize(28, 28));
    //m_tab->setTabShape(QTabWidget::Triangular);
    m_tab->setTabPosition(QTabWidget::South);

    int n = m_tab->count();
    for (int i=0; i<n; i++)
    {
        QWidget *w = m_tab->widget(0);
        m_tab->removeTab(0);
        if (w) delete w;
    }
    foreach (BasePage *page, m_pages)
    {
        QIcon icon(page->iconPath());
        m_tab->addTab(page, icon, page->caption());
    }
}
void MainForm::initSplitter()
{
    m_protocol = new LProtocolBox(false, this);
    if (v_splitter) {delete v_splitter; v_splitter = NULL;}
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    v_splitter->addWidget(m_tab);
    v_splitter->addWidget(m_protocol);
}
void MainForm::initConfigObj()
{
    if (m_configObj) {delete m_configObj; m_configObj = NULL;}
    m_configObj = new CFDConfigObject(lCommonSettings.paramValue("config").toString(), this);

    connect(m_configObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_configObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    m_protocol->addText("Try load CFD configuration .....", LProtocolBox::ttOk);
    m_configObj->tryLoadConfig();

    //save tickers to file (by wish)
    /*
    QStringList tickers(m_configObj->getTickers(true));
    QString fname("log/ista_tickers.txt");
    QString err = LFile::writeFileSL(fname, tickers);
    if (!err.isEmpty()) slotError(err);
    else m_protocol->addText(QString("saved to file insta_tickers, %1 ps.").arg(tickers.count()));
    */
}
void MainForm::initCalcObj()
{
    if (m_calcObj) {delete m_calcObj; m_calcObj = NULL;}
    m_calcObj = new CFDCalcObj(m_configObj->calcActionParams(), this);

    connect(m_calcObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_calcObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    connect(m_calcObj, SIGNAL(signalSendLog(const LogStruct&)), this, SIGNAL(signalSendLog(const LogStruct&)));

    m_protocol->addText("Calculate object ready!", LProtocolBox::ttOk);
    m_protocol->addSpace();

    HtmlPage *page = qobject_cast<HtmlPage*>(m_pages.value(BasePage::ptHtml));
    if (!page)
    {
        qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to HtmlPage from m_pages");
        return;
    }
    connect(page, SIGNAL(signalNewPrice(QString, double)), m_calcObj, SLOT(slotNewPrice(QString, double)));

    CFDPage *cfd_page = qobject_cast<CFDPage*>(m_pages.value(BasePage::ptCFDStat));
    if (!cfd_page)
    {
        qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to CFDPage from m_pages");
        return;
    }
    connect(m_calcObj, SIGNAL(signalUpdateCFDTable(const QStringList&)), cfd_page, SLOT(slotNewPrice(const QStringList&)));
    connect(cfd_page, SIGNAL(signalGetInstaPtr(const QString&, bool&)), m_configObj, SLOT(slotSetInstaPtr(const QString&, bool&)));
}
void MainForm::initBotObj()
{
    if (m_bot) {delete m_bot; m_bot = NULL;}
    m_bot = new TGBot(m_configObj->calcActionParams(), this);

    connect(m_bot, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_bot, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    connect(m_bot, SIGNAL(signalSendLog(const LogStruct&)), this, SIGNAL(signalSendLog(const LogStruct&)));
    connect(m_bot, SIGNAL(signalGetLastPrice(const QString&, double&, int&)), m_calcObj, SLOT(slotGetLastPrice(const QString&, double&, int&)));
    connect(m_calcObj, SIGNAL(signalInfoToBot(const QString&, const QList<double>&)), m_bot, SLOT(slotNewChangingPrices(const QString&, const QList<double>&)));
    connect(m_bot, SIGNAL(signalGetInstaPtr(const QString&, bool&)), m_configObj, SLOT(slotSetInstaPtr(const QString&, bool&)));

    m_protocol->addSpace();
    m_protocol->addText("Try bot configuration .....", LProtocolBox::ttOk);
    m_bot->loadConfig(lCommonSettings.paramValue("config").toString());
    if (m_bot->invalid())
    {
        slotError(QString("invalid loaded bot parameters"));
        return;
    }
}
void MainForm::fillPages()
{
    //config page
    ConfigPage *config_page = qobject_cast<ConfigPage*>(m_pages.value(BasePage::ptConfig));
    if (config_page)
    {
        config_page->reinitCFDTable();
        config_page->reinitTGTable();
        config_page->setTGBotParams(m_bot->getParams());
        QStringList data(m_configObj->getSources());
        config_page->setSourses(data);

        int n = m_configObj->cfdCount();
        for (int i=0; i<n; i++)
        {
            QStringList row_data(m_configObj->getCFDObjectData(i));
            config_page->addCFDObject(row_data);
        }
        config_page->updatePage();
    }
    else qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to ConfigPage from m_pages");

    //chart page
    ChartPage *chart_page = qobject_cast<ChartPage*>(m_pages.value(BasePage::ptChart));
    if (chart_page)
    {
        chart_page->initSource();
        connect(chart_page, SIGNAL(signalGetChartData(const QString&, QMap<QDateTime, float>&)), m_calcObj, SLOT(slotSetChartData(const QString&, QMap<QDateTime, float>&)));
    }
    else qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to ChartPage from m_pages");

    //divs page
    DivPage *div_page = qobject_cast<DivPage*>(m_pages.value(BasePage::ptDiv));
    if (div_page)
    {
        div_page->initSource();

        const GetDivsParams &div_params = m_configObj->divParams();
        div_page->setReqParams(div_params.source_url, div_params.request_interval*3600, div_params.look_div_days);
        div_page->setShownHistory(div_params.show_last);
        div_page->setTickTimerInterval(div_params.timer_interval);
        div_page->setLightValues(div_params.light_div_size, div_params.light_price);

        connect(div_page, SIGNAL(signalGetInstaPtr(const QString&, bool&)), m_configObj, SLOT(slotSetInstaPtr(const QString&, bool&)));
        connect(div_page, SIGNAL(signalGetLastPrice(const QString&, double&, int&)), m_calcObj, SLOT(slotGetLastPrice(const QString&, double&, int&)));

        div_page->tickTimerStart();
    }
    else qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to DivPage from m_pages");
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("config");
    lCommonSettings.addParam(QString("Config file"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("req_interval");
    lCommonSettings.addParam(QString("Request interval, sec"), LSimpleDialog::sdtIntCombo, key);
    for (int i=1; i<=20; i++) combo_list.append(QString::number(i*3));
    lCommonSettings.setComboList(key, combo_list);

    key = QString("log_max_size");
    lCommonSettings.addParam(QString("Log max size"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "200" << "500" << "1000" << "2000" << "3000" << "5000";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("chart_points_size");
    lCommonSettings.addParam(QString("Chart points size"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "0" << "1" << "2" << "3" << "4" << "5";
    lCommonSettings.setComboList(key, combo_list);


}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        case LMainWidget::atStop: {stop(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::slotTimer()
{
    QString next_ticker;
    m_configObj->getNextTicker(next_ticker);
    m_protocol->addSpace();
    m_protocol->addText(QString("next price request for [%1] .........").arg(next_ticker), LProtocolBox::ttFile);

    HtmlPage *page = qobject_cast<HtmlPage*>(m_pages.value(BasePage::ptHtml));
    if (page) page->tryRequest(next_ticker);
    else qWarning()<<QString("MainForm::slotTimer() ERROR: invalid convert to HtmlPage from m_pages");
}
void MainForm::updateActionsEnable(bool stoped)
{
    getAction(LMainWidget::atStop)->setEnabled(!stoped);
    getAction(LMainWidget::atStart)->setEnabled(stoped);
    getAction(LMainWidget::atSettings)->setEnabled(stoped);
    getAction(LMainWidget::atExit)->setEnabled(stoped);
}
void MainForm::start()
{
    m_protocol->addSpace();
    m_protocol->addText(QString("Monitoring started, request interval: %1 sec.").arg(reqInterval()/1000), 5);
    updateActionsEnable(false);

    m_timer->setInterval(reqInterval());
    m_timer->start();
    m_bot->startCheckingUpdatesTimer();
}
void MainForm::stop()
{
    m_timer->stop();
    m_bot->stopCheckingUpdatesTimer();
    m_protocol->addText("Monitoring stoped!", 5);
    updateActionsEnable(true);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/tab/page_index").arg(objectName()), m_tab->currentIndex());
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_tab->setCurrentIndex(settings.value(QString("%1/tab/page_index").arg(objectName()), 0).toInt());

    initConfigObj();
    initCalcObj();
    initBotObj();
    fillPages();

    QStringList keys;
    keys.append(QString("log_max_size"));
    keys.append(QString("chart_points_size"));
    slotAppSettingsChanged(keys);
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    LMainWidget::slotAppSettingsChanged(keys);

    QString key = QString("log_max_size");
    if (keys.contains(key))
    {
        int n = lCommonSettings.paramValue(key).toInt();
        LogPage *page = qobject_cast<LogPage*>(m_pages.value(BasePage::ptLog));
        if (!page)
        {
            qWarning()<<QString("MainForm::slotAppSettingsChanged() ERR: invalid convert to LogPage from m_pages");
            return;
        }
        page->setMaxSize(n);
        page->updatePage();
    }

    key = QString("chart_points_size");
    if (keys.contains(key)) emit signalPointsSizeChanged(chartPointsSize());
}
int MainForm::reqInterval() const
{
    return (lCommonSettings.paramValue("req_interval").toInt() * 1000);
}
quint8 MainForm::chartPointsSize() const
{
    return (lCommonSettings.paramValue("chart_points_size").toUInt());
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}


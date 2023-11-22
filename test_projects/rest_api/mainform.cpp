#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "instrument.h"
#include "lstatic.h"
#include "lsimplewidget.h"
#include "lhttpapirequester.h"
#include "lfile.h"
#include "lhttp_types.h"
#include "apipages.h"
#include "apireqpage.h"
#include "apicommonsettings.h"
#include "apicouponpage.h"
#include "apieventspage.h"
#include "apiorderspage.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QWebEngineView>
#include <QTimer>
#include <QProgressBar>
#include <QGroupBox>
#include <QStyleFactory>
#include <QListWidget>
#include <QTreeWidget>
#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_tab(NULL)
{
    setObjectName("main_form_restapi_tester");
}
void MainForm::initPages()
{
    APIReqPage *req_page = new  APIReqPage(this);
    m_pages.insert(aptReq, req_page);
    connect(req_page, SIGNAL(signalFinished(int)), this, SLOT(slotReqFinished(int)));
    connect(req_page, SIGNAL(signalGetReqParams(QString&, QString&)), this, SLOT(slotGetReqParams(QString&, QString&)));
    connect(req_page, SIGNAL(signalGetPricesDepth(quint16&)), this, SLOT(slotGetPricesDepth(quint16&)));
    connect(req_page, SIGNAL(signalGetCandleSize(QString&)), this, SLOT(slotGetCandleSize(QString&)));

    APIBondsPage *bond_page = new  APIBondsPage(this);
    m_pages.insert(aptBond, bond_page);
    connect(req_page, SIGNAL(signalGetSelectedBondUID(QString&)), bond_page, SLOT(slotSetSelectedUID(QString&)));
    connect(req_page, SIGNAL(signalGetSelectedBondUIDList(QStringList&)), bond_page, SLOT(slotSetSelectedUIDList(QStringList&)));
    connect(req_page, SIGNAL(signalGetBondCycleData(QStringList&)), bond_page, SLOT(slotSetCycleData(QStringList&)));
    connect(req_page, SIGNAL(signalCyclePrice(const QString&, float)), bond_page, SLOT(slotCyclePrice(const QString&, float)));

    APIStocksPage *stock_page = new  APIStocksPage(this);
    m_pages.insert(aptStock, stock_page);
    connect(req_page, SIGNAL(signalGetSelectedStockUID(QString&)), stock_page, SLOT(slotSetSelectedUID(QString&)));
    connect(req_page, SIGNAL(signalGetSelectedStockUIDList(QStringList&)), stock_page, SLOT(slotSetSelectedUIDList(QStringList&)));
    connect(req_page, SIGNAL(signalGetStockCycleData(QStringList&)), stock_page, SLOT(slotSetCycleData(QStringList&)));
    connect(req_page, SIGNAL(signalCyclePrice(const QString&, float)), stock_page, SLOT(slotCyclePrice(const QString&, float)));

    APICouponPage *c_page = new  APICouponPage(this);
    m_pages.insert(aptCoupon, c_page);
    connect(c_page, SIGNAL(signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)), bond_page, SLOT(slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)));
    connect(bond_page, SIGNAL(signalFilter(const QStringList&)), c_page, SLOT(slotFilter(const QStringList&)));
    connect(c_page, SIGNAL(signalGetTickerByFigi(const QString&, QString&)), bond_page, SLOT(slotGetTickerByFigi(const QString&, QString&)));

    APIDivPage *d_page = new  APIDivPage(this);
    m_pages.insert(aptDiv, d_page);
    connect(d_page, SIGNAL(signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)), stock_page, SLOT(slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)));
    connect(d_page, SIGNAL(signalGetTickerByFigi(const QString&, QString&)), stock_page, SLOT(slotGetTickerByFigi(const QString&, QString&)));
    connect(stock_page, SIGNAL(signalFilter(const QStringList&)), d_page, SLOT(slotFilter(const QStringList&)));

    APIEventsPage *events_page = new  APIEventsPage(this);
    m_pages.insert(aptEvent, events_page);
    connect(req_page, SIGNAL(signalLoadEvents(const QJsonObject&)), events_page, SLOT(slotLoadEvents(const QJsonObject&)));
    connect(events_page, SIGNAL(signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)), stock_page, SLOT(slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)));
    connect(events_page, SIGNAL(signalGetTickerByFigi(const QString&, QString&)), stock_page, SLOT(slotGetTickerByFigi(const QString&, QString&)));
    connect(events_page, SIGNAL(signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)), bond_page, SLOT(slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)));
    connect(events_page, SIGNAL(signalGetTickerByFigi(const QString&, QString&)), bond_page, SLOT(slotGetTickerByFigi(const QString&, QString&)));

    APIOrdersPage *orders_page = new  APIOrdersPage(this);
    m_pages.insert(aptOrders, orders_page);
    connect(req_page, SIGNAL(signalLoadOrders(const QJsonObject&)), orders_page, SLOT(slotLoadOrders(const QJsonObject&)));
    connect(orders_page, SIGNAL(signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)), stock_page, SLOT(slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)));
    connect(orders_page, SIGNAL(signalGetTickerByFigi(const QString&, QString&)), stock_page, SLOT(slotGetTickerByFigi(const QString&, QString&)));
    connect(orders_page, SIGNAL(signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)), bond_page, SLOT(slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&)));
    connect(orders_page, SIGNAL(signalGetTickerByFigi(const QString&, QString&)), bond_page, SLOT(slotGetTickerByFigi(const QString&, QString&)));
    connect(orders_page, SIGNAL(signalGetPaperTypeByUID(const QString&, QString&)), bond_page, SLOT(slotGetPaperTypeByUID(const QString&, QString&)));
    connect(orders_page, SIGNAL(signalGetPaperTypeByUID(const QString&, QString&)), stock_page, SLOT(slotGetPaperTypeByUID(const QString&, QString&)));

    APIProfitabilityPage *profit_page = new  APIProfitabilityPage(this);
    m_pages.insert(aptProfitability, profit_page);
    connect(bond_page, SIGNAL(signalNeedCalcProfitability(const BondDesc&, float)), profit_page, SLOT(slotRecalcProfitability(const BondDesc&, float)));
    connect(profit_page, SIGNAL(signalGetCouponRec(const QString&, const BCoupon*&)), c_page, SLOT(slotGetCouponRec(const QString&, const BCoupon*&)));





    APIBagPage *bag_page = new  APIBagPage(this);
    m_pages.insert(aptBag, bag_page);
    connect(req_page, SIGNAL(signalLoadPortfolio(const QJsonObject&)), bag_page, SIGNAL(signalLoadPortfolio(const QJsonObject&)));
    connect(req_page, SIGNAL(signalLoadPositions(const QJsonObject&)), bag_page, SIGNAL(signalLoadPositions(const QJsonObject&)));
    connect(bag_page, SIGNAL(signalGetPaperInfo(QStringList&)), bond_page, SLOT(slotGetPaperInfo(QStringList&)));
    connect(bag_page, SIGNAL(signalGetPaperInfo(QStringList&)), stock_page, SLOT(slotGetPaperInfo(QStringList&)));

    m_tab->clear();
    foreach (LSimpleWidget *page, m_pages)
    {
        QIcon icon(page->iconPath());
        m_tab->addTab(page, icon, page->caption());
        connect(page, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
        connect(page, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    }
}
void MainForm::loadData()
{
    m_protocol->addSpace();
    m_protocol->addText("Loading data .....", LProtocolBox::ttData);
    LSimpleWidget *page = activePage();
    if (!page) {slotError("PAGE IS NULL!!!!"); return;}


    switch (page->userSign())
    {
        case aptBond:
        {
            APIBondsPage *bond_page = qobject_cast<APIBondsPage*>(m_pages.value(aptBond));
            if (bond_page) bond_page->loadData();
            else slotError("bond_page is NULL");
            break;
        }
        case aptStock:
        {
            APIStocksPage *stock_page = qobject_cast<APIStocksPage*>(m_pages.value(aptStock));
            if (stock_page) stock_page->loadData();
            else slotError("stock_page is NULL");
            break;
        }
        case aptCoupon:
        case aptDiv:
        {
            APICouponPageAbstract *coupon_page = qobject_cast<APICouponPageAbstract*>(m_pages.value(page->userSign()));
            if (coupon_page) coupon_page->loadData();
            else slotError(QString("coupon page (sign=%1) is NULL").arg(page->userSign()));
            break;
        }
        default:
        {
            m_protocol->addText("active page belong not for loading", LProtocolBox::ttWarning);
            break;
        }
    }
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=10; i++)
        combo_list.append(QString::number(i));
    combo_list.append(QString::number(-1));

    QString key = QString("server_api");
    lCommonSettings.addParam(QString("Server API"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("invest-public-api.tinkoff.ru"));

    key = QString("base_api_uri");
    lCommonSettings.addParam(QString("Base API URI"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("rest/tinkoff.public.invest.api.contract.v1"));

    key = QString("token");
    lCommonSettings.addParam(QString("API token"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("expand_level");
    lCommonSettings.addParam(QString("Expand JSON view level"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);

    key = QString("depth");
    lCommonSettings.addParam(QString("Depth prices"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(2));

    key = QString("candle_size");
    lCommonSettings.addParam(QString("Candle size (one interval)"), LSimpleDialog::sdtStringCombo, key);

    key = QString("print_headers");
    lCommonSettings.addParam(QString("Out headers to protocol"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);

    key = QString("auto_start");
    lCommonSettings.addParam(QString("Run AUTO_START requests"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {sendAPIRequest(); break;}
        case LMainWidget::atLoadData: {loadData(); break;}
        case LMainWidget::atClear: {clear(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    m_protocol = new LProtocolBox(false, this);
    m_tab = new QTabWidget(this);
    m_tab->setIconSize(QSize(28, 28));
    v_splitter->addWidget(m_tab);
    v_splitter->addWidget(m_protocol);
    addWidget(v_splitter, 0, 0);
}
void MainForm::sendAPIRequest()
{
    m_protocol->addText(QString("Try send request..."), LProtocolBox::ttData);
    APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
    if (req_page)
    {
        enableActions(false);
        req_page->trySendReq();
    }
    else slotError("req_page is NULL");
}
void MainForm::enableActions(bool b)
{
    getAction(LMainWidget::atStart)->setEnabled(b);
    getAction(LMainWidget::atSettings)->setEnabled(b);
    getAction(LMainWidget::atClear)->setEnabled(b);
}
void MainForm::clear()
{
    m_protocol->clearProtocol();
    LSimpleWidget *page = activePage();
    if (page) page->resetPage();
}
LSimpleWidget* MainForm::activePage() const
{
    if (m_tab->count() == 0) return NULL;
    return (qobject_cast<LSimpleWidget*>(m_tab->currentWidget()));
}
void MainForm::save()
{
    LMainWidget::save();        

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    foreach (LSimpleWidget *page, m_pages)
        page->save(settings);

    settings.setValue(QString("%1/tab_index").arg(objectName()), m_tab->currentIndex());
}
void MainForm::load()
{
    QString err;
    api_commonSettings.loadConfig(err);
    if (!err.isEmpty())
    {
        slotError(err);
        enableActions(false);
        return;
    }
    else
    {
        slotMsg(QString("loaded candle sizes: %1").arg(api_commonSettings.candle_sizes.keys().count()));
        slotMsg(QString("loaded api functions: %1").arg(api_commonSettings.services.count()));
        lCommonSettings.setComboList(QString("candle_size"), api_commonSettings.candle_sizes.keys());
        initPages();
    }


    LMainWidget::load();
    api_commonSettings.parseToken(token());

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    foreach (LSimpleWidget *page, m_pages)
        page->load(settings);

    APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
    if (req_page)
    {
        req_page->setServerAPI(hptHttps, serverAPI());
        req_page->setExpandLevel(expandLevel());
        req_page->setPrintHeaders(printHeaders());
    }
    else slotError("req_page is NULL");

    int tab_index = settings.value(QString("%1/tab_index").arg(objectName()), 0).toInt();
    if (m_tab) m_tab->setCurrentIndex(tab_index);

    runAutoStart();
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::slotReqFinished(int result)
{
    if (result == hreOk || result == hreServerErr)
    {
        APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
        req_page->checkReply();
        if (req_page->replyOk())
            m_protocol->addText("REPLY CODE OK:", LProtocolBox::ttOk);
    }

    m_protocol->addText(" ---------------- Request finished -------------------", LProtocolBox::ttData);
    m_protocol->addSpace();
    if (!autoStartModeNow()) enableActions(true);
}
void MainForm::slotAppSettingsChanged(QStringList list)
{
    LMainWidget::slotAppSettingsChanged(list);
    if (list.contains("server_api") || list.contains("print_headers"))
    {
        APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
        if (req_page)
        {
            req_page->setServerAPI(hptHttps, serverAPI());
            req_page->setPrintHeaders(printHeaders());
        }
        else slotError("req_page is NULL");
    }
    else if (list.contains("expand_level"))
    {
        APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
        if (req_page) req_page->setExpandLevel(expandLevel());
        else slotError("req_page is NULL");
    }
}
void MainForm::slotAutoStart()
{
    QTimer *ast = qobject_cast<QTimer*>(sender());
    if (!ast) {slotError("Auto start timer is NULL"); return;}

    APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
    if (req_page && req_page->requesterBuzy())
    {
        slotError("Requester object is buzy.");
        return;
    }

    if (api_commonSettings.start_reqs.invalid())
    {
        ast->stop();
        enableActions(true);
        getAction(LMainWidget::atLoadData)->setEnabled(true);
        slotMsg("Auto start finished!");
        return;
    }

    QString next_src(api_commonSettings.start_reqs.src.first());
    slotMsg(QString("next src: %1").arg(next_src));
    api_commonSettings.start_reqs.src.removeFirst();
    req_page->autoStartReq(next_src);
}
void MainForm::runAutoStart()
{
    if (!autoStart()) return;
    if (api_commonSettings.start_reqs.invalid())
    {
        slotError("Auto start node is invalid or is absent");
        return;
    }

    QString s = QString("RUN AUTO_START MODE: timeout=%1, req count %2").arg(api_commonSettings.start_reqs.timeout).arg(api_commonSettings.start_reqs.src.count());
    m_protocol->addText(s, LProtocolBox::ttFile);
    enableActions(false);
    getAction(LMainWidget::atLoadData)->setEnabled(false);

    autoLoadDataFiles();

    QTimer *ast = new QTimer(this);
    connect(ast, SIGNAL(timeout()), this, SLOT(slotAutoStart()));
    ast->start(api_commonSettings.start_reqs.timeout);
}
bool MainForm::autoStartModeNow() const
{
    return !getAction(LMainWidget::atLoadData)->isEnabled();
}
void MainForm::autoLoadDataFiles()
{
    m_protocol->addSpace();
    m_protocol->addText("Loading data .....", LProtocolBox::ttData);
    APIBondsPage *bond_page = qobject_cast<APIBondsPage*>(m_pages.value(aptBond));
    if (bond_page) bond_page->loadData();
    else slotError("bond_page is NULL");
    APIStocksPage *stock_page = qobject_cast<APIStocksPage*>(m_pages.value(aptStock));
    if (stock_page) stock_page->loadData();
    else slotError("stock_page is NULL");
    APICouponPage *coupon_page = qobject_cast<APICouponPage*>(m_pages.value(aptCoupon));
    if (coupon_page) coupon_page->loadData();
    else slotError("coupon_page is NULL");
    APIDivPage *div_page = qobject_cast<APIDivPage*>(m_pages.value(aptDiv));
    if (div_page) div_page->loadData();
    else slotError("div_page is NULL");
}



//private funcs
QString MainForm::serverAPI() const
{
    return lCommonSettings.paramValue("server_api").toString();
}
QString MainForm::baseURI() const
{
    return lCommonSettings.paramValue("base_api_uri").toString();
}
QString MainForm::token() const
{
    return lCommonSettings.paramValue("token").toString();
}
QString MainForm::candleSize() const
{
    return lCommonSettings.paramValue("candle_size").toString();
}
bool MainForm::printHeaders() const
{
    return lCommonSettings.paramValue("print_headers").toBool();
}
bool MainForm::autoStart() const
{
    return lCommonSettings.paramValue("auto_start").toBool();
}
int MainForm::expandLevel() const
{
    return lCommonSettings.paramValue("expand_level").toInt();
}
int MainForm::depth() const
{
    return lCommonSettings.paramValue("depth").toInt();
}



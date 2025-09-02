#include "basetab_v3.h"
#include "appcommonsettings.h"
#include "deficonfig.h"
#include "basetabpage_v3.h"
#include "nodejsbridge.h"
#include "wallettabpage.h"
#include "txpage.h"

#include <QDebug>
#include <QTimer>
#include <QSplitter>
#include <QTabWidget>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>


#define STATE_TIMER_INTERVAL        700 //ms
#define UPDATING_TIMEOUT            15 //sec, таймаут выполнения одного сценария запросов



//BaseTabV3
DefiChainTabV3::DefiChainTabV3(QWidget *parent, int chain_id)
    :LSimpleWidget(parent, 20),
    m_tab(NULL),
    m_timer(NULL),
    m_timerCounter(0),
    js_bridge(NULL)
{
    m_userSign = chain_id;
    setObjectName(QString("v3_chain_tab_%1").arg(chain_id));

    //this->setContentsMargins(0, 0, 0, 0);
    initTab();
    initReqStateWidget();
    initJsBridgeObj();
    this->layout()->setMargin(0);

    m_timer = new QTimer(this);
    m_timer->setInterval(STATE_TIMER_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    m_timer->stop();

}
void DefiChainTabV3::initJsBridgeObj()
{
    js_bridge = new NodejsBridge(this, chainId());
    connect(js_bridge, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(js_bridge, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(js_bridge, SIGNAL(signalFinished(int)), this, SLOT(slotJSScriptFinished(int)));

}
void DefiChainTabV3::slotJSScriptFinished(int code)
{
    qDebug("DefiChainTabV3::slotJSScriptFinished()");
    qDebug()<<QString("result code %1").arg(code);
    stopUpdating();

    QPalette p = m_reqStateEdit->palette();
    QString result = "OK";
    if (code != 0)
    {
        emit signalError("DefiChainTabV3: js_bridge finished code fault");
        result = "FAULT";
        p.setColor(QPalette::Text, Qt::red);
    }
    else
    {
        p.setColor(QPalette::Text, Qt::blue);
    }
    m_reqStateEdit->setPalette(p);
    m_reqStateEdit->setText(QString("script finished, result %1").arg(result));
}
void DefiChainTabV3::slotTabsPricesUpdate()
{
    qDebug("DefiChainTabV3::slotTabsPricesUpdate()");
    const DefiWalletTabPage *wpage =walletPage();
    if (wpage) wpage->updatePrices();

}
const DefiWalletTabPage* DefiChainTabV3::walletPage() const
{
    int n = m_tab->pageCount();
    for (int i=0; i<n; i++)
    {
        const BaseTabPage_V3 *page = qobject_cast<const BaseTabPage_V3*>(tabWidget()->widget(i));
        if (page)
        {
            if (page->kind() == dpkWallet)
                return (qobject_cast<const DefiWalletTabPage*>(page));
        }
    }
    return NULL;
}
const DefiTxTabPage* DefiChainTabV3::txPage() const
{
    int n = m_tab->pageCount();
    for (int i=0; i<n; i++)
    {
        const BaseTabPage_V3 *page = qobject_cast<const BaseTabPage_V3*>(tabWidget()->widget(i));
        if (page)
        {
            if (page->kind() == dpkTx)
                return (qobject_cast<const DefiTxTabPage*>(page));
        }
    }
    return NULL;
}



void DefiChainTabV3::tabActivated()
{
    qDebug("DefiChainTabV3::tabActivated()");
    QString chain = "???";
    int pos = defi_config.chainIndexOf(chainId());
    if (pos >= 0) chain = defi_config.chains.at(pos).name;

    //prepare json for define current chain
    QJsonObject j_params;
    j_params.insert("value", chain);
    emit signalRewriteJsonFile(j_params, AppCommonSettings::curChainNodeJSFile()); //rewrite json-file


    //prepare json for define wallet assets list
    QJsonObject j_assets;
    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id != chainId()) continue;

        QJsonObject j_token;
        j_token.insert("decimal", int(v.decimal));
        j_token.insert("address", v.address);
        j_assets.insert(v.name, j_token);
    }
    emit signalRewriteJsonFile(j_assets, AppCommonSettings::walletAssetsNodeJSFile()); //rewrite json-file

}
void DefiChainTabV3::initTab()
{
    m_tab = new LTabWidgetBox(this);
    m_tab->removeAllPages();
    m_tab->setTitle("Chain page");
    //m_tab->setBorderNone();
    tabWidget()->setObjectName(QString("tab_widget_%1").arg(chainId()));

    h_splitter->addWidget(m_tab);
    //m_tab->layout()->setMargin(4);
    m_tab->layout()->setSpacing(0);
}
void DefiChainTabV3::initReqStateWidget()
{
    QWidget *req_w = new QWidget(this);
    m_tab->layout()->addWidget(req_w);

    if (req_w->layout()) delete req_w->layout();
    req_w->setLayout(new QHBoxLayout(0));
    m_tab->layout()->setMargin(0);

    req_w->layout()->addWidget(new QLabel("REQUEST STATE:"));

    m_reqStateEdit = new QLineEdit(this);
    m_reqStateEdit->setReadOnly(true);
    req_w->layout()->addWidget(m_reqStateEdit);
    //req_w->layout()->setContentsMargins(10, 0, 10, 4);
    req_w->layout()->setMargin(2);
}
QTabWidget* DefiChainTabV3::tabWidget() const
{
    if (m_tab) return m_tab->tab();
    return NULL;
}
void DefiChainTabV3::slotTimer()
{
    m_timerCounter++;
    qDebug()<<QString("PAGE[%1] slotTimer, counter %2").arg(userSign()).arg(m_timerCounter);


    int t = m_timerCounter*STATE_TIMER_INTERVAL;
    if (t > (UPDATING_TIMEOUT*1000))
    {
        emit signalError("request processing timeout");
        stopUpdating();
    }
}
void DefiChainTabV3::startUpdating()
{
    qDebug()<<QString("DefiChainTabV3::startUpdating page=%1").arg(tabWidget()->currentIndex());
    BaseTabPage_V3 *page = qobject_cast<BaseTabPage_V3*>(tabWidget()->currentWidget());
    if (!page) {emit signalError("current tab page is null"); return;}

    /*
    if (js_bridge->buzy())  {emit signalError("nodejs bridge is buzy"); return;}
    emit signalEnableControls(false);
    m_timerCounter = 0;
    m_timer->start();
    */

    page->sendUpdateDataRequest();
}
void DefiChainTabV3::stopUpdating()
{
    qDebug()<<QString("UG_BasePage::stopUpdating  page=%1").arg(userSign());
    m_timer->stop();
    emit signalEnableControls(true);
}
void DefiChainTabV3::connectPageSignals()
{
    const DefiTxTabPage *tx_page =  txPage();
    if (!tx_page) {emit signalError("DefiChainTabV3: tx_page is NULL");  return;}

    for (int i=0; i<tabWidget()->count(); i++)
    {
        BaseTabPage_V3 *w = qobject_cast<BaseTabPage_V3*>(tabWidget()->widget(i));
        if (w)
        {
            w->setChain(chainId());
            connect(w, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
            connect(w, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

            connect(w, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)), this, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)));
            connect(w, SIGNAL(signalRunNodejsBridge(QString, const QStringList&)), this, SLOT(slotPageSendReq(QString, const QStringList&)));
            connect(js_bridge, SIGNAL(signalNodejsReply(const QJsonObject&)), w, SLOT(slotNodejsReply(const QJsonObject&)));

            if (w->kind() != dpkTx)
            {
                connect(w, SIGNAL(signalNewTx(const TxLogRecord&)), tx_page, SLOT(slotNewTx(const TxLogRecord&)));
            }
        }
    }
}
void DefiChainTabV3::slotPageSendReq(QString req_name, const QStringList &js_args)
{
    QPalette p = m_reqStateEdit->palette();
    if (js_bridge->buzy())
    {
        emit signalError("nodejs bridge is buzy");
        p.setColor(QPalette::Text, Qt::red);
        m_reqStateEdit->setPalette(p);
        m_reqStateEdit->setText("nodejs bridge is buzy");
        return;
    }

    p.setColor(QPalette::Text, "#A0A080");
    m_reqStateEdit->setPalette(p);
    m_reqStateEdit->setText(QString("start, req_name=%1 ..........").arg(req_name));

    emit signalEnableControls(false);
    m_timerCounter = 0;
    m_timer->start();

    emit signalMsg("\n\n\n[STARTED PAGE JS_REQUEST]");
    js_bridge->slotRunScriptArgs(js_args);
}

void DefiChainTabV3::load(QSettings &settings)
{
 //   qDebug("DefiChainTabV3::load");
    LSimpleWidget::load(settings);

    for (int i=0; i<tabWidget()->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(tabWidget()->widget(i));
        if (w) w->load(settings);
        else qWarning("WARNING: tabWidget()->widget(i) is NULL");
    }

}
void DefiChainTabV3::save(QSettings &settings)
{
    LSimpleWidget::save(settings);

    for (int i=0; i<m_tab->pageCount(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(tabWidget()->widget(i));
        if (w) w->save(settings);
        else qWarning("WARNING: tabWidget()->widget(i) is NULL");
    }
}





#include "basetab_v3.h"
#include "appcommonsettings.h"
#include "deficonfig.h"
#include "basetabpage_v3.h"
#include "nodejsbridge.h"
#include "wallettabpage.h"

#include <QDebug>
#include <QTimer>
#include <QSplitter>
#include <QTabWidget>
#include <QJsonObject>


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
    initJsBridgeObj();

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
    //connect(js_bridge, SIGNAL(signalNodejsReply(const QJsonObject&)), this, SLOT(slotNodejsReply(const QJsonObject&)));

}
/*
void DefiChainTabV3::slotNodejsReply(const QJsonObject &js_reply)
{

}
*/

void DefiChainTabV3::slotJSScriptFinished(int code)
{
    qDebug("DefiChainTabV3::slotJSScriptFinished()");
    qDebug()<<QString("result code %1").arg(code);
    stopUpdating();

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
    m_tab->setBorderNone();
    tabWidget()->setObjectName(QString("tab_widget_%1").arg(chainId()));

    h_splitter->addWidget(m_tab);
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


    emit signalEnableControls(false);
    m_timerCounter = 0;
    m_timer->start();

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
    for (int i=0; i<tabWidget()->count(); i++)
    {
        BaseTabPage_V3 *w = qobject_cast<BaseTabPage_V3*>(tabWidget()->widget(i));
        if (w)
        {
            connect(w, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)), this, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)));
            connect(w, SIGNAL(signalRunNodejsBridge(const QStringList&)), js_bridge, SLOT(slotRunScriptArgs(const QStringList&)));
            connect(js_bridge, SIGNAL(signalNodejsReply(const QJsonObject&)), w, SLOT(slotNodejsReply(const QJsonObject&)));
        }
    }
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


/*
bool UG_BasePage::updatingRunning() const
{
    return m_timer->isActive();
}
void UG_BasePage::reset()
{
    if (m_reqData) {delete m_reqData; m_reqData = NULL;}
}
void UG_BasePage::sendRequest(QString name_extra)
{
    m_reqData->name = UG_APIReqParams::strReqTypeByType(m_reqData->req_type, name_extra);
    //m_reqData->is_running = true;
    emit signalMsg(QString("QUERY: %1").arg(m_reqData->query));
    emit signalSendReq(m_reqData);
}
bool UG_BasePage::updateTimeOver(bool forcibly)
{
    QTime ct = QTime::currentTime();
    if (minUpdatingInterval() < 3) return false;
    if (!m_updateTime.isValid() || forcibly) {m_updateTime = ct; return true;}
    if (m_updateTime.secsTo(ct) > minUpdatingInterval()) {m_updateTime = ct; return true;}
    return false;
}

*/




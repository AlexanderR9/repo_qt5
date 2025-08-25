#include "centralwidget_v3.h"
#include "deficonfig.h"
#include "basetab_v3.h"
#include "wallettabpage.h"
#include "appcommonsettings.h"
#include "lfile.h"
#include "lhttp_types.h"
#include "lhttpapirequester.h"
#include "lstring.h"
#include "lmath.h"


#include <QStackedWidget>
#include <QSplitter>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>


//CentralWidgetV3
CentralWidgetV3::CentralWidgetV3(QWidget *parent)
    :LSimpleWidget(parent, 22),
    w_list(NULL),
    w_stack(NULL),
    http_requester(NULL)
{
    setObjectName("central_widget_v3");

    init();

}
void CentralWidgetV3::initHttpRequester()
{
    http_requester = new LHttpApiRequester(this);
    http_requester->setHttpProtocolType(hptHttps);
    http_requester->setApiServer(defi_config.bb_settings.api_server);
    http_requester->setUri(AppCommonSettings::bbGetPricesUri());

    connect(http_requester, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(http_requester, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(http_requester, SIGNAL(signalFinished(int)), this, SLOT(slotHttpReqFinished(int)));

}
void CentralWidgetV3::slotSendHttpReq()
{
    emit signalMsg(QString("\n Start HTTP request ..."));
    if (http_requester->isBuzy()) {emit signalError("Requester object is buzy."); return;}

    qDebug()<<QString("BB_CentralWidget::slotSendReq ");
    const BybitSettings &bb = defi_config.bb_settings;
    if (bb.invalid()) {emit signalError("Bybit API settings is invalid, check app config"); return;}
    QString params;
    int pos = LString::strIndexOfByEnd(AppCommonSettings::bbGetPricesUri(), "?");
    if (pos > 0) params = LString::strTrimLeft(AppCommonSettings::bbGetPricesUri(), pos);


    //QString symbol_params = "symbol=LDOUSDTPOLUSDT";
    //QString symbol_params = "symbol=POLUSDT&val2=ETHUSDT&val3=LDOUSDT";
    //params = QString("%1&%2").arg(params).arg(symbol_params);
    //http_requester->setUri(QString("%1&%2").arg(AppCommonSettings::bbGetPricesUri()).arg(symbol_params));


    // prepare/update headers
    qint64 ts = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    QString hmac_msg = QString("%1%2%3%4").arg(ts).arg(bb.public_key).arg(bb.timeout).arg(params);
    QByteArray ba_sign(LMath::calcHMACSha256(bb.private_key.toLatin1(), hmac_msg.toLatin1()));
    http_requester->addReqHeader(QString("X-BAPI-API-KEY"), bb.public_key);
    http_requester->addReqHeader(QString("X-BAPI-TIMESTAMP"), QString::number(ts));
    http_requester->addReqHeader(QString("X-BAPI-SIGN"), QString(ba_sign.toHex()));
    http_requester->addReqHeader(QString("X-BAPI-RECV-WINDOW"), QString::number(bb.timeout));

    //send req
    emit signalMsg(QString("URL: %1").arg(http_requester->fullUrl()));
    emit signalEnableControls(false);
    http_requester->start(hrmGet);
}
void CentralWidgetV3::slotHttpReqFinished(int code)
{
    qDebug("DefiChainTabV3::slotHttpReqFinished()");
    const LHttpApiReplyData& r = http_requester->lastReply();
    emit signalMsg(QString("next request finished, result %1/%2").arg(r.result_code).arg(code));
    emit signalEnableControls(true);

    if (r.data.isEmpty()) emit signalMsg("WARNING: REPLY JSON IS EMPTY");

    if (r.isOk())
    {
        qDebug()<<QString("result OK,  fields %1").arg(r.data.keys().count());
        const QJsonValue &jv = r.data.value("result");
        if (jv.isNull()) {emit signalError("CentralWidgetV3: result QJsonValue not found"); return;}
        const QJsonValue &j_list = jv.toObject().value("list");
        if (!j_list.isArray()) {emit signalError("CentralWidgetV3: j_list not array"); return;}
        const QJsonArray j_arr = j_list.toArray();
        qDebug()<<QString("got price records: %1").arg(j_arr.count());
        if (j_arr.isEmpty()) {emit signalError("CentralWidgetV3: array of records is empty"); return;}

        newPricesReceived(j_arr);
    }
    else emit signalError("request fault");
}
void CentralWidgetV3::startUpdating()
{
    DefiChainTabV3 *tab = currentTab();
    if (tab) tab->startUpdating();

   // slotSendHttpReq();

}
void CentralWidgetV3::setEnableControl(bool b)
{
    w_list->listWidget()->setEnabled(b);

    DefiChainTabV3 *tab = currentTab();
    if (tab) tab->tabWidget()->setEnabled(b);
}
void CentralWidgetV3::init()
{
    w_list = new LListWidgetBox(this);
    w_list->setTitle("Chains");
    w_list->setObjectName("chain_list_box");
    w_list->setFontSizeItems(16);
    w_list->setBaseColor("#FFEEEE", "#006400");

    w_stack = new QStackedWidget(this);
    w_stack->setObjectName("chain_stack_widget");
   // w_stack->setContentsMargins(0, 0, 0, 0);
    clearStack();

    h_splitter->addWidget(w_list);
    h_splitter->addWidget(w_stack);
   // h_splitter->setContentsMargins(0, 0, 0, 0);
    //w_stack->setFlat(true);

    connect(w_list->listWidget(), SIGNAL(currentRowChanged(int)), this, SLOT(slotChainChanged(int)));

}
void CentralWidgetV3::newPricesReceived(const QJsonArray &j_arr)
{
    bool ok = false;
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("CentralWidgetV3::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QString symbol = j_el.value("symbol").toString().trimmed();
        symbol.remove("USDT");

        float prs = j_el.value("lastPrice").toString().toFloat(&ok);
        if (!ok || prs <= 0) qWarning()<<QString("BB_PricesPage - WARNING invalid convert to float [%1]").arg(j_el.value("lastPrice").toString());

        for (int j=0; j<defi_config.tokens.count(); j++)
        {
            if (defi_config.tokens.at(j).is_stable) continue;
            QString t_name = defi_config.tokens.at(j).name;
            if (defi_config.tokens.at(j).isWraped()) t_name = LString::strTrimLeft(t_name, 1);
            if (t_name == symbol) defi_config.tokens[j].last_price = prs;
        }
    }

    emit signalTabsPricesUpdate();
}
void CentralWidgetV3::clearStack()
{
    while (w_stack->count() > 0)
    {
        QWidget *w = w_stack->widget(0);
        w_stack->removeWidget(w);
        delete w;
        w = NULL;
    }
}
void CentralWidgetV3::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    initHttpRequester();
    loadDefiData();

    int chain_index = settings.value(QString("%1/%2/index").arg(objectName()).arg(w_list->objectName()), 0).toInt();
    if (w_list->listWidget()->count() > 0 && chain_index < w_list->listWidget()->count())
    {
        w_list->listWidget()->item(chain_index)->setSelected(true);
        slotChainChanged(chain_index);
    }

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->load(settings);
    }
}
void CentralWidgetV3::save(QSettings &settings)
{
    LSimpleWidget::save(settings);

    if (w_list->listWidget()->count() > 0)
        settings.setValue(QString("%1/%2/index").arg(objectName()).arg(w_list->objectName()), w_stack->currentIndex());


    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->save(settings);
    }
}
void CentralWidgetV3::loadDefiData()
{
    w_list->listWidget()->clear();
    if (defi_config.chains.isEmpty()) {emit signalError("chain list is empty"); return;}

    foreach (const DefiChain &v, defi_config.chains)
    {        
        w_list->addItem(v.title, v.fullIconPath());
        w_list->listWidget()->setIconSize(QSize(24, 24));

        DefiChainTabV3 *tab = new DefiChainTabV3(this, v.chain_id);
        w_stack->addWidget(tab);

        createTabPages(v.chain_id);
        tab->connectPageSignals();

        connect(tab, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
        connect(tab, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
        connect(tab, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)), this, SLOT(slotRewriteJsonFile(const QJsonObject&, QString)));
        connect(tab, SIGNAL(signalEnableControls(bool)), this, SIGNAL(signalEnableControls(bool)));
        connect(this, SIGNAL(signalTabsPricesUpdate()), tab, SLOT(slotTabsPricesUpdate()));
        connect(tab, SIGNAL(signalGetPrices()), this, SLOT(slotSendHttpReq()));
    }
}
void CentralWidgetV3::createTabPages(int chain_id)
{
    const DefiChainTabV3 *tab = tabByChain(chain_id);
    if (!tab) {emit signalError(QString("tab for chain(%1) is NULL").arg(chain_id)); return;}

    QString chain_icon;
    int i_chain = defi_config.chainIndexOf(chain_id);
    if (i_chain >= 0) chain_icon = defi_config.chains.at(i_chain).fullIconPath();

    //create pages
    DefiWalletTabPage *page = new DefiWalletTabPage(this);
    page->setChain(chain_id);
    tab->tabWidget()->addTab(page, QIcon(chain_icon), AppCommonSettings::tabPageTitle(page->kind()));
    page->initTokenList(chain_id);

    connect(page, SIGNAL(signalGetPrices()), tab, SIGNAL(signalGetPrices()));

}
const DefiChainTabV3* CentralWidgetV3::tabByChain(int chain_id) const
{
    for (int i=0; i<w_stack->count(); i++)
    {
        QWidget *w = w_stack->widget(i);
        const DefiChainTabV3 *tab = qobject_cast<const DefiChainTabV3*>(w);
        if (tab)
        {
            if (tab->chainId() == chain_id) return tab;
        }
    }
    return NULL;
}
DefiChainTabV3* CentralWidgetV3::currentTab() const
{
    QWidget *w = w_stack->currentWidget();
    DefiChainTabV3 *tab = qobject_cast<DefiChainTabV3*>(w);
    return tab;
}
void CentralWidgetV3::slotChainChanged(int i)
{
    qDebug()<<QString("CentralWidgetV3::slotChainChanged i=%1").arg(i);
    if (i >=0 && i < w_stack->count())
    {
        w_stack->setCurrentIndex(i);
        DefiChainTabV3 *tab = currentTab();
        if (tab) tab->tabActivated();
    }
    else emit signalError(QString("CentralWidgetV3:  invalid stack index(%1)").arg(i));
}
void CentralWidgetV3::slotRewriteJsonFile(const QJsonObject &j_params, QString fname)
{
    qDebug()<<QString("CentralWidgetV3::slotRewriteJsonFile  fname[%1]").arg(fname);
    QJsonDocument j_doc(j_params);
    QString fdata(j_doc.toJson());
    fdata.append(QChar('\n'));

    QString nodejs_dir(AppCommonSettings::nodejsPath().trimmed());
    if (nodejs_dir.isEmpty())
    {
        emit signalError("nodejs scripts path is empty");
        return;
    }
    if (!LFile::dirExists(nodejs_dir))
    {
        emit signalError(QString("nodejs scripts dir[%1] not found").arg(nodejs_dir));
        return;
    }

    QString full_name = QString("%1%2%3").arg(nodejs_dir).arg(QDir::separator()).arg(fname);
    emit signalMsg(QString("writing json file for node_js script [%1].........").arg(fname));
    QString err = LFile::writeFile(full_name, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}
    else emit signalMsg("JSON file done!");
}
void CentralWidgetV3::breakUpdating()
{
    qDebug("CentralWidgetV3::breakUpdating()");
    DefiChainTabV3 *tab = currentTab();
    if (tab) tab->stopUpdating();

}


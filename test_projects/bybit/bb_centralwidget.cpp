#include "bb_centralwidget.h"
#include "bb_chartpage.h"
#include "lhttp_types.h"
#include "lhttpapirequester.h"
#include "jsonviewpage.h"
#include "bb_apistruct.h"
#include "apiconfig.h"
#include "bb_positionspage.h"
#include "bb_historypage.h"
#include "bb_bagstatepage.h"

#include <QStackedWidget>
#include <QSplitter>
#include <QSettings>
#include <QDebug>
#include <QAction>
#include <QDateTime>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#define API_SERV    QString("api.bybit.com")


//BB_CentralWidget
BB_CentralWidget::BB_CentralWidget(QWidget *parent)
    :LSimpleWidget(parent, 20),
      w_list(NULL),
      w_stack(NULL),
      m_reqObj(NULL)
{
    setObjectName("central_widget");
    init();
    createPages();
    initReqObject();

    connect(w_list->listWidget(), SIGNAL(currentRowChanged(int)), this, SLOT(slotPageChanged(int)));
    connect(this, SIGNAL(signalEnableControls(bool)), this, SLOT(slotEnableControls(bool)));
    connect(w_stack, SIGNAL(currentChanged(int)), this, SLOT(slotPageActivated(int)));

}
void BB_CentralWidget::slotPageChanged(int i)
{
   // qDebug()<<QString("slotPageChanged  i=%1  pageCount %2").arg(i).arg(pageCount());
    if (i < 0 || i >= pageCount()) return;
    w_stack->setCurrentIndex(i);
}
void BB_CentralWidget::slotPageActivated(int i)
{
    qDebug("BB_CentralWidget::slotPageActivated");
    if (i < 0 || i >= pageCount()) return;

    BB_BasePage *page = qobject_cast<BB_BasePage*>(w_stack->widget(i));
    if (page)
    {
        signalPageChanged(page->userSign());
        page->updateDataPage(false);
    }
}
int BB_CentralWidget::pageCount() const
{
    if (w_stack) return w_stack->count();
    return -1;
}
void BB_CentralWidget::createPages()
{
    JSONViewPage *j_page = new JSONViewPage(this);
    w_stack->addWidget(j_page);

    BB_ChartPage *p_chart = new BB_ChartPage(this);
    w_stack->addWidget(p_chart);

    BB_PositionsPage *pos_page = new BB_PositionsPage(this);
    w_stack->addWidget(pos_page);

    BB_HistoryPage *h_page = new BB_HistoryPage(this);
    w_stack->addWidget(h_page);

    BB_BagStatePage *bs_page = new BB_BagStatePage(this);
    w_stack->addWidget(bs_page);
    connect(bs_page, SIGNAL(signalGetPosState(BB_BagState&)), pos_page, SLOT(slotGetPosState(BB_BagState&)));
    connect(bs_page, SIGNAL(signalGetHistoryState(BB_HistoryState&)), h_page, SLOT(slotGetHistoryState(BB_HistoryState&)));

    BB_FundRatePage *fr_page = new BB_FundRatePage(this);
    w_stack->addWidget(fr_page);

    for (int i=0; i<w_stack->count(); i++)
    {
        BB_BasePage *w = qobject_cast<BB_BasePage*>(w_stack->widget(i));
        if (w)
        {
            w_list->addItem(w->caption(), w->iconPath());
            connect(w, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
            connect(w, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
            connect(w, SIGNAL(signalSendReq(const BB_APIReqParams*)), this, SLOT(slotSendReq(const BB_APIReqParams*)));
            connect(this, SIGNAL(signalJsonReply(int, const QJsonObject&)), w, SLOT(slotJsonReply(int, const QJsonObject&)));
        }
    }
}
void BB_CentralWidget::init()
{
    w_list = new LListWidgetBox(this);
    w_list->setTitle("Pages");
    w_list->setBaseColor("#EFECEC", "#556B2F");

    w_stack = new QStackedWidget(this);
    clearStack();

    h_splitter->addWidget(w_list);
    h_splitter->addWidget(w_stack);
}
void BB_CentralWidget::initReqObject()
{
    m_reqObj = new LHttpApiRequester(this);
    m_reqObj->setHttpProtocolType(hptHttps);
    m_reqObj->setApiServer(API_SERV);

    connect(m_reqObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_reqObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_reqObj, SIGNAL(signalFinished(int)), this, SLOT(slotReqFinished(int)));
}
void BB_CentralWidget::clearStack()
{
    while (w_stack->count() > 0)
    {
        QWidget *w = w_stack->widget(0);
        w_stack->removeWidget(w);
        delete w;
        w = NULL;
    }
}
void BB_CentralWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->load(settings);
    }

    //int cp = settings.value(QString("%1/current_page").arg(objectName()), 0).toInt();
    //w_list->listWidget()->setCurrentRow(cp);
    w_list->listWidget()->setCurrentRow(0);

    BB_BasePage *page = qobject_cast<BB_BasePage*>(w_stack->currentWidget());
    if (page) signalPageChanged(page->userSign());

}
void BB_CentralWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->save(settings);
    }

    settings.setValue(QString("%1/current_page").arg(objectName()), w_stack->currentIndex());
}
void BB_CentralWidget::slotReqFinished(int result)
{
    emit signalMsg(QString("next request finished, result %1").arg(result));
    emit signalEnableControls(true);

    const LHttpApiReplyData& r = m_reqObj->lastReply();
    if (r.data.isEmpty()) emit signalMsg("WARNING: REPLY JSON IS EMPTY");

    if (r.isOk()) emit signalJsonReply(userSign(), r.data);
    else emit signalError("request fault");

    //out headers of reply
    emit signalMsg(QString("------------headers %1---------------").arg(r.headers.count()));
    foreach (const QString v, r.headers) emit signalMsg(v);
}
void BB_CentralWidget::prepareReq(const BB_APIReqParams *req_data)
{
    qint64 ts = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    QString hmac_msg = QString("%1%2%3%4").arg(ts).arg(api_config.api_key).arg(api_config.req_delay).arg(req_data->paramsLine());
    QByteArray ba_sign(APIConfig::calcHMACSha256(api_config.api_key_private.toLatin1(), hmac_msg.toLatin1()));

    m_reqObj->addReqHeader(QString("X-BAPI-API-KEY"), api_config.api_key);
    m_reqObj->addReqHeader(QString("X-BAPI-TIMESTAMP"), QString::number(ts));
    m_reqObj->addReqHeader(QString("X-BAPI-SIGN"), QString(ba_sign.toHex()));
    m_reqObj->addReqHeader(QString("X-BAPI-RECV-WINDOW"), QString::number(api_config.req_delay));
    m_reqObj->setUri(req_data->fullUri());
}
void BB_CentralWidget::slotSendReq(const BB_APIReqParams *req_data)
{
    if (!req_data) {emit signalError("request data is NULL"); return;}
    emit signalMsg(QString("\n Start request [%1] ...").arg(req_data->name));
    if (req_data->invalid()) {emit signalError("request data is invalid"); return;}
    if (requesterBuzy()) {emit signalError("Requester object is buzy."); return;}

    qDebug()<<QString("BB_CentralWidget::slotSendReq req_type=%1").arg(req_data->req_type);
    m_userSign = req_data->req_type;
    prepareReq(req_data);
    emit signalMsg(QString("URL: %1").arg(m_reqObj->fullUrl()));
    emit signalEnableControls(false);

   // qDebug()<<req_data->toStr();
    m_reqObj->start(req_data->metod);
}
void BB_CentralWidget::slotEnableControls(bool b)
{
    w_list->setEnabled(b);
    w_stack->setEnabled(b);
}
void BB_CentralWidget::setUpdatingInterval(quint16 secs)
{
    for (int i=0; i<w_stack->count(); i++)
    {
        BB_BasePage *w = qobject_cast<BB_BasePage*>(w_stack->widget(i));
        if (w) w->setMinUpdatingInterval(secs);
    }
}
void BB_CentralWidget::setExpandLevel(int l)
{
    JSONViewPage *j_page = qobject_cast<JSONViewPage*>(w_stack->widget(0));
    if (j_page) j_page->setExpandLevel(l);
}
bool BB_CentralWidget::requesterBuzy() const
{
    if (m_reqObj) return m_reqObj->isBuzy();
    return false;
}
void BB_CentralWidget::updateDataPage()
{
    BB_BasePage *page = qobject_cast<BB_BasePage*>(w_stack->currentWidget());
    if (page) page->updateDataPage(true);
}
void BB_CentralWidget::actAdd()
{
    BB_ChartPage *page = qobject_cast<BB_ChartPage*>(w_stack->currentWidget());
    if (page) page->addFavorToken();
}
void BB_CentralWidget::actRemove()
{
    BB_ChartPage *page = qobject_cast<BB_ChartPage*>(w_stack->currentWidget());
    if (page) page->removeFavorToken();
}


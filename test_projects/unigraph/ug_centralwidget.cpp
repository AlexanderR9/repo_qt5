#include "ug_centralwidget.h"
#include "lhttp_types.h"
#include "lhttpapirequester.h"
#include "ug_jsonviewpage.h"
#include "subgraphreq.h"
#include "ug_apistruct.h"
#include "ug_poolpage.h"
#include "ug_tokenpage.h"


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
//#include <QLineEdit>
#include <QVBoxLayout>


//UG_CentralWidget
UG_CentralWidget::UG_CentralWidget(QWidget *parent)
    :LSimpleWidget(parent, 32),
      w_list(NULL),
      w_stack(NULL),
      m_reqObj(NULL),
      m_graphReqObj(NULL)
{
    setObjectName("ug_central_widget");
    init();
    createPages();
    initReqObject();

    connect(w_list->listWidget(), SIGNAL(currentRowChanged(int)), this, SLOT(slotPageChanged(int)));
    connect(this, SIGNAL(signalEnableControls(bool)), this, SLOT(slotEnableControls(bool)));
    connect(w_stack, SIGNAL(currentChanged(int)), this, SLOT(slotPageActivated(int)));

    m_graphReqObj = new SubGraphReq();

}
void UG_CentralWidget::setApiKeys(QString akey, QString sub_id)
{
    if (m_graphReqObj) m_graphReqObj->setApiKeys(akey, sub_id);
}
void UG_CentralWidget::setApiServer(QString s)
{
    if (m_reqObj) m_reqObj->setApiServer(s);
    qDebug()<<s;
}
void UG_CentralWidget::slotPageChanged(int i)
{
   // qDebug()<<QString("slotPageChanged  i=%1  pageCount %2").arg(i).arg(pageCount());
    if (i < 0 || i >= pageCount()) return;
    w_stack->setCurrentIndex(i);
}
void UG_CentralWidget::slotPageActivated(int i)
{
    //qDebug("BB_CentralWidget::slotPageActivated");
    if (i < 0 || i >= pageCount()) return;

    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->widget(i));
    if (page)
    {
        emit signalPageChanged(page->userSign());
        //page->updateDataPage(false);
    }
}
int UG_CentralWidget::pageCount() const
{
    if (w_stack) return w_stack->count();
    return -1;
}
void UG_CentralWidget::createPages()
{
   // qDebug("UG_CentralWidget::createPages() 1");
    UG_JSONViewPage *j_page = new UG_JSONViewPage(this);
    w_stack->addWidget(j_page);

    UG_PoolPage *p_page = new UG_PoolPage(this);
    w_stack->addWidget(p_page);
    connect(p_page, SIGNAL(signalGetFilterParams(quint16&, double&)), this, SIGNAL(signalGetFilterParams(quint16&, double&)));

    UG_TokenPage *t_page = new UG_TokenPage(this);
    w_stack->addWidget(t_page);
    connect(t_page, SIGNAL(signalGetTokensFromPoolPage(QHash<QString, QString>&)), p_page, SLOT(slotSetTokensFromPage(QHash<QString, QString>&)));


    for (int i=0; i<w_stack->count(); i++)
    {
        UG_BasePage *w = qobject_cast<UG_BasePage*>(w_stack->widget(i));
        if (w)
        {
            w_list->addItem(w->caption(), w->iconPath());
            connect(w, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
            connect(w, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
            connect(w, SIGNAL(signalSendReq(const UG_APIReqParams*)), this, SLOT(slotSendReq(const UG_APIReqParams*)));
            connect(w, SIGNAL(signalStopUpdating()), this, SLOT(slotStopUpdating()));
            connect(this, SIGNAL(signalJsonReply(int, const QJsonObject&)), w, SLOT(slotJsonReply(int, const QJsonObject&)));
            connect(this, SIGNAL(signalBuzy()), w, SLOT(slotReqBuzyNow()));
        }
    }
}
void UG_CentralWidget::init()
{
    w_list = new LListWidgetBox(this);
    w_list->setTitle("Pages");
    w_list->setBaseColor("#EFECEC", "#556B2F");

    w_stack = new QStackedWidget(this);
    clearStack();

    h_splitter->addWidget(w_list);
    v_splitter->addWidget(w_stack);
}
void UG_CentralWidget::initReqObject()
{
    m_reqObj = new LHttpApiRequester(this);
    m_reqObj->setHttpProtocolType(hptHttps);
    m_reqObj->addReqHeader(QString("Content-Type"), QString("application/json"));
    m_reqObj->addReqHeader(QString("accept"), QString("application/json"));

    connect(m_reqObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_reqObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_reqObj, SIGNAL(signalFinished(int)), this, SLOT(slotReqFinished(int)));
}
void UG_CentralWidget::clearStack()
{
    while (w_stack->count() > 0)
    {
        QWidget *w = w_stack->widget(0);
        w_stack->removeWidget(w);
        delete w;
        w = NULL;
    }
}
void UG_CentralWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->load(settings);
    }

    int cpi = settings.value(QString("%1/current_page").arg(objectName()), 0).toInt();
    w_list->listWidget()->setCurrentRow(cpi);

    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->currentWidget());
    if (page) emit signalPageChanged(page->userSign());
}
void UG_CentralWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->save(settings);
    }
    settings.setValue(QString("%1/current_page").arg(objectName()), w_stack->currentIndex());
}
void UG_CentralWidget::slotReqFinished(int result)
{
    qDebug()<<QString("REQUEST FINISHED result=%1").arg(result);
    emit signalMsg(QString("next request finished, result %1").arg(result));
    if (!updatingRunning()) {qDebug("!updatingRunning()"); return;}

    const LHttpApiReplyData& r = m_reqObj->lastReply();
    if (r.data.isEmpty()) emit signalMsg("WARNING: REPLY JSON IS EMPTY");
    if (r.isOk()) emit signalJsonReply(userSign(), r.data);
    else emit signalError("request fault");

    if (w_stack->currentIndex() == 0)
    {
        slotStopUpdating();
        //emit signalEnableControls(true);
    }
    else
    {
        if (!r.isOk())
        {
            emit signalError("Updating breaked");
            slotStopUpdating();
            //emit signalEnableControls(true);
        }
    }





    return;
    //out headers of reply
    emit signalMsg("");
    emit signalMsg("FETCHED RESPONSE:");
    emit signalMsg(QString("------------headers %1---------------").arg(r.headers.count()));
    foreach (const QString v, r.headers) {emit signalMsg(v);}
}
void UG_CentralWidget::slotSendReq(const UG_APIReqParams *req_data)
{
    m_reqObj->clearMetaData();
    if (isBuzy())
    {
        emit signalError("Requester object is buzy.");
        emit signalBuzy();
        return;
    }

    emit signalMsg(QString("\n Start request [%1] ...").arg(req_data->name));
    if (req_data->invalid()) {emit signalError("request data is invalid"); return;}

    //qDebug()<<QString("BB_CentralWidget::slotSendReq req_type=%1").arg(req_data->req_type);
    m_reqObj->setUri(m_graphReqObj->graphUri());
    m_userSign = req_data->req_type;
    m_reqObj->addMetaData("query", req_data->query);

    sendQuery();
}
void UG_CentralWidget::slotEnableControls(bool b)
{
    w_list->setEnabled(b);
    w_stack->setEnabled(b);
}
void UG_CentralWidget::setUpdatingInterval(quint16 secs)
{
    for (int i=0; i<w_stack->count(); i++)
    {
        UG_BasePage *w = qobject_cast<UG_BasePage*>(w_stack->widget(i));
        if (w) w->setMinUpdatingInterval(secs);
    }
}
void UG_CentralWidget::setExpandLevel(int l)
{
    UG_JSONViewPage *j_page = qobject_cast<UG_JSONViewPage*>(w_stack->widget(0));
    if (j_page) j_page->setExpandLevel(l);
}
void UG_CentralWidget::setViewPrecision(quint8 p)
{
    UG_JSONViewPage *j_page = qobject_cast<UG_JSONViewPage*>(w_stack->widget(0));
    if (j_page) j_page->setPrecision(p);
}
bool UG_CentralWidget::requesterBuzy() const
{
    if (m_reqObj) return m_reqObj->isBuzy();
    return false;
}
void UG_CentralWidget::updateDataPage()
{
    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->currentWidget());
    if (!page) return;

    if (page->userSign() == rtJsonView) freeReq();
//    else page->updateDataPage(true);
}
void UG_CentralWidget::updateDataPage(quint16 t)
{
    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->currentWidget());
    if (!page) return;

    qDebug("//////////////start updating page, many req///////////////");

    if (page->userSign() != rtJsonView)
    {
        emit signalEnableControls(false);
        page->startUpdating(t);
    }
    else emit signalError(QString("current page is rtJsonView"));
}
void UG_CentralWidget::slotStopUpdating()
{
    qDebug("UG_CentralWidget::stopUpdating()");
    for (int i=0; i<w_stack->count(); i++)
    {
        UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->widget(i));
        if (page)
        {
            if (page->updatingRunning())
                page->stopUpdating();
        }
    }
    //slotEnableControls(true);
    emit signalEnableControls(true);
}
bool UG_CentralWidget::updatingRunning() const
{
    for (int i=0; i<w_stack->count(); i++)
    {
        UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->widget(i));
        if (page)
        {
            //qDebug()<<QString("page_%1").arg(i);
            if (page->updatingRunning()) return true;
        }
        else qWarning()<<QString("UG_CentralWidget : WARNING page is NULL, index %1").arg(i);
    }
    return false;
}
bool UG_CentralWidget::isBuzy() const
{
    if (!m_reqObj) return true;
    return m_reqObj->isBuzy();
}
void UG_CentralWidget::freeReq()
{
    qDebug("//////////////start free req///////////////");
    emit signalMsg(QString("try send FREE request.... "));
    if (isBuzy())
    {
        emit signalError("request object is buzy");
        emit signalEnableControls(true);
        return;
    }

    m_reqObj->clearMetaData();
    m_reqObj->setUri(m_graphReqObj->graphUri());

    QString s_query(qobject_cast<UG_JSONViewPage*>(w_stack->currentWidget())->freeQueryData());
    if (s_query.isEmpty())
    {
        emit signalError("Query data is empty");
        return;
    }
    m_reqObj->addMetaData("query", s_query);

    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->currentWidget());
    if (page) page->startUpdating(12000);

    sendQuery();
}
void UG_CentralWidget::sendQuery()
{
    qDebug("-------------------------------------------");
    emit signalMsg(QString("URL: %1").arg(m_reqObj->fullUrl()));
    qDebug()<<m_reqObj->metadata();
    emit signalEnableControls(false);
    m_reqObj->start(hrmPost);
}
void UG_CentralWidget::actAdd()
{
    //BB_ChartPage *page = qobject_cast<BB_ChartPage*>(w_stack->currentWidget());
    //if (page) page->addFavorToken();
}
void UG_CentralWidget::actRemove()
{
    //BB_ChartPage *page = qobject_cast<BB_ChartPage*>(w_stack->currentWidget());
    //if (page) page->removeFavorToken();
}
void UG_CentralWidget::actStop()
{
    //BB_ShadowExplorer *page = qobject_cast<BB_ShadowExplorer*>(w_stack->currentWidget());
    //if (page) page->stopTimer();
}
void UG_CentralWidget::actLoadData()
{
    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->currentWidget());
    if (page) page->loadData();
}
void UG_CentralWidget::actSaveData()
{
    UG_BasePage *page = qobject_cast<UG_BasePage*>(w_stack->currentWidget());
    if (page) page->saveData();
}
void UG_CentralWidget::actChart()
{
    //BB_ShadowExplorer *page = qobject_cast<BB_ShadowExplorer*>(w_stack->currentWidget());
    //if (page) page->showChart();
}


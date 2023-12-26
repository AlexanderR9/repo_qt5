#include "bb_centralwidget.h"
#include "bb_chartpage.h"
#include "lhttp_types.h"
#include "lhttpapirequester.h"
#include "jsonviewpage.h"
#include "bb_apistruct.h"
#include "apiconfig.h"
#include "bb_positionspage.h"

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
}
void BB_CentralWidget::slotPageChanged(int i)
{
    qDebug()<<QString("slotPageChanged  i=%1  pageCount %2").arg(i).arg(pageCount());
    if (i < 0 || i >= pageCount()) return;
    w_stack->setCurrentIndex(i);

}
int BB_CentralWidget::pageCount() const
{
    if (w_stack) return w_stack->count();
    return -1;
}
void BB_CentralWidget::createPages()
{
    BB_ChartPage *p_chart = new BB_ChartPage(this);
    w_stack->addWidget(p_chart);
    connect(p_chart, SIGNAL(signalSendReq(const BB_APIReqParams&)), this, SLOT(slotSendReq(const BB_APIReqParams&)));
    connect(this, SIGNAL(signalJsonReply(int, const QJsonObject&)), p_chart, SLOT(slotJsonReply(int, const QJsonObject&)));

    JSONViewPage *j_page = new JSONViewPage(this);
    w_stack->addWidget(j_page);
    connect(this, SIGNAL(signalJsonReply(int, const QJsonObject&)), j_page, SLOT(slotReloadJsonReply(int, const QJsonObject&)));

    BB_PositionsPage *pos_page = new BB_PositionsPage(this);
    w_stack->addWidget(pos_page);
    connect(pos_page, SIGNAL(signalSendReq(const BB_APIReqParams&)), this, SLOT(slotSendReq(const BB_APIReqParams&)));
    connect(this, SIGNAL(signalJsonReply(int, const QJsonObject&)), pos_page, SLOT(slotJsonReply(int, const QJsonObject&)));


    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w)
        {
            w_list->addItem(w->caption(), w->iconPath());
            connect(w, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
            connect(w, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
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

    int cp = settings.value(QString("%1/current_page").arg(objectName()), 0).toInt();
    w_list->listWidget()->setCurrentRow(cp);

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
void BB_CentralWidget::prepareReq(const BB_APIReqParams &req_data)
{
    qint64 ts = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    QString hmac_msg = QString("%1%2%3%4").arg(ts).arg(api_config.api_key).arg(api_config.req_delay).arg(req_data.paramsLine());
    QByteArray ba_sign(APIConfig::calcHMACSha256(api_config.api_key_private.toLatin1(), hmac_msg.toLatin1()));

    m_reqObj->addReqHeader(QString("X-BAPI-API-KEY"), api_config.api_key);
    m_reqObj->addReqHeader(QString("X-BAPI-TIMESTAMP"), QString::number(ts));
    m_reqObj->addReqHeader(QString("X-BAPI-SIGN"), QString(ba_sign.toHex()));
    m_reqObj->addReqHeader(QString("X-BAPI-RECV-WINDOW"), QString::number(api_config.req_delay));
    m_reqObj->setUri(req_data.fullUri());
}
void BB_CentralWidget::slotSendReq(const BB_APIReqParams &req_data)
{
    emit signalMsg(QString("\n Start request [%1] ...").arg(req_data.name));
    if (req_data.invalid())
    {
        emit signalError("request data is invalid");
        return;
    }
    if (requesterBuzy())
    {
        emit signalError("Requester object is buzy.");
        return;
    }


    m_userSign = req_data.req_type;
    prepareReq(req_data);
    emit signalMsg(QString("URL: %1").arg(m_reqObj->fullUrl()));
    emit signalEnableControls(false);

    qDebug()<<req_data.toStr();
    m_reqObj->start(req_data.metod);

}
void BB_CentralWidget::slotEnableControls(bool b)
{
    w_list->setEnabled(b);
    w_stack->setEnabled(b);
}
void BB_CentralWidget::setExpandLevel(int l)
{
    JSONViewPage *j_page = qobject_cast<JSONViewPage*>(w_stack->widget(1));
    if (j_page) j_page->setExpandLevel(l);
}
bool BB_CentralWidget::requesterBuzy() const
{
    if (m_reqObj) return m_reqObj->isBuzy();
    return false;
}
void BB_CentralWidget::updateDataPage()
{
    if (w_stack->currentIndex() == 2)
    {
        BB_PositionsPage *page = qobject_cast<BB_PositionsPage*>(w_stack->currentWidget());
        if (page) page->updateDataPage();
    }
}



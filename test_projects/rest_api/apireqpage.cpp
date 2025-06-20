#include "apireqpage.h"
#include "lfile.h"
#include "lhttpapirequester.h"
#include "apipages.h"
#include "apiorderspage.h"
#include "apicommonsettings.h"
#include "lhttp_types.h"
#include "reqpreparer.h"
#include "cycleworker.h"
#include "lsplash.h"

#include <QListWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTest>


//APIReqPage
APIReqPage::APIReqPage(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_sourceBox(NULL),
      m_replyBox(NULL),      
      m_reqObj(NULL),
      m_reqPreparer(NULL),
      m_printHeaders(true),
      m_cycleWroker(NULL),
      m_needUpdateInfo(ruiNone)
{
    setObjectName("api_req_page");
    m_userSign = aptReq;

    initWidgets();
    initSources();
    initReqObject();
    initReqPreparer();
    initCycleWorker();
}
APIReqPage::~APIReqPage()
{
    if (m_reqPreparer) {delete m_reqPreparer; m_reqPreparer = NULL;}
    if (m_reqObj) {delete m_reqObj; m_reqObj = NULL;}
}
void APIReqPage::updateOrders()
{
    m_needUpdateInfo = ruiOrders;
    needUpdateInfo();
}
void APIReqPage::updateBag()
{
    m_needUpdateInfo = ruiBagPositions;
    needUpdateInfo();
}
void APIReqPage::getVisibleBondPrices()
{
    if (!selectSrcRow("of bonds"))
    {
        emit signalError("[price of bonds]:SRC not found by list.");
        emit signalFinished(hreWrongReqParams);
        return;
    }
    trySendReq();
}
void APIReqPage::updateEvents()
{
    if (!selectSrcRow("getoperation"))
    {
        emit signalError("[getoperation]:SRC not found by list.");
        emit signalFinished(hreWrongReqParams);
        return;
    }
    trySendReq();
}
void APIReqPage::needUpdateInfo()
{
    emit signalDisableActions(true);
    switch (m_needUpdateInfo)
    {
        case ruiOrders:
        {
            if (!selectSrcRow("getorders"))
            {
                m_needUpdateInfo = ruiNone;
                emit signalError("[getorders]:SRC not found by list.");
                emit signalFinished(hreWrongReqParams);
                return;
            }
            m_needUpdateInfo = ruiStopOrders;
            trySendReq();
            break;
        }
        case ruiStopOrders:
        {
            if (!selectSrcRow("getstoporders"))
            {
                m_needUpdateInfo = ruiNone;
                emit signalError("[getstoporders]:SRC not found by list.");
                emit signalFinished(hreWrongReqParams);
                return;
            }
            m_needUpdateInfo = ruiNone;
            trySendReq();
            break;
        }
        case ruiBagPositions:
        {
            if (!selectSrcRow("getpos"))
            {
                m_needUpdateInfo = ruiNone;
                emit signalError("[getpos]:SRC not found by list.");
                emit signalFinished(hreWrongReqParams);
                return;
            }
            m_needUpdateInfo = ruiBagAmount;
            trySendReq();
            break;
        }
        case ruiBagAmount:
        {
            if (!selectSrcRow("getportf"))
            {
                m_needUpdateInfo = ruiNone;
                emit signalError("[getportf]:SRC not found by list.");
                emit signalFinished(hreWrongReqParams);
                return;
            }
            m_needUpdateInfo = ruiNone;
            trySendReq();
            break;
        }
        default:
        {
            emit signalError(QString("invalid update info code: %1").arg(m_needUpdateInfo));
            emit signalFinished(hreWrongReqParams);
            m_needUpdateInfo = ruiNone;
            break;
        }
    }
}
bool APIReqPage::selectSrcRow(QString s) const
{
    m_sourceBox->listWidget()->clearSelection();
    m_sourceBox->listWidget()->clearFocus();
    if (s.trimmed().isEmpty()) return false;

    for (int i=0; i<m_sourceBox->listWidget()->count(); i++)
    {
        QListWidgetItem *l_item = m_sourceBox->listWidget()->item(i);
        if (l_item->text().toLower().contains(s))
        {
            m_sourceBox->listWidget()->setCurrentRow(i);
            return true;
        }
    }
    return false;
}
void APIReqPage::slotReqFinished(int result)
{
    if (m_cycleWroker->cycleModeOn())
    {
        if (result == hreOk || result == hreServerErr)
        {
            checkReply();
            if (replyOk()) emit signalMsg("REPLY CODE OK:");
        }
        emit signalMsg("next cycle request finished \n");
    }
    else emit signalFinished(result);
}
void APIReqPage::initReqObject()
{
    m_reqObj = new LHttpApiRequester(this);
    connect(m_reqObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_reqObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_reqObj, SIGNAL(signalFinished(int)), this, SLOT(slotReqFinished(int)));
}
void APIReqPage::initReqPreparer()
{
    m_reqPreparer = new ApiReqPreparer(m_reqObj, this);
    connect(m_reqPreparer, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_reqPreparer, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_reqPreparer, SIGNAL(signalGetReqParams(QString&, QString&)), this, SIGNAL(signalGetReqParams(QString&, QString&)));
    connect(m_reqPreparer, SIGNAL(signalGetSelectedBondUID(QString&)), this, SIGNAL(signalGetSelectedBondUID(QString&)));
    connect(m_reqPreparer, SIGNAL(signalGetSelectedStockUID(QString&)), this, SIGNAL(signalGetSelectedStockUID(QString&)));
    connect(m_reqPreparer, SIGNAL(signalGetSelectedBondUIDList(QStringList&)), this, SIGNAL(signalGetSelectedBondUIDList(QStringList&)));
    connect(m_reqPreparer, SIGNAL(signalGetSelectedStockUIDList(QStringList&)), this, SIGNAL(signalGetSelectedStockUIDList(QStringList&)));
    connect(m_reqPreparer, SIGNAL(signalGetPricesDepth(quint16&)), this, SIGNAL(signalGetPricesDepth(quint16&)));
    connect(m_reqPreparer, SIGNAL(signalGetCandleSize(QString&)), this, SIGNAL(signalGetCandleSize(QString&)));
}
void APIReqPage::initCycleWorker()
{
    m_cycleWroker = new CycleWorker(this);
    connect(m_cycleWroker, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_cycleWroker, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_cycleWroker, SIGNAL(signalFinished()), this, SLOT(slotCycleWorkerFinished()));
    connect(m_cycleWroker, SIGNAL(signalNextReq()), this, SLOT(slotCycleWorkerNextReq()));
    connect(m_cycleWroker, SIGNAL(signalCyclePrice(const QString&, float)), this, SIGNAL(signalCyclePrice(const QString&, float)));

}
void APIReqPage::resetPage()
{
    m_replyBox->clearView();
}
void APIReqPage::initWidgets()
{
    m_sourceBox = new LListWidgetBox(this);
    m_sourceBox->setTitle("API sources");
    m_replyBox = new LTreeWidgetBox(this);
    m_replyBox->setTitle("Reply data");

    QStringList headers;
    headers << "Key" << "Value" << "Data type";
    m_replyBox->setHeaderLabels(headers);
    headers.clear();
    headers << "JSON struct" << QString() << QString();
    m_replyBox->addRootItem(headers);
    m_replyBox->setRootItemAttrs(Qt::darkCyan, 0, false, true);
    m_replyBox->resizeByContents();
    m_replyBox->view()->setSelectionMode(QAbstractItemView::SingleSelection);
    m_replyBox->view()->setSelectionBehavior(QAbstractItemView::SelectRows);

    h_splitter->addWidget(m_sourceBox);
    h_splitter->addWidget(m_replyBox);
}
void APIReqPage::initSources()
{
    quint16 row = 0;
    foreach(const QString &v, api_commonSettings.services)
    {
        //qDebug()<<QString("SRC: %1").arg(v);
        m_sourceBox->listWidget()->addItem(v);
        int pos = v.indexOf('/');
        if (pos > 0)
        {
            QString s_color = api_commonSettings.service_colors.value(v.left(pos));
            //qDebug()<<QString("pos=%1,  v=%2, color=%3").arg(pos).arg(v.left(pos)).arg(s_color);
            m_sourceBox->setRowTextColor(row, s_color);
        }
        row++;
    }
}
void APIReqPage::slotTrySendOrderReq(const PlaceOrderData &req_data)
{
    if (requesterBuzy())
    {
        emit signalError("Requester object is buzy.");
        emit signalFinished(hreBuzy);
        return;
    }
    m_reqPreparer->clearCycleData();
    if (req_data.invalid())
    {
        emit signalError("Order request data is invalid.");
        emit signalFinished(hreWrongReqParams);
        return;
    }

    QString need_src = req_data.isCancel() ? "cancel" : "post";
    if (req_data.is_stop) need_src.append("stop");
    need_src.append("order");
    if (!selectSrcRow(need_src))
    {
        emit signalError(QString("[%1]:SRC not found by list.").arg(need_src));
        emit signalFinished(hreWrongReqParams);
        return;
    }

    emit signalDisableActions(true);
    int row = m_sourceBox->listWidget()->currentRow();
    standardRequest(m_sourceBox->listWidget()->item(row)->text(), &req_data);
}
void APIReqPage::trySendReq()
{
    m_reqPreparer->clearCycleData();
    int row = m_sourceBox->listWidget()->currentRow();
    if (row < 0)
    {
        emit signalError("You must select API source.");
        emit signalFinished(hreWrongReqParams);
        return;
    }
    if (requesterBuzy())
    {
        emit signalError("Requester object is buzy.");
        emit signalFinished(hreBuzy);
        return;
    }

    //check cycle mode
    QString src = m_sourceBox->listWidget()->item(row)->text();
    m_cycleWroker->checkCycleMode(src);
    if (m_cycleWroker->cycleModeOn())
    {
        //turn on mode cycle
        prepareCycleData();
        if (!m_cycleWroker->cycleModeOn()) emit signalFinished(hreUnknown);
        else m_sourceBox->setEnabled(false);
        return;
    }

    /////////////////STANDARD REQ////////////////////////////////
    standardRequest(src);
}
void APIReqPage::toDebugReqMetadata()
{
    qDebug()<<QString("/////////// REQUEST METADATA ///////////////");
    const QJsonObject &j_obj = m_reqObj->metadata();
    QJsonObject::const_iterator it =  j_obj.constBegin();
    while (it != j_obj.constEnd())
    {
        if (it.value().isObject())
        {
            qDebug()<<QString("KEY[%1] => OBJ").arg(it.key());
            QJsonObject::const_iterator it2 =  it.value().toObject().constBegin();
            while (it2 != it.value().toObject().constEnd())
            {
                if (it2.value().isDouble()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (DOUBLE)").arg(it2.key()).arg(it2.value().toDouble());
                else if (it2.value().isBool()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (BOOL)").arg(it2.key()).arg(it2.value().toBool());
                else qDebug()<<QString("   KEY[%1] => VALUE[%2]").arg(it2.key()).arg(it2.value().toString());
                it2++;
            }
        }
        else
        {
            if (it.value().isDouble()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (DOUBLE)").arg(it.key()).arg(it.value().toDouble());
            else if (it.value().isBool()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (BOOL)").arg(it.key()).arg(it.value().toBool());
            else qDebug()<<QString("KEY[%1] => VALUE[%2]").arg(it.key()).arg(it.value().toString());
        }
        it++;
    }
    qDebug()<<QString();
}
void APIReqPage::standardRequest(const QString &src, const PlaceOrderData *req_data)
{
    qDebug()<<QString("APIReqPage::standardRequest  src[%1]  req_data[%2]").arg(src).arg(req_data?"TRUE":"NULL");
    m_reqPreparer->prepare(src, req_data);
    if (m_reqPreparer->invalidReq())
    {
        qDebug("req is FAULT");
        int e_code = hreUnknown;
        if (m_reqObj->metadata().value("err").isString()) e_code = m_reqObj->metadata().value("err").toString().toInt();
        else emit signalError(QString("invalid metadata err_value type"));
        emit signalError(QString("request did't start (code=%1)").arg(e_code));
        emit signalFinished(e_code);
    }
    else
    {
        qDebug("req was prepared OK!");
        printHeaders();
        emit signalMsg("request started ....");
        toDebugReqMetadata();
        addOrdersLog(req_data);
        m_reqObj->start(hrmPost);
    }
}
void APIReqPage::addOrdersLog(const PlaceOrderData *req_data)
{
    if (!req_data) return;

    QString err;
    QString fname(APIOrdersPage::logFile());
    if (!LFile::fileExists(fname))
    {
        err = LFile::writeFile(fname, "SENT ORDERS LOG: \n");
        if (!err.isEmpty())
        {
            emit signalError(QString("APIReqPage::addOrdersLog() - %1").arg(err));
            return;
        }
    }

    QString ticker;
    emit signalGetTickerByFigi(req_data->uid, ticker);

    QDateTime dt(QDateTime::currentDateTimeUtc());
    QString s = QString("%1 / %2").arg(dt.date().toString(InstrumentBase::userDateMask())).arg(dt.time().toString(InstrumentBase::userTimeMask()));
    s = QString("%1 / %2 / %3").arg(s).arg(req_data->kind.trimmed().toUpper()).arg(req_data->lots);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(QString::number(req_data->price, 'f', 2)).arg(req_data->uid).arg(ticker);
    if (req_data->isStop()) s.append(" / IS_STOP");
    if (req_data->nominal > 0) s.append(" / IS_BOND");

    err = LFile::appendFile(fname, QString("%1 \n").arg(s));
    if (!err.isEmpty())
        emit signalError(QString("APIReqPage::addOrdersLog() - %1").arg(err));

}
void APIReqPage::slotCycleWorkerNextReq()
{
    if (requesterBuzy())
    {
        emit signalError("Requester object is buzy.");
        emit signalFinished(hreBuzy);
        return;
    }

    QString src;
    foreach(const QString &v, api_commonSettings.services)
        if (v.contains(m_cycleWroker->apiMetod())) {src = v; break;}

    if (src.isEmpty())
    {
        emit signalError(QString("cycle breaked, invalid api_metod: %1").arg(m_cycleWroker->apiMetod()));
        m_cycleWroker->breakCycle();
        emit signalFinished(hreWrongReqParams);
        return;
    }

    emit signalMsg("next cycle request");
    QStringList next_data;
    m_cycleWroker->getNextCycleData(next_data);
    m_reqPreparer->setCycleData(next_data);
    m_reqPreparer->prepare(src, NULL);
    printHeaders();
    m_reqObj->start(hrmPost);

}
void APIReqPage::slotCycleWorkerFinished()
{
    emit signalMsg(QString("-------------------CYCLE WORKER FINISHED---------------------"));
    emit signalFinished(hreUnknown);
    m_sourceBox->setEnabled(true);
}
void APIReqPage::prepareCycleData()
{
    QStringList data;
    emit signalGetBondCycleData(data);
    emit signalGetStockCycleData(data);
    m_cycleWroker->prepareCycleData(data);
}
void APIReqPage::autoStartReq(QString src)
{
    int n = m_sourceBox->listWidget()->count();
    if (n <= 0) {emit signalError("Sources list is empty"); return;}

    bool find_ok = false;
    for (int i=0; i<n; i++)
    {
        QString s = m_sourceBox->listWidget()->item(i)->text();
        if (s.contains(src))
        {
            m_sourceBox->listWidget()->setCurrentRow(i);
            //m_sourceBox->listWidget()->item(i)->setSelected(true);
            find_ok = true;
            break;
        }
    }

    if (find_ok) trySendReq();
    else emit signalError("SRC not found among config sources list");
}
void APIReqPage::setServerAPI(int p_type, const QString &serv_url)
{
    m_reqObj->setHttpProtocolType(p_type);
    m_reqObj->setApiServer(serv_url);
}
bool APIReqPage::requesterBuzy() const
{
    if (m_reqObj) return m_reqObj->isBuzy();
    return false;
}
void APIReqPage::checkReply()
{
    const LHttpApiReplyData& r = m_reqObj->lastReply();
    printHeaders("resp");

    if (r.isOk())
    {
        m_replyBox->loadJSON(r.data, "JSON struct");
        m_replyBox->expandLevel();
        m_replyBox->resizeByContents();
        m_replyBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);

        if (m_cycleWroker->cycleModeOn()) m_cycleWroker->handleReplyData(r.data);
        else  handleReplyData();
    }
    else
    {
        qWarning()<<QString("*******************APIReqPage::checkReply() WARNING code=%1**************").arg(r.result_code);
        foreach (const QString &v, m_reqObj->lastReply().headers)
        {
            QString s = v.trimmed().toLower();
            if (s.contains("not found")) emit signalError(v);
            if (s.contains("forbidden")) emit signalError(v);
        }

/*
        QJsonObject::const_iterator it =  r.data.constBegin();
        while (it != r.data.constEnd())
        {
            if (it.value().isObject())
            {
                qDebug()<<QString("KEY[%1] => OBJ").arg(it.key());
                QJsonObject::const_iterator it2 =  it.value().toObject().constBegin();
                while (it2 != it.value().toObject().constEnd())
                {
                    if (it2.value().isDouble()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (DOUBLE)").arg(it2.key()).arg(it2.value().toDouble());
                    else if (it2.value().isBool()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (BOOL)").arg(it2.key()).arg(it2.value().toBool());
                    else qDebug()<<QString("   KEY[%1] => VALUE[%2]").arg(it2.key()).arg(it2.value().toString());
                    it2++;
                }
            }
            else
            {
                if (it.value().isDouble()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (DOUBLE)").arg(it.key()).arg(it.value().toDouble());
                else if (it.value().isBool()) qDebug()<<QString("   KEY[%1] => VALUE[%2] (BOOL)").arg(it.key()).arg(it.value().toBool());
                else qDebug()<<QString("KEY[%1] => VALUE[%2]").arg(it.key()).arg(it.value().toString());
            }
            it++;
        }
        qDebug()<<QString();
        */

    }

    if (isNeedUpdateInfo()) needUpdateInfo();
}
bool APIReqPage::replyOk() const
{
    return m_reqObj->lastReply().isOk();
}
void APIReqPage::setExpandLevel(int a)
{
    m_replyBox->setExpandLevel(a);
}
void APIReqPage::handleReplyData()
{
    QString src = m_reqObj->fullUrl().toLower();
    qDebug()<<QString("APIReqPage::handleReplyData() SRC [%1]").arg(src);

    if (src.right(5) == "bonds")
    {
        saveBondsFile();
    }
    else if (src.right(6) == "shares")
    {
        saveStocksFile();
    }
    else if (src.right(11) == "getaccounts")
    {
        parseUserID();
    }
    else if (src.right(12) == "getpositions")
    {
        emit signalLoadPositions(m_reqObj->lastReply().data);
    }
    else if (src.right(12) == "getportfolio")
    {
        emit signalLoadPortfolio(m_reqObj->lastReply().data);
    }
    else if (src.right(13) == "getoperations")
    {
        emit signalLoadEvents(m_reqObj->lastReply().data);
    }
    else if (src.right(9) == "getorders")
    {
        emit signalLoadOrders(m_reqObj->lastReply().data);
    }
    else if (src.right(13) == "getstoporders")
    {
        qDebug("reply stop_orders");
        emit signalLoadStopOrders(m_reqObj->lastReply().data);
    }
    else if (src.right(10) == "lastprices")
    {
        const LHttpApiReplyData& r = m_reqObj->lastReply();
        m_cycleWroker->parseLastPrices(r.data);
    }
}
void APIReqPage::parseUserID()
{
    const LHttpApiReplyData& r = m_reqObj->lastReply();
    const QJsonArray &j_arr = r.data.constBegin().value().toArray();
    if (j_arr.isEmpty()) return;

    if (j_arr.first().isObject())
    {
        const QJsonObject &j_obj = j_arr.first().toObject();
        QString s_id = j_obj.value("id").toString();
        bool ok;
        api_commonSettings.user_id = s_id.toInt(&ok);
        if (!ok) emit signalError(QString("invalid user ID: %1").arg(s_id));
        else emit signalMsg(QString("was got user ID: %1").arg(api_commonSettings.user_id));
    }
}
void APIReqPage::saveBondsFile()
{
    qDebug("APIReqPage::saveBondsFile()");
    //read current data
    QString fname(APIBondsPage::dataFile());
    QString cur_data;
    if (LFile::fileExists(fname))
    {
        QString err = LFile::readFileStr(fname, cur_data);
        if (!err.isEmpty())
        {
            cur_data.clear();
            emit signalError(QString("APIReqPage::saveBondsFile() - %1").arg(err));
        }
    }
    if (cur_data.isEmpty()) LFile::writeFile(fname, "BOND INFO: \n");

    const LHttpApiReplyData& r = m_reqObj->lastReply();
    const QJsonArray &j_arr = r.data.constBegin().value().toArray();
    if (j_arr.isEmpty())
    {
        emit signalError(QString("APIReqPage::saveBondsFile() - QJsonArray is empty"));
        return;
    }

    //find last rec number
    QStringList cur_list(LString::trimSplitList(cur_data));
    QString last_s = cur_list.last().trimmed();
    quint16 rec_number = 9999;
    int pos = last_s.left(6).indexOf(".");
    if (pos > 0) rec_number = last_s.left(pos).toUInt() + 1;
    qDebug()<<QString("cur data list size %1, next rec number %2").arg(cur_list.count()).arg(rec_number);

    //write new records to bonds-file
    int n = j_arr.count();
    int n_new = 0;
    for (int i=0; i<n; i++)
    {
        BondDesc bond(j_arr.at(i));
        if (bond.invalid()) {qWarning() << QString("WARNING: INVALID BOND  [%1]").arg(bond.toStr()); continue;}
        if (cur_data.contains(bond.uid)) continue; //такая запись уже присутствует в файле

        //qDebug()<<QString("add new bond [%1]").arg(bond.toStr());
        bool has_isin = false;
        foreach (const QString &fline, cur_list) //проверка на предмет совпадения isin новой записи и тех что уже есть в файле
        {
            if (fline.contains(bond.isin))
            {
                emit signalMsg(QString("ISIN: [%1] already exists,  pos[%2]").arg(bond.isin).arg(fline.left(4)));
                emit signalMsg(QString("FLINE: [%1]").arg(fline));
                emit signalMsg(QString("NEW RECORD: [%1]").arg(bond.toStr()));
                emit signalMsg(QString());

                has_isin = true;
                break;
                qDebug()<<QString("coincidence isin: ").toUpper()<<fline<<"\n";
            }
        }

        if (!has_isin)
        {
            LFile::appendFile(fname, QString("%1.  %2 \n").arg(rec_number).arg(bond.toStr()));
            rec_number++;
            n_new++;
        }
    }
    emit signalMsg(QString("saved file of bonds list (size %1/%2)").arg(n).arg(n_new));
}
void APIReqPage::saveStocksFile()
{
    //qDebug("APIReqPage::saveStocksFile()");
    LFile::writeFile(APIStocksPage::dataFile(), "STOCK INFO: \n");
    const LHttpApiReplyData& r = m_reqObj->lastReply();
    const QJsonArray &j_arr = r.data.constBegin().value().toArray();
    if (j_arr.isEmpty()) return;

    int n = j_arr.count();
    for (int i=0; i<n; i++)
    {
        StockDesc stock(j_arr.at(i));
        LFile::appendFile(APIStocksPage::dataFile(), QString("%1.  %2 \n").arg(i+1).arg(stock.toStr()));
    }
    emit signalMsg(QString("saved file of stocks list (size %1)").arg(n));
}
void APIReqPage::printHeaders(QString s)
{
    if (!m_printHeaders) return;

    if (s == "resp") //reply headers to protocol
    {
        emit signalMsg("REPLY HEADERS:");
        foreach (const QString &v, m_reqObj->lastReply().headers)
            emit signalMsg(v);
        emit signalMsg(QString());
    }
    else if (s == "req") //request headers to protocol
    {
        QStringList req_headers;
        m_reqObj->getReqHeaders(req_headers);
        emit signalMsg("REQUEST HEADERS:");
        foreach (const QString &v, req_headers)
            emit signalMsg(v);
        emit signalMsg("-------------------------------------------");
    }
}




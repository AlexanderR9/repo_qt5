#include "apireqpage.h"
#include "lfile.h"
#include "lhttpapirequester.h"
#include "apipages.h"
#include "apicommonsettings.h"
#include "lhttp_types.h"
#include "reqpreparer.h"

#include <QListWidget>
#include <QTreeWidget>
#include <QSplitter>



//APIReqPage
APIReqPage::APIReqPage(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_sourceBox(NULL),
      m_replyBox(NULL),      
      m_reqObj(NULL),
      m_reqPreparer(NULL),
      m_printHeaders(true)
{
    setObjectName("api_req_page");

    initWidgets();
    initSources();
    initReqObject();
    initReqPreparer();
}
APIReqPage::~APIReqPage()
{
    if (m_reqPreparer) {delete m_reqPreparer; m_reqPreparer = NULL;}
    if (m_reqObj) {delete m_reqObj; m_reqObj = NULL;}
}
void APIReqPage::initReqObject()
{
    m_reqObj = new LHttpApiRequester(this);
    connect(m_reqObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_reqObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_reqObj, SIGNAL(signalFinished(int)), this, SIGNAL(signalFinished(int)));
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
    foreach(const QString &v, api_commonSettings.services)
    {
        m_sourceBox->listWidget()->addItem(v);
    }
}
void APIReqPage::trySendReq()
{
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

    //prepareReq(row);
    QString src = m_sourceBox->listWidget()->item(row)->text();
    m_reqPreparer->prepare(src);
    if (m_reqPreparer->invalidReq())
    {
        int e_code = hreUnknown;
        if (m_reqObj->metadata().value("err").isString()) e_code = m_reqObj->metadata().value("err").toString().toInt();
        else emit signalError(QString("invalid metadata err_value type"));
        emit signalError(QString("request did't start (code=%1)").arg(e_code));
        emit signalFinished(e_code);
    }
    else
    {
        printHeaders();
        m_reqObj->start(hrmPost);
    }
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
    else emit signalError("SRC not fond among config sources list");
}
void APIReqPage::setServerAPI(int p_type, const QString &serv_url)
{
    m_reqObj->setHttpProtocolType(p_type);
    m_reqObj->setApiServer(serv_url);
}


/*
void APIReqPage::prepareReq(int source_row)
{


    m_reqObj->clearMetaData();

    QString token, baseURI;
    emit signalGetReqParams(token, baseURI);

    //QString uid;
    QString src = m_sourceBox->listWidget()->item(source_row)->text();
    m_reqObj->addReqHeader(QString("Authorization"), QString("Bearer %1").arg(api_commonSettings.token));
    m_reqObj->addReqHeader(QString("accept"), QString("application/json"));
    m_reqObj->addReqHeader(QString("Content-Type"), QString("application/json"));
    m_reqObj->setUri(QString("%1.%2").arg(baseURI).arg(src));

    if (src.contains("OperationsService")) prepareReqOperations();
    else if (src.contains("BondBy")) prepareReqBondBy();
    else if (src.contains("ShareBy")) prepareReqShareBy();
    else if (src.contains("GetBondCoupons")) prepareReqCoupons();
    else if (src.contains("GetDividends")) prepareReqDivs();
    else if (src.contains("GetLastPrices")) prepareReqLastPrices();
    else if (src.contains("MarketDataService")) prepareReqMarket(src);

}

void APIReqPage::prepareReqDivs()
{

}
void APIReqPage::prepareReqCoupons()
{

}
void APIReqPage::prepareReqLastPrices()
{
    QStringList uid_list;
    emit signalGetSelectedBondUIDList(uid_list);
    if (uid_list.isEmpty())
    {
        emit signalGetSelectedStockUIDList(uid_list);
        if (uid_list.isEmpty())
        {
            emit signalError("you must select some bond or share in the table");
            m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
            return;
        }
    }

    emit signalMsg(QString("SELECTED UIDs: %1").arg(uid_list.count()));
    QJsonArray j_arr = QJsonArray::fromStringList(uid_list);
    m_reqObj->addMetaData_arr("instrumentId", j_arr);
}
void APIReqPage::prepareReqMarket(const QString &src)
{
    QString uid;
    emit signalGetSelectedBondUID(uid);
    if (uid.isEmpty())
    {
        emit signalGetSelectedStockUID(uid);
        if (uid.isEmpty())
        {
            emit signalError("you must select some bond or share in the table");
            m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
            return;
        }
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(uid));
    m_reqObj->addMetaData("instrumentId", uid);

    if (src.contains("Book"))
    {
        quint16 dp = 0;
        emit signalGetPricesDepth(dp);
        m_reqObj->addMetaData("depth", QString::number(dp));
    }
    else if (src.contains("Candles"))
    {
        QString candle_size;
        emit signalGetCandleSize(candle_size);
        m_reqObj->addMetaData("interval", api_commonSettings.candle_sizes.value(candle_size));
        m_reqObj->addMetaData("from", api_commonSettings.beginPoint(api_commonSettings.i_history.prices));
        m_reqObj->addMetaData("to", api_commonSettings.endPoint(api_commonSettings.i_history.prices));
        emit signalMsg(QString("HISTORY_PERIOD: %1 - %2").arg(api_commonSettings.i_history.prices.begin_date).arg(api_commonSettings.i_history.prices.end_date));
    }
}
void APIReqPage::prepareReqShareBy()
{
    QString uid;
    emit signalGetSelectedStockUID(uid);
    if (uid.isEmpty())
    {
        emit signalError("you must select some stock in the table");
        m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
        return;
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(uid));
    m_reqObj->addMetaData("idType", "INSTRUMENT_ID_TYPE_UID");
    m_reqObj->addMetaData("id", uid);
}
void APIReqPage::prepareReqBondBy()
{
    QString uid;
    emit signalGetSelectedBondUID(uid);
    if (uid.isEmpty())
    {
        emit signalError("you must select some bond in the table");
        m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
        return;
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(uid));
    m_reqObj->addMetaData("idType", "INSTRUMENT_ID_TYPE_UID");
    m_reqObj->addMetaData("id", uid);
}
void APIReqPage::prepareReqOperations()
{
    m_reqObj->addMetaData("currency", "RUB");
    m_reqObj->addMetaData("accountId", QString::number(api_commonSettings.user_id));
}
*/



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
        handleReplyData();
    }
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
        qDebug("emit getpositions");
        emit signalLoadPositions(m_reqObj->lastReply().data);
    }
    else if (src.right(12) == "getportfolio")
    {
        qDebug("emit signalLoadPortfolio");
        emit signalLoadPortfolio(m_reqObj->lastReply().data);
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
    //qDebug("APIReqPage::saveBondsFile()");
    LFile::writeFile(APIBondsPage::dataFile(), "BOND INFO: \n");
    const LHttpApiReplyData& r = m_reqObj->lastReply();
    const QJsonArray &j_arr = r.data.constBegin().value().toArray();
    if (j_arr.isEmpty()) return;

    int n = j_arr.count();
    for (int i=0; i<n; i++)
    {
        BondDesc bond(j_arr.at(i));
        LFile::appendFile(APIBondsPage::dataFile(), QString("%1.  %2 \n").arg(i+1).arg(bond.toStr()));
    }
    emit signalMsg(QString("saved file of bonds list (size %1)").arg(n));
}
void APIReqPage::saveStocksFile()
{
    qDebug("APIReqPage::saveStocksFile()");
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




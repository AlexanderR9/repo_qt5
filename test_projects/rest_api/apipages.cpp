#include "apipages.h"
#include "lhttpapirequester.h"
#include "lhttp_types.h"
#include "lfile.h"
#include "ltable.h"
#include "instrument.h"
#include "apicommonsettings.h"

#include <QSplitter>
#include <QListWidget>
#include <QTreeWidget>
#include <QColor>
#include <QLabel>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QGridLayout>
#include <QSpacerItem>
#include <QTableWidget>
#include <QDir>

#define COUNTRY_COL         2
#define RISK_COL            6
#define COUP_YEAR_COL       5

//APIReqPage
APIReqPage::APIReqPage(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_sourceBox(NULL),
      m_replyBox(NULL),
      m_reqObj(NULL)
{
    setObjectName("api_req_page");
    initWidgets();
    initSources();

    m_reqObj = new LHttpApiRequester(this);
    connect(m_reqObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_reqObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_reqObj, SIGNAL(signalFinished(int)), this, SIGNAL(signalFinished(int)));
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

    prepareReq(row);
    emit signalMsg(QString("URL:   %1 \n").arg(m_reqObj->fullUrl()));

    //request headers to protocol
    QStringList req_headers;
    m_reqObj->getReqHeaders(req_headers);
    emit signalMsg("REQUEST HEADERS:");
    foreach (const QString &v, req_headers)
        emit signalMsg(v);

    m_reqObj->start(hrmPost);
}
void APIReqPage::setServerAPI(int p_type, const QString &serv_url)
{
    m_reqObj->setHttpProtocolType(p_type);
    m_reqObj->setApiServer(serv_url);
}
void APIReqPage::prepareReq(int source_row)
{
    m_reqObj->clearMetaData();

    QString token, baseURI;
    emit signalGetReqParams(token, baseURI);

    QString uid;
    QString src = m_sourceBox->listWidget()->item(source_row)->text();
    m_reqObj->addReqHeader(QString("Authorization"), QString("Bearer %1").arg(api_commonSettings.token));
    m_reqObj->addReqHeader(QString("accept"), QString("application/json"));
    m_reqObj->addReqHeader(QString("Content-Type"), QString("application/json"));
    m_reqObj->setUri(QString("%1.%2").arg(baseURI).arg(src));

    if (src.contains("OperationsService"))
    {
        m_reqObj->addMetaData("currency", "RUB");
    }
    else if (src.contains("BondBy"))
    {
        emit signalGetSelectedBondUID(uid);
        if (uid.isEmpty()) emit signalError("you must select some bond in the table");
        else emit signalMsg(QString("SELECTED UID: %1").arg(uid));

        m_reqObj->addMetaData("idType", "INSTRUMENT_ID_TYPE_UID");
        m_reqObj->addMetaData("id", uid);
    }
    else if (src.contains("MarketDataService"))
    {
        emit signalGetSelectedBondUID(uid);
        if (uid.isEmpty()) emit signalError("you must select some bond in the table");
        else emit signalMsg(QString("SELECTED UID: %1").arg(uid));
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
            m_reqObj->addMetaData("from", "2023-10-16T07:00:00Z");
            m_reqObj->addMetaData("to", "2023-10-19T18:00:00Z");
        }
    }
}
void APIReqPage::checkReply()
{
    const LHttpApiReplyData& r = m_reqObj->lastReply();

    //reply headers to protocol
    emit signalMsg("REPLY HEADERS:");
    foreach (const QString &v, r.headers)
        emit signalMsg(v);

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
    if (src.right(5) == "bonds")
    {
        saveBondsFile();
    }
}
void APIReqPage::saveBondsFile()
{
    //qDebug("APIReqPage::saveBondsFile()");
    LFile::writeFile(APIBondsPage::dataFile(), "BOND INFO: \n");
    const LHttpApiReplyData& r = m_reqObj->lastReply();
    const QJsonArray &j_arr = r.data.constBegin().value().toArray();
    if (!j_arr.isEmpty())
    {
        int n = j_arr.count();
        for (int i=0; i<n; i++)
        {
            BondDesc bond(j_arr.at(i));
            LFile::appendFile(APIBondsPage::dataFile(), QString("%1.  %2 \n").arg(i+1).arg(bond.toStr()));
        }
    }
}




//APIBondsPage
APIBondsPage::APIBondsPage(QWidget *parent)
    :APITablePageBase(parent),
    m_countryFilterControl(NULL),
    m_riskFilterControl(NULL)

{
    setObjectName("api_bonds_page");

    QStringList headers;
    headers << "Company" << "Tiker" << "Country" << "Currency" << "Finish date" << "Coupons" << "Risk";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Bonds list");

    initFilterBox();
}
void APIBondsPage::resetPage()
{
    APITablePageBase::resetPage();
    m_countryFilterControl->setCurrentIndex(0);
    m_riskFilterControl->setCurrentIndex(0);
}
void APIBondsPage::initFilterBox()
{
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QGridLayout *g_lay = new QGridLayout(m_filterBox);
    g_lay->addWidget(new QLabel("By country"), 0, 0);
    g_lay->addWidget(new QLabel("By risk"), 1, 0);

    m_countryFilterControl = new QComboBox(this);
    m_countryFilterControl->setObjectName("country_control");
    m_countryFilterControl->addItem("All");
    m_countryFilterControl->addItem("Only Rus");
    m_countryFilterControl->addItem("Exept Rus");
    g_lay->addWidget(m_countryFilterControl, 0, 1);

    m_riskFilterControl = new QComboBox(this);
    m_riskFilterControl->setObjectName("risk_control");
    m_riskFilterControl->addItem("All");
    m_riskFilterControl->addItem("High");
    m_riskFilterControl->addItem("Low");
    m_riskFilterControl->addItem("Medium");
    g_lay->addWidget(m_riskFilterControl, 1, 1);

    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, 2, 0, 2, 2);

    connect(m_countryFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
    connect(m_riskFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
}
void APIBondsPage::slotFilter(QString f_value)
{
    qDebug()<<QString("slotFilter,  f_value=%1").arg(f_value);
    if (!sender()) return;
    f_value = f_value.trimmed().toLower();

    if (sender()->objectName().contains("country"))
        countryFilter(f_value);
    else if (sender()->objectName().contains("risk"))
        riskFilter(f_value);

    m_tableBox->searchExec();
}
void APIBondsPage::riskFilter(const QString &f_value)
{
    if (f_value.contains("all")) return;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    for (int i=n-1; i>=0; i--)
    {
        QString tv = t->item(i, RISK_COL)->text().trimmed().toLower();
        if (f_value.contains("high") && !tv.contains("high")) t->removeRow(i);
        else if (f_value.contains("medium") && !tv.contains("moderate")) t->removeRow(i);
        else if (f_value.contains("low") && !tv.contains("low")) t->removeRow(i);
    }
}
void APIBondsPage::countryFilter(const QString &f_value)
{
    if (f_value.contains("all")) return;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    QString f_key("Российская Федерация");
    for (int i=n-1; i>=0; i--)
    {
        QString tv = t->item(i, COUNTRY_COL)->text().trimmed();
        if (f_value.contains("only") && tv != f_key) t->removeRow(i);
        else if (f_value.contains("exept") && tv == f_key) t->removeRow(i);
    }
}
void APIBondsPage::loadData()
{
    m_data.clear();

    emit signalMsg(QString("OPEN FILE: %1").arg(APIBondsPage::dataFile()));
    QStringList list;
    QString err = LFile::readFileSL(APIBondsPage::dataFile(), list);
    if (!err.isEmpty()) {emit signalError(err); return;}

    int n_invalid = 0;
    foreach (const QString &v, list)
    {
        if (v.trimmed().isEmpty()) continue;
        if (!v.contains(".") || !v.contains("/")) continue;
        BondDesc rec;
        rec.fromFileLine(v);
        if (!rec.invalid()) m_data.append(rec);
        else {qWarning() << QString("INVALID BOND LINE: [%1]").arg(v); n_invalid++;}
    }

    emit signalMsg(QString("found invalid lines: %1/%2").arg(n_invalid).arg(list.count()));
    sortByDate();
    reloadTableByData();
}
void APIBondsPage::reloadTableByData()
{
    resetPage();
    if (m_data.isEmpty())
    {
        emit signalMsg(QString("Bonds data is empty!"));
        return;
    }

    foreach (const BondDesc &rec, m_data)
    {
        LTable::addTableRow(m_tableBox->table(), rec.toTableRowData());
        int l_row = m_tableBox->table()->rowCount() - 1;
        m_tableBox->table()->item(l_row, 0)->setData(Qt::UserRole, rec.number);
        if (!rec.api_trade) LTable::setTableRowColor(m_tableBox->table(), l_row, Qt::lightGray);
        if (rec.coupons_year > 4) m_tableBox->table()->item(l_row, COUP_YEAR_COL)->setTextColor(Qt::blue);
        if (rec.risk.toLower().contains("high")) m_tableBox->table()->item(l_row, RISK_COL)->setTextColor(Qt::red);
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#E6E6FA", "#800000");
    emit signalMsg(QString("loaded validity records: %1").arg(m_data.count()));
    m_tableBox->searchExec();
}
void APIBondsPage::slotSetSelectedBondUID(QString &uid)
{
    uid.clear();
    QList<int> sel_rows = LTable::selectedRows(m_tableBox->table());
    if (sel_rows.isEmpty()) return;

    bool ok;
    quint16 rec_number = m_tableBox->table()->item(sel_rows.first(), 0)->data(Qt::UserRole).toUInt(&ok);
    if (!ok) return;

    foreach (const BondDesc &rec, m_data)
        if (rec.number == rec_number) {uid = rec.uid; break;}
}
void APIBondsPage::sortByDate()
{
    if (m_data.count() < 3) return;


    int n = m_data.count();
    while (1 == 1)
    {
        bool has_replace = false;
        for (int i=0; i<n-1; i++)
        {
            const QDate &d = m_data.at(i).finish_date;
            const QDate &d_next = m_data.at(i+1).finish_date;
            if (!d.isValid() && d_next.isValid())
            {
                BondDesc rec = m_data.takeAt(i);
                m_data.insert(i+1, rec);
                has_replace = true;
            }
            else if (d.isValid() && d_next.isValid())
            {
                if (d_next < d)
                {
                    BondDesc rec = m_data.takeAt(i);
                    m_data.insert(i+1, rec);
                    has_replace = true;
                }
            }
        }
        if (!has_replace) break;
    }
}
QString APIBondsPage::dataFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(QString("bonds.txt"));
}


//APIStoksPage
APIStoksPage::APIStoksPage(QWidget *parent)
    :APITablePageBase(parent)
{
    setObjectName("api_stocks_page");

    QStringList headers;
    headers << "Company" << "Tiker" << "Country" << "Currency";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Stocks list");

}

//APITablePageBase
APITablePageBase::APITablePageBase(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_tableBox(NULL),
      m_filterBox(NULL)
{
    setObjectName("api_base_page");
    initWidgets();
}
void APITablePageBase::initWidgets()
{
    m_tableBox = new LSearchTableWidgetBox(this);
    m_tableBox->setTitle("Data");
    m_filterBox = new QGroupBox(this);
    m_filterBox->setTitle("Filter parameters");


    QStringList headers;
    m_tableBox->resizeByContents();
    m_tableBox->sortingOn();
    h_splitter->addWidget(m_tableBox);
    h_splitter->addWidget(m_filterBox);
}
void APITablePageBase::resetPage()
{
    LTable::removeAllRowsTable(m_tableBox->table());
    m_tableBox->searchReset();
}

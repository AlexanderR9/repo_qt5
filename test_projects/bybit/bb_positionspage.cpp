#include "bb_positionspage.h"
#include "bb_apistruct.h"
#include "lhttp_types.h"
#include "ltable.h"
#include "apiconfig.h"
#include "bb_apistruct.h"

#include <QSplitter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>

#define API_POS_URI                 QString("v5/position/list")
#define API_ORDERS_URI              QString("v5/order/realtime")
#define API_SPOT_ASSET_URI          QString("v5/account/wallet-balance")


#define TRADE_TYPE_COL              3
#define FREEZED_SUM_COL             5
#define MAX_ORDERS_PAGE             50
#define TICKER_COL                  1
#define DATE_COL                    0
#define OVER_DAYS                   90
#define NOT_ADJACENT_POS            QString("#FFBB66")


//BB_PositionsPage
BB_PositionsPage::BB_PositionsPage(QWidget *parent)
    :BB_BasePage(parent, 20, rtPositions),
    m_table(NULL),
    m_orderTable(NULL)
{
    setObjectName("positions_page");
    init();

    m_reqData->params.insert("category", "linear");
    m_reqData->params.insert("settleCoin", "USDT");

}
void BB_PositionsPage::clearTables()
{
    m_table->removeAllRows();
    m_orderTable->removeAllRows();
    m_reqData->params.remove("cursor");
}
void BB_PositionsPage::init()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setObjectName("pos_table");
    m_table->setTitle("Current positions");
    initTable(m_table);

    //init sorting data
    m_table->addSortingData(1, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(2, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(3, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(4, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(5, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(6, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(8, LSearchTableWidgetBox::sdtNumeric);
    m_table->sortingOn();

    m_orderTable = new LSearchTableWidgetBox(this);
    m_orderTable->setObjectName("order_table");
    m_orderTable->setTitle("Placed orders");
    initTable(m_orderTable);

    m_orderTable->addSortingData(1, LSearchTableWidgetBox::sdtString);
    m_orderTable->addSortingData(2, LSearchTableWidgetBox::sdtNumeric);
    m_orderTable->addSortingData(3, LSearchTableWidgetBox::sdtString);
    m_orderTable->addSortingData(5, LSearchTableWidgetBox::sdtNumeric);
    m_orderTable->addSortingData(6, LSearchTableWidgetBox::sdtString);
    m_orderTable->sortingOn();

}
void BB_PositionsPage::initTable(LSearchTableWidgetBox *t)
{
    if (!t) return;
    t->vHeaderHide();
    t->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    t->setHeaderLabels(tableHeaders(t->objectName()));
    t->resizeByContents();
    h_splitter->addWidget(t);
    t->searchExec();
}
QStringList BB_PositionsPage::tableHeaders(QString type) const
{
    //qDebug()<<QString("t_type=%1").arg(type);
    QStringList list;
    list << "Date" << "Ticker" << "Volume" << "Action" << "Open price" << "Freezed sum";
    if (type.contains("pos")) list << "Leverage" << "Current price" << "Result";
    else if (type.contains("order")) list << "Order type";// << "Status";
    return list;
}
void BB_PositionsPage::updateDataPage(bool forcibly)
{
    if (!updateTimeOver(forcibly)) return;

    clearTables();
    m_reqData->uri = API_POS_URI;
    m_reqData->req_type = rtPositions;
    m_reqData->params.insert("cursor", QString::number(0));

    sendRequest(api_config.req_limit_pos);
}
void BB_PositionsPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != rtPositions && req_type != rtOrders) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_PositionsPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_PositionsPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {checkAdjacent(req_type); emit signalError("BB_PositionsPage: j_arr QJsonArray is empty");}

    QString next_cursor;
    if (req_type == rtPositions)
    {
        fillPosTable(j_arr);

        //next request (orders: next page)
        next_cursor = jv.toObject().value("nextPageCursor").toString().trimmed();
        if (!next_cursor.isEmpty())
        {
            m_reqData->params.insert("cursor", next_cursor);
            sendRequest(api_config.req_limit_pos);
        }
        else //request for (orders: first page)
        {            
            m_reqData->uri = API_ORDERS_URI;
            m_reqData->req_type = rtOrders;
            m_reqData->params.insert("cursor", QString::number(0));
            sendRequest(MAX_ORDERS_PAGE);
        }
    }
    else if (req_type == rtOrders)
    {
        fillOrdersTable(j_arr);

        //next request (orders: next page)
        next_cursor = jv.toObject().value("nextPageCursor").toString().trimmed();
        if (!next_cursor.isEmpty())
        {
            m_reqData->params.insert("cursor", next_cursor);
            sendRequest(MAX_ORDERS_PAGE);
        }
    }
}
QString BB_PositionsPage::getTimePoint(const QJsonObject &j_el, bool &days_over) const
{
    QString ts = j_el.value("createdTime").toString().trimmed();
    qint64 a = ts.toLong();
    ts = APIConfig::fromTimeStamp(a, QString("dd.MM.yyyy"));
    QDateTime dt;
    dt.setMSecsSinceEpoch(a);
    int d_to = dt.date().daysTo(QDate::currentDate());
    days_over = (d_to > OVER_DAYS);
    return QString("%1 (%2 days)").arg(ts).arg(d_to);
}
void BB_PositionsPage::fillPosTable(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) return;

    QTableWidget *t = m_table->table();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_PositionsPage::fillTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QString tt = j_el.value("side").toString().toUpper();

        QStringList row_data;
        quint8 prs = 2;
        float open_price = j_el.value("avgPrice").toString().toFloat();
        float cur_price = j_el.value("markPrice").toString().toFloat();
        float lots = j_el.value("size").toString().toFloat();
        float freezed = open_price*lots;
        float result = (cur_price-open_price)*lots;
        if (tt == "SELL") result *= (-1);
        if (open_price < 1) prs = 3;

        bool days_over = false;
        row_data << getTimePoint(j_el, days_over);
        row_data << j_el.value("symbol").toString() << j_el.value("size").toString() << tt;
        row_data << QString::number(open_price, 'f', prs) << QString::number(freezed, 'f', prs) << j_el.value("leverage").toString();
        row_data << QString::number(cur_price, 'f', prs) << QString::number(result, 'f', 2);

        LTable::addTableRow(t, row_data);

        if (tt == "SELL") t->item(i, TRADE_TYPE_COL)->setTextColor("#AA0000");
        if (result < 0) t->item(i, t->columnCount()-1)->setTextColor("#AA0000");
        if (days_over) t->item(i, DATE_COL)->setTextColor("#880000");
    }
    m_table->resizeByContents();
    m_table->searchExec();
}
void BB_PositionsPage::fillOrdersTable(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) return;

    QTableWidget *t = m_orderTable->table();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_PositionsPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QString o_type = j_el.value("stopOrderType").toString().toUpper().trimmed();
        if (o_type.isEmpty()) o_type = j_el.value("orderType").toString().toUpper().trimmed();

        QString s_freezed("-");
        if (o_type == "LIMIT")
        {
            float lots = j_el.value("qty").toString().toFloat();
            float freezed = j_el.value("price").toString().toFloat()*lots;
            s_freezed = QString::number(freezed, 'f', (freezed < 0.1) ? 3 : 2);
        }

        QString s_price = QString("%1 / %2").arg(j_el.value("price").toString()).arg(j_el.value("avgPrice").toString());

        QStringList row_data;
        bool days_over = false;
        row_data << getTimePoint(j_el, days_over);
        row_data << j_el.value("symbol").toString() << j_el.value("qty").toString() << j_el.value("side").toString().toUpper();
        row_data << j_el.value("price").toString() << s_freezed;
        row_data << o_type;// << j_el.value("orderStatus").toString();
        LTable::addTableRow(t, row_data);

        if (days_over) t->item(i, DATE_COL)->setTextColor("#880000");
    }

    m_orderTable->resizeByContents();
    m_orderTable->searchExec();
}
void BB_PositionsPage::slotGetPosState(BB_BagState &state)
{
    state.n_pos = m_table->table()->rowCount();
    state.n_order = m_orderTable->table()->rowCount();
    state.freezed_pos = 0;
    state.freezed_order = 0;
    state.pos_result = 0;

    //check pos table
    if (state.n_pos > 0)
    {
        QTableWidget *t = m_table->table();
        for (int i=0; i<state.n_pos; i++)
        {
            state.freezed_pos += t->item(i, FREEZED_SUM_COL)->text().toFloat();
            state.pos_result += t->item(i, t->columnCount()-1)->text().toFloat();
        }
    }

    //check order table
    if (state.n_order > 0)
    {
        QTableWidget *t = m_orderTable->table();
        for (int i=0; i<state.n_order; i++)
        {
            QString s(t->item(i, FREEZED_SUM_COL)->text().trimmed());
            if (s != "-") state.freezed_order += s.toFloat();
        }
    }
}
void BB_PositionsPage::checkAdjacent(int req_type)
{
    if (req_type != rtOrders) return;

    int n = m_table->table()->rowCount();
    for (int i=0; i<n; i++)
    {
        QString ticker = m_table->table()->item(i, TICKER_COL)->text();
        bool has_ticker = false;
        int k = 0;
        for (k=0; k<m_orderTable->table()->rowCount(); k++)
            if (m_orderTable->table()->item(k, TICKER_COL)->text() == ticker) {has_ticker = true; break;}
        if (!has_ticker) LTable::setTableRowColor(m_table->table(), i, NOT_ADJACENT_POS);
    }

    n = m_orderTable->table()->rowCount();
    for (int i=0; i<n; i++)
    {
        QString ticker = m_orderTable->table()->item(i, TICKER_COL)->text();
        bool has_ticker = false;
        int k = 0;
        for (k=0; k<m_table->table()->rowCount(); k++)
            if (m_table->table()->item(k, TICKER_COL)->text() == ticker) {has_ticker = true; break;}
        if (!has_ticker) LTable::setTableRowColor(m_orderTable->table(), i, Qt::lightGray);
    }
}

//BB_SpotPositionsPage
BB_SpotPositionsPage::BB_SpotPositionsPage(QWidget *parent)
    :BB_PositionsPage(parent)
{
    setObjectName("spot_positions_page");
    reinitWidgets();

    m_userSign = rtSpotAssets;
    m_reqData->params.clear();
}
void BB_SpotPositionsPage::reinitWidgets()
{
    QStringList list;
    list << "Ticker" << "Volume" << "Available" << "USD size" << "Price (1ps)";

    m_table->setHeaderLabels(list);
    m_table->resizeByContents();

    m_table->setTitle("Wallet assets");
}
void BB_SpotPositionsPage::updateDataPage(bool forcibly)
{
    if (!updateTimeOver(forcibly)) return;

    clearTables();

    m_reqData->params.clear();
    m_reqData->uri = API_SPOT_ASSET_URI;
    m_reqData->req_type = rtSpotAssets;
    m_reqData->params.insert("accountType", "UNIFIED");

    sendRequest();
}
void BB_SpotPositionsPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != rtSpotAssets && req_type != rtSportOrders) return;
    qDebug("BB_SpotPositionsPage::slotJsonReply");

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_SpotPositionsPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_SpotPositionsPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_SpotPositionsPage: j_arr QJsonArray is empty");}

    if (req_type == rtSpotAssets)
    {
        fillPosTable(j_arr);
        getSpotOrders();
    }
    else if (req_type == rtSportOrders)
    {
        fillOrdersTable(j_arr);
    }
}
void BB_SpotPositionsPage::getSpotOrders()
{
    m_reqData->params.clear();
    m_reqData->uri = API_ORDERS_URI;
    m_reqData->req_type = rtSportOrders;
    m_reqData->params.insert("category", "spot");
    m_reqData->params.insert("settleCoin", "USDT");

    sendRequest(MAX_ORDERS_PAGE);
}
void BB_SpotPositionsPage::fillPosTable(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) {qWarning()<<QString("BB_SpotPositionsPage::fillTable WARNING j_arr is empty"); return;}
    const QJsonArray &j_arr_assets = j_arr.first().toObject().value("coin").toArray();
    if (j_arr_assets.isEmpty())  {emit signalError("BB_SpotPositionsPage: j_arr_assets QJsonArray is empty");}

    QTableWidget *t = m_table->table();
    for (int i=0; i<j_arr_assets.count(); i++)
    {
        QJsonObject j_el = j_arr_assets.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_SpotPositionsPage::fillTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QStringList row_data;
        float size = j_el.value("walletBalance").toString().toFloat();
        float avail = j_el.value("availableToWithdraw").toString().toFloat();
        float usd =  j_el.value("usdValue").toString().toFloat();

        row_data << j_el.value("coin").toString();
        row_data << QString::number(size, 'f', 2);
        row_data << QString::number(avail, 'f', 2);
        row_data << QString::number(usd, 'f', 1);
        row_data << QString::number(usd/size, 'f', 3);

        LTable::addTableRow(t, row_data);

        if (size != avail) t->item(i, 2)->setTextColor("#AA0000");
    }

    m_table->resizeByContents();
    m_table->searchExec();
}
void BB_SpotPositionsPage::fillOrdersTable(const QJsonArray &j_arr)
{
    BB_PositionsPage::fillOrdersTable(j_arr);
}


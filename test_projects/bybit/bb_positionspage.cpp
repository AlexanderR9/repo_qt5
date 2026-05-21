#include "bb_positionspage.h"
#include "bb_apistruct.h"
#include "lhttp_types.h"
#include "ltable.h"
#include "apiconfig.h"
#include "bb_apistruct.h"
#include "apitradedialog.h"
#include "lstring.h"


#include <QSplitter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>

#define API_POS_URI                 QString("v5/position/list")
#define API_ORDERS_URI              QString("v5/order/realtime")
#define API_SPOT_ASSET_URI          QString("v5/account/wallet-balance")
#define API_ORDER_CANCEL_URI        QString("v5/order/cancel")
#define API_ORDER_MODIFY_URI        QString("v5/order/amend")
#define API_ORDER_CREATE_URI        QString("v5/order/create")


#define TRADE_TYPE_COL              3
#define LOT_COL                     2
#define TICKER_COL                  1
#define DATE_COL                    0
#define FREEZED_SUM_COL             5
#define LIMIT_PRICE_COL             4
#define CUSTOM_ID_COL               6
#define OPTION_MARKET_PRICE_COL     5


#define MAX_ORDERS_PAGE             50
#define OVER_DAYS                   90
#define NOT_ADJACENT_POS            QString("#FFBB66")


//BB_PositionsPage
BB_PositionsPage::BB_PositionsPage(QWidget *parent)
    :BB_BasePage(parent, 20, rtPositions),
    m_table(NULL),
    m_orderTable(NULL),
    m_category("linear")
{
    setObjectName("positions_page");
    init();

    // init popup
    initPopupMenu();

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
    m_reqData->metod = hrmGet;

    m_reqData->uri = API_POS_URI;
    m_reqData->req_type = rtPositions;

    m_reqData->params.clear();
    m_reqData->params.insert("cursor", QString::number(0));
    m_reqData->params.insert("category", m_category);
    m_reqData->params.insert("settleCoin", "USDT");


    sendRequest(api_config.req_limit_pos);
}
void BB_PositionsPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != rtPositions && req_type != rtOrders) return;

    if (m_reqData->metod == hrmPost)
    {
        qDebug("WAS POST REQ");
        return;
    }


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
        //return;

        //next request (orders: next page)
        /*
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
        */

        // simple variant
        m_reqData->uri = API_ORDERS_URI;
        m_reqData->req_type = rtOrders;
        m_reqData->params.insert("cursor", QString::number(0));
        sendRequest(MAX_ORDERS_PAGE);

    }
    else if (req_type == rtOrders)
    {
        fillOrdersTable(j_arr);

        //next request (orders: next page)
        /*
        next_cursor = jv.toObject().value("nextPageCursor").toString().trimmed();
        if (!next_cursor.isEmpty())
        {
            m_reqData->params.insert("cursor", next_cursor);
            sendRequest(MAX_ORDERS_PAGE);
        }
        */
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
        QString order_id = j_el.value("orderId").toString().trimmed();


        QStringList row_data;
        bool days_over = false;
        row_data << getTimePoint(j_el, days_over);
        row_data << j_el.value("symbol").toString() << j_el.value("qty").toString() << j_el.value("side").toString().toUpper();
        row_data << j_el.value("price").toString() << s_freezed;
        row_data << o_type;// << j_el.value("orderStatus").toString();
        LTable::addTableRow(t, row_data);

        if (days_over) t->item(i, DATE_COL)->setTextColor("#880000");
        t->item(i, 0)->setData(Qt::UserRole, order_id);

        qDebug()<<QString("%1  order_id[%2]").arg(t->item(i, TICKER_COL)->text()).arg(order_id);

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
void BB_PositionsPage::initPopupMenu()
{
    QString path = APIConfig::commonIconsPath(); // icons path

    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Modify", QString("%1/ball_green.svg").arg(path));
    QPair<QString, QString> pair2("Cancel", QString("%1/ball_red.svg").arg(path));
    act_list.append(pair1);
    act_list.append(pair2);

    //init popup menu actions
    m_orderTable->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_orderTable->connectSlotToPopupAction(i_menu, this, SLOT(slotOrderModify())); i_menu++;
    m_orderTable->connectSlotToPopupAction(i_menu, this, SLOT(slotOrderCancel())); i_menu++;


    ////////////////////////////////////////
    act_list[1].first = "Close by market";
    m_table->popupMenuActivate(act_list);

    i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotPositionModify())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotPositionClose())); i_menu++;

}
// colntrol orders
void BB_PositionsPage::prepareReqOrderData(TradeOperationData &data)
{
    QTableWidget *t = m_orderTable->table();
    QList<int> sel_rows = LTable::selectedRows(t);
    if (sel_rows.isEmpty() || sel_rows.count() > 1)
    {
        data.order_type = totNone;
        emit signalError("can't modify order, selection record is invalid");
        return;
    }

    int row = sel_rows.first();
    data.ticker = t->item(row, TICKER_COL)->text().trimmed();
    data.lot_size = t->item(row, LOT_COL)->text().toFloat();
    data.asset_price = t->item(row, LIMIT_PRICE_COL)->text().toFloat();
    data.type = t->item(row, TRADE_TYPE_COL)->text().trimmed();
    data.custom_id = t->item(row, 0)->data(Qt::UserRole).toString().trimmed();

}
void BB_PositionsPage::slotOrderModify()
{
    qDebug("BB_PositionsPage::slotOrderModify()");
    TradeOperationData data(totModify);
    prepareReqOrderData(data);
    if (data.invalidType()) return;

    QString saved_id = data.custom_id;

    // start dialog
    APILinearModifyDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        qDebug()<<QString("d.isApply(MODIFY):  new limit price %1").arg(data.asset_price);
        data.custom_id = saved_id;
        sendTradeReq(data);
    }
    else
    {
        qDebug("canceled operation [MODIFY]");
        emit signalError("Operation canceled! [MODIFY]");
    }
}
void BB_PositionsPage::slotOrderCancel()
{
    qDebug("BB_PositionsPage::slotOrderCancel()");
    TradeOperationData data(totCancel);
    prepareReqOrderData(data);
    if (data.invalidType()) return;

    QString saved_id = data.custom_id;

    // start dialog
    APILinearCancelDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        qDebug("d.isApply(CANCEL)");
        data.custom_id = saved_id;
        sendTradeReq(data);
    }
    else
    {
        qDebug("canceled operation [CANCEL]");
        emit signalError("Operation canceled! [CANCEL]");
    }
}
void BB_PositionsPage::sendTradeReq(const TradeOperationData &data)
{
    emit signalMsg("Try send request on trade command ...........");
    qDebug()<<QString("Try send request trade order_id[%1] ..............").arg(data.custom_id);

    // reinit req params
    m_reqData->metod = hrmPost;
    m_reqData->uri = API_ORDER_CANCEL_URI;
    if (data.order_type == totModify) m_reqData->uri = API_ORDER_MODIFY_URI;
    m_reqData->req_type = rtOrders;

    m_reqData->params.clear();
    m_reqData->params.insert("category", m_category);
    m_reqData->params.insert("symbol", data.ticker);
    m_reqData->params.insert("orderId", data.custom_id);
    if (data.order_type == totModify)
        m_reqData->params.insert("price", QString::number(data.asset_price, 'f', 2));

    emit signalMsg(m_reqData->toStr());
    m_reqData->outParams();
    sendRequest();
}
// colntrol positions
void BB_PositionsPage::prepareReqPositionData(TradeOperationData &data)
{
    QTableWidget *t = m_table->table();
    QList<int> sel_rows = LTable::selectedRows(t);
    if (sel_rows.isEmpty() || sel_rows.count() > 1)
    {
        data.order_type = totNone;
        emit signalError("can't modify position, selection record is invalid");
        return;
    }

    int row = sel_rows.first();
    data.ticker = t->item(row, TICKER_COL)->text().trimmed();
    data.lot_size = t->item(row, LOT_COL)->text().toFloat();
    data.strike = t->item(row, LIMIT_PRICE_COL)->text().toFloat(); // open price
    data.asset_price = t->item(row, t->columnCount()-2)->text().toFloat(); // current price
    data.type = t->item(row, TRADE_TYPE_COL)->text().trimmed();
    data.result = t->item(row, t->columnCount()-1)->text().toFloat();

}
void BB_PositionsPage::slotPositionModify()
{
    qDebug("BB_PositionsPage::slotPositionModify()");
    TradeOperationData data(totPosStopPrice);
    prepareReqPositionData(data);
    if (data.invalidType()) return;

    // start dialog
    APIPositionControlDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        qDebug()<<QString("d.isApply(MODIFY_POS):  new limit price %1").arg(data.asset_price);
        sendTradePosControlReq(data);
    }
    else
    {
        qDebug("canceled operation [MODIFY_POS]");
        emit signalError("Operation canceled! [MODIFY_POS]");
    }
}
void BB_PositionsPage::slotPositionClose()
{
    qDebug("BB_PositionsPage::slotPositionClose()");
    TradeOperationData data(totClosePosByMarket);
    prepareReqPositionData(data);
    if (data.invalidType()) return;


    // start dialog
    APIPositionControlDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        qDebug()<<QString("d.isApply(CLOSE_POS):  current price %1").arg(data.asset_price);
        sendTradePosControlReq(data);
    }
    else
    {
        qDebug("canceled operation [CLOSE_POS]");
        emit signalError("Operation canceled! [CLOSE_POS]");
    }
}
void BB_PositionsPage::sendTradePosControlReq(const TradeOperationData &data)
{
    emit signalMsg("Try send request for control position ...........");

    // reinit req params
    m_reqData->metod = hrmPost;
    m_reqData->uri = API_ORDER_CREATE_URI;
    m_reqData->req_type = rtPositions;

    m_reqData->params.clear();
    m_reqData->params.insert("category", m_category);
    m_reqData->params.insert("symbol", data.ticker);
    QString side = (data.type.toLower() == "buy" ? "Sell" : "Buy");
    m_reqData->params.insert("side", side);

    if (data.order_type == totPosStopPrice)
    {
        m_reqData->params.insert("orderType", "Limit");
        m_reqData->params.insert("timeInForce", "GTC");
        m_reqData->params.insert("price", QString::number(data.asset_price, 'f', 2));
    }
    else // close by market
    {
        m_reqData->params.insert("orderType", "Market");
    }

    m_reqData->params.insert("reduceOnly", "true");
    m_reqData->params.insert("qty", QString::number(data.lot_size));

    // define positionIdx field
    int positionIdx = (data.isBuy() ? 1 : 2);
    if (data.ticker.contains("DOT")) positionIdx = 0;
    m_reqData->params.insert("positionIdx", QString::number(positionIdx));


    //send req
    emit signalMsg(m_reqData->toStr());
    m_reqData->outParams();
    sendRequest();
}



//BB_SpotPositionsPage
BB_SpotPositionsPage::BB_SpotPositionsPage(QWidget *parent)
    :BB_PositionsPage(parent)
{
    setObjectName("spot_positions_page");
    reinitWidgets();

    m_userSign = rtSpotAssets;
    m_reqData->params.clear();

    m_orderTable->popupMenuDestroy();
    m_table->popupMenuDestroy();

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



//BB_OptionPositionsPage
BB_OptionPositionsPage::BB_OptionPositionsPage(QWidget *parent)
    :BB_PositionsPage(parent)
{
    setObjectName("option_positions_page");
    m_category = QString("option");

    reinitWidgets();

    m_userSign = rtOptionPositions;
    reinitReqData();
}
void BB_OptionPositionsPage::reinitWidgets()
{
    QStringList list;
    list << "Trigger time" << "Ticker" << "Volume" << "Action" << "Open price" << "Market price" << "Theta/Vega" << "Result";

    m_table->setHeaderLabels(list);
    m_table->resizeByContents();
    m_table->setTitle("Active options");

    list.clear();
    list << "Date" << "Ticker" << "Volume" << "Action" << "Open price" << "Status" << "Custom ID" << "Exec qty";
    m_orderTable->setHeaderLabels(list);
    m_orderTable->resizeByContents();
}
void BB_OptionPositionsPage::reinitReqData()
{
    m_reqData->params.clear();
    m_reqData->req_type = m_userSign;
    m_reqData->params.insert("category", m_category);
    m_reqData->params.insert("settleCoin", "USDT");

}
void BB_OptionPositionsPage::updateDataPage(bool forcibly)
{
    if (!updateTimeOver(forcibly)) return;

    clearTables();

    m_reqData->metod = hrmGet;
    m_reqData->uri = API_POS_URI;
    m_reqData->params.clear();
    m_reqData->req_type = rtOptionPositions;
    m_reqData->params.insert("category", m_category);

    sendRequest();
}
void BB_OptionPositionsPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != rtOptionPositions && req_type != rtOptionOrders) return;

    if (m_reqData->metod == hrmPost)
    {
        qDebug("WAS POST REQ");
        return;
    }


    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_OptionPositionsPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_OptionPositionsPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_OptionPositionsPage: j_arr QJsonArray is empty");}

    if (req_type == rtOptionPositions)
    {
        fillPosTable(j_arr);
        //return;


        m_reqData->uri = API_ORDERS_URI;
        m_reqData->req_type = rtOptionOrders;
        m_reqData->params.insert("cursor", QString::number(0));
        sendRequest(MAX_ORDERS_PAGE);
    }
    else if (req_type == rtOptionOrders)
    {
        fillOrdersTable(j_arr);
    }
}
void BB_OptionPositionsPage::fillPosTable(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) {qWarning()<<QString("BB_OptionPositionsPage::fillTable WARNING j_arr is empty"); return;}

    QTableWidget *t = m_table->table();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_OptionPositionsPage::fillTable WARNING j_el is empty (index=%1)").arg(i); break;}

        //    list << "Trigger time" << "Ticker" << "Lot" << "Action" << "Open price" << "Market price" << "Result";
        QString act = j_el.value("side").toString().trimmed().toUpper();

        float lot = j_el.value("size").toString().toFloat();
        float open_price = j_el.value("avgPrice").toString().toFloat();
        float cur_price = j_el.value("markPrice").toString().toFloat();
        float result = (open_price-cur_price)*lot;
        if (act == "BUY") result *= (-1);
        float theta = j_el.value("theta").toString().toFloat();
        float vega = j_el.value("vega").toString().toFloat();

        QStringList row_data;
        double ts = j_el.value("openTime").toDouble();
        row_data << APIConfig::fromTimeStamp(qint64(ts), QString("dd.MM.yyyy hh:mm"));
        row_data << j_el.value("symbol").toString();
        row_data << QString::number(lot, 'f', 2) << act;
        row_data << QString::number(open_price, 'f', 2);
        row_data << QString::number(cur_price, 'f', 2);
        row_data << QString("%1 / %2").arg(QString::number(theta, 'f', 3)).arg(QString::number(vega, 'f', 3));
        row_data << QString::number(result, 'f', 2);

        LTable::addTableRow(t, row_data);

        if (result < 0) t->item(i, t->columnCount()-1)->setTextColor("#AA0000");
    }

    m_table->resizeByContents();
    m_table->searchExec();
}
void BB_OptionPositionsPage::fillOrdersTable(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) return;

    QTableWidget *t = m_orderTable->table();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_OptionPositionsPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QStringList row_data;
        QString ts = j_el.value("createdTime").toString().trimmed();
        row_data << APIConfig::fromTimeStamp(ts.toLong(), QString("dd.MM.yyyy"));
        row_data << j_el.value("symbol").toString() << j_el.value("qty").toString() << j_el.value("side").toString().toUpper();
        row_data << j_el.value("price").toString() << j_el.value("orderStatus").toString();
        row_data << j_el.value("orderLinkId").toString() << j_el.value("cumExecQty").toString();

        QString order_id = j_el.value("orderId").toString().trimmed();

        LTable::addTableRow(t, row_data);
        t->item(i, 0)->setData(Qt::UserRole, order_id);
    }

    m_orderTable->resizeByContents();
    m_orderTable->searchExec();
}
void BB_OptionPositionsPage::slotOrderModify()
{
    qDebug("BB_OptionPositionsPage::slotOrderModify()");
    BB_PositionsPage::slotOrderModify();

}
void BB_OptionPositionsPage::slotOrderCancel()
{
    qDebug("BB_OptionPositionsPage::slotOrderCancel()");
    BB_PositionsPage::slotOrderCancel();

}
void BB_OptionPositionsPage::sendTradeReq(const TradeOperationData &data)
{
    emit signalMsg("Try send request on trade command for OPTION ...........");
    qDebug()<<QString("Try send request trade option_order_id[%1] ..............").arg(data.custom_id);


    // reinit req params
    m_reqData->metod = hrmPost;
    m_reqData->uri = API_ORDER_CANCEL_URI;
    if (data.order_type == totModify) m_reqData->uri = API_ORDER_MODIFY_URI;
    m_reqData->req_type = rtOptionOrders;


    m_reqData->params.clear();
    m_reqData->params.insert("category", m_category);
    m_reqData->params.insert("symbol", data.ticker);
    m_reqData->params.insert("orderId", data.custom_id);
    if (data.order_type == totModify)
        m_reqData->params.insert("price", QString::number(data.asset_price, 'f', 2));

    emit signalMsg(m_reqData->toStr());
    m_reqData->outParams();
    sendRequest();
}
void BB_OptionPositionsPage::prepareReqPositionData(TradeOperationData &data)
{
    BB_PositionsPage::prepareReqPositionData(data);
    if (data.invalidType()) return;

    QTableWidget *t = m_table->table();
    int row = LTable::selectedRows(t).first();
    data.asset_price = t->item(row, OPTION_MARKET_PRICE_COL)->text().toFloat(); // current price

}
void BB_OptionPositionsPage::sendTradePosControlReq(const TradeOperationData &data)
{
    emit signalMsg("Try send request for control option_position ...........");

    // reinit req params
    m_reqData->metod = hrmPost;
    m_reqData->uri = API_ORDER_CREATE_URI;
    m_reqData->req_type = rtOptionPositions;

    m_reqData->params.clear();
    m_reqData->params.insert("category", m_category);
    m_reqData->params.insert("symbol", data.ticker);
    QString side = (data.type.toLower() == "buy" ? "Sell" : "Buy");
    m_reqData->params.insert("side", side);

    if (data.order_type == totPosStopPrice)
    {
        m_reqData->params.insert("orderType", "Limit");
        m_reqData->params.insert("timeInForce", "GTC");
        m_reqData->params.insert("price", QString::number(data.asset_price, 'f', 2));
    }
    else // close by market
    {
        m_reqData->params.insert("orderType", "Market");
    }

    m_reqData->params.insert("reduceOnly", "true");
    m_reqData->params.insert("qty", QString::number(data.lot_size));

    // set field     orderLinkId
    QString custom_id = LString::strTrimLeft(data.ticker, 4);
    custom_id = LString::strTrimRight(custom_id, 5);
    QString s_time = QTime::currentTime().toString("hhmmss");
    QString kind = (data.type.toLower() == "sell" ? "buy" :  "sell");
    custom_id = QString("%1_%2_%3").arg(custom_id.trimmed().toLower()).arg(kind).arg(s_time);
    m_reqData->params.insert("orderLinkId", custom_id);


    //send req
    emit signalMsg(m_reqData->toStr());
    m_reqData->outParams();
    sendRequest();
}



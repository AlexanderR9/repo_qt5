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
#define TRADE_TYPE_COL              2
#define FREEZED_SUM_COL             4
#define MAX_ORDERS_PAGE             50


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
    m_table->addSortingData(0, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(1, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(2, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(3, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(4, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(5, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(7, LSearchTableWidgetBox::sdtNumeric);
    m_table->sortingOn();

    m_orderTable = new LSearchTableWidgetBox(this);
    m_orderTable->setObjectName("order_table");
    m_orderTable->setTitle("Placed orders");
    initTable(m_orderTable);

    m_orderTable->addSortingData(0, LSearchTableWidgetBox::sdtString);
    m_orderTable->addSortingData(1, LSearchTableWidgetBox::sdtNumeric);
    m_orderTable->addSortingData(2, LSearchTableWidgetBox::sdtString);
    m_orderTable->addSortingData(4, LSearchTableWidgetBox::sdtNumeric);
    m_orderTable->addSortingData(5, LSearchTableWidgetBox::sdtString);
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
    qDebug()<<QString("t_type=%1").arg(type);
    QStringList list;
    list << "Ticker" << "Volume" << "Action" << "Open price" << "Freezed sum";
    if (type.contains("pos")) list << "Leverage" << "Current price" << "Result";
    else if (type.contains("order")) list << "Order type";
    return list;
}
void BB_PositionsPage::updateDataPage(bool force)
{
    if (!updateTimeOver(force)) return;

    clearTables();
    m_reqData->uri = API_POS_URI;
    m_reqData->req_type = rtPositions;
    sendRequest(api_config.req_limit_pos);
}
void BB_PositionsPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    BB_BasePage::slotJsonReply(req_type, j_obj);
    if (req_type != rtPositions && req_type != rtOrders) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_PositionsPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_PositionsPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_PositionsPage: j_arr QJsonArray is empty"); return;}

    if (req_type == rtPositions)
    {
        fillPosTable(j_arr);

        //next request (orders: first page)
        m_reqData->uri = API_ORDERS_URI;
        m_reqData->req_type = rtOrders;
        m_reqData->params.insert("cursor", QString::number(0));
        sendRequest(MAX_ORDERS_PAGE);
    }
    else if (req_type == rtOrders)
    {
        //qDebug()<<QString("getted part of orders, size=%1, m_orderCursor=%2").arg(j_arr.count()).arg(m_orderCursor);
        fillOrdersTable(j_arr);

        //next request (orders: next page)
        QString next_cursor = jv.toObject().value("nextPageCursor").toString().trimmed();
        if (!next_cursor.isEmpty())
        {
            qDebug()<<QString("nextPageCursor: [%1]").arg(next_cursor);
            m_reqData->params.insert("cursor", next_cursor);
            sendRequest(MAX_ORDERS_PAGE);
        }
    }
}
void BB_PositionsPage::fillPosTable(const QJsonArray &j_arr)
{
    m_table->removeAllRows();
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

        row_data << j_el.value("symbol").toString() << j_el.value("size").toString() << tt;
        row_data << QString::number(open_price, 'f', prs) << QString::number(freezed, 'f', prs) << j_el.value("leverage").toString();
        row_data << QString::number(cur_price, 'f', prs) << QString::number(result, 'f', 2);

        LTable::addTableRow(m_table->table(), row_data);

        if (tt == "SELL")
            m_table->table()->item(i, TRADE_TYPE_COL)->setTextColor("#AA0000");

        if (result < 0)
            m_table->table()->item(i, m_table->table()->columnCount()-1)->setTextColor("#AA0000");
    }
    m_table->resizeByContents();
    m_table->searchExec();
}
void BB_PositionsPage::fillOrdersTable(const QJsonArray &j_arr)
{
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

        QStringList row_data;
        row_data << j_el.value("symbol").toString() << j_el.value("qty").toString() << j_el.value("side").toString().toUpper();
        row_data << j_el.value("price").toString() << s_freezed;
        row_data << o_type;
        LTable::addTableRow(m_orderTable->table(), row_data);
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



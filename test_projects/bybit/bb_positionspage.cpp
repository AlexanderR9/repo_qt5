#include "bb_positionspage.h"
#include "bb_apistruct.h"
#include "lhttp_types.h"
#include "ltable.h"
#include "apiconfig.h"

#include <QSplitter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>

#define API_POS_URI             QString("v5/position/list")
#define API_ORDERS_URI          QString("v5/order/realtime")

#define TRADE_TYPE_COL              2
#define FREEZED_SUM_COL             4



//BB_PositionsPage
BB_PositionsPage::BB_PositionsPage(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_table(NULL),
      m_orderTable(NULL),
      cur_stage(0)
{
    setObjectName("positions_page");
    init();

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
void BB_PositionsPage::updateDataPage()
{
    BB_APIReqParams req_data(QString("GET_POSITIONS"), hrmGet);
    switch (cur_stage)
    {
        case 0:
        {
            cur_stage = rtPositions;
            req_data.uri = API_POS_URI;
            break;
        }
        case rtPositions:
        {
            cur_stage = rtOrders;
            req_data.name = QString("GET_PLACED_ORDERS");
            req_data.uri = API_ORDERS_URI;
            break;
        }
        default:
        {
            cur_stage = 0;
            return;
        }
    }

    qDebug("BB_PositionsPage::updateDataPage()");
    req_data.params.insert("category", "linear");
    req_data.params.insert("settleCoin", "USDT");
    req_data.params.insert("limit", QString::number(api_config.req_limit_pos));
    req_data.req_type = cur_stage;

    emit signalSendReq(req_data);
}
void BB_PositionsPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    qDebug()<<QString("BB_ChartPage::slotJsonReply req_type=%1").arg(req_type);
    if (req_type != rtPositions && req_type != rtOrders) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_PositionsPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_PositionsPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_PositionsPage: j_arr QJsonArray is empty"); return;}

    if (req_type == rtPositions) fillPosTable(j_arr);
    else if (req_type == rtOrders) fillOrdersTable(j_arr);

    updateDataPage();
}
void BB_PositionsPage::fillPosTable(const QJsonArray &j_arr)
{
    qDebug("BB_PositionsPage::fillPosTable");
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
    qDebug("BB_PositionsPage::fillOrdersTable");
    m_orderTable->removeAllRows();
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



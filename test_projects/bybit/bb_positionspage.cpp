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
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Current positions");
    initTable(m_table);

    m_orderTable = new LTableWidgetBox(this);
    m_orderTable->setTitle("Placed orders");
    initTable(m_orderTable);

}
void BB_PositionsPage::initTable(LTableWidgetBox *t)
{
    if (!t) return;
    //t->vHeaderHide();
    t->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    t->setHeaderLabels(tableHeaders());
    t->resizeByContents();
    h_splitter->addWidget(t);
}
QStringList BB_PositionsPage::tableHeaders() const
{
    QStringList list;
    list << "Ticker" << "Volume" << "Leverage" << "Open price" << "Current price" << "Type";
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

        QStringList row_data;
        row_data << j_el.value("symbol").toString() << j_el.value("size").toString();
        row_data << j_el.value("leverage").toString() << j_el.value("avgPrice").toString();
        row_data << j_el.value("markPrice").toString() << j_el.value("side").toString().toUpper();

        LTable::addTableRow(m_table->table(), row_data);
    }
    m_table->resizeByContents();
}
void BB_PositionsPage::fillOrdersTable(const QJsonArray &j_arr)
{
    qDebug("BB_PositionsPage::fillOrdersTable");
    m_orderTable->removeAllRows();
    for (int i=0; i<j_arr.count(); i++)
    {

        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_PositionsPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QStringList row_data;
        row_data << j_el.value("symbol").toString() << j_el.value("qty").toString();
        row_data << "-" << j_el.value("price").toString();
        row_data << "-" << j_el.value("side").toString().toUpper();

        LTable::addTableRow(m_orderTable->table(), row_data);

    }
    m_orderTable->resizeByContents();
}



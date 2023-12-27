#include "bb_historypage.h"
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
#include <QGroupBox>

//#define API_POS_URI             QString("v5/position/list")
#define API_ORDERS_URI          QString("v5/order/history")



//BB_HistoryPage
BB_HistoryPage::BB_HistoryPage(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_table(NULL)
{
    setObjectName("history_page");
    init();

}
void BB_HistoryPage::init()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Current positions");
    m_table->vHeaderHide();
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_table->setHeaderLabels(tableHeaders());
    m_table->resizeByContents();

    QGroupBox *filterBox = new QGroupBox(this);
    filterBox->setTitle("Filter parameters");

    h_splitter->addWidget(m_table);
    h_splitter->addWidget(filterBox);
}
QStringList BB_HistoryPage::tableHeaders() const
{
    QStringList list;
    list << "Time" << "Ticker" << "Volume" << "Type";
    return list;
}
void BB_HistoryPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    qDebug()<<QString("BB_ChartPage::slotJsonReply req_type=%1").arg(req_type);
    if (req_type != rtPositions && req_type != rtOrders) return;

}
void BB_HistoryPage::updateDataPage()
{
    BB_APIReqParams req_data(QString("GET_HISTORY"), hrmGet);
    /*
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
    */

    qDebug("BB_HistoryPage::updateDataPage()");
    req_data.uri = API_ORDERS_URI;
    req_data.params.insert("category", "linear");
    req_data.params.insert("settleCoin", "USDT");
    //req_data.params.insert("orderStatus", "Cancelled");

    req_data.params.insert("limit", QString::number(api_config.req_limit_pos));
    req_data.req_type = rtHistory;

    emit signalSendReq(req_data);
}



#include "bb_spothistorypage.h"
#include "ltable.h"


#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>


#define ACTION_COL                  2


//Записи выполнения запросов пользователей, отсортированные по execTime в порядке убывания.
//Однако для классического спота они сортируются по execId в порядке убывания.
#define API_EXEC_ORDERS_URI      QString("v5/execution/list")

//BB_SpotHistoryPage
BB_SpotHistoryPage::BB_SpotHistoryPage(QWidget *parent)
    :BB_HistoryPage(parent)
{
    setObjectName("spot_history_page");
    reinitWidgets();

    m_userSign = rtSpotHistory;
    m_reqData->params.clear();
    m_reqData->uri = API_EXEC_ORDERS_URI;

}
void BB_SpotHistoryPage::loadTablesByContainers()
{


}
void BB_SpotHistoryPage::loadContainers()
{

}
void BB_SpotHistoryPage::fillTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tablePos->table();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_SpotHistoryPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        BB_HistorySpot pos;
        pos.fromJson(j_el);
        if (pos.invalid())
        {
            qWarning()<<QString("WARNING: invalid pos from j_el: ")<<pos.toFileLine();
            continue;
        }

        LTable::addTableRow(t, pos.toTableRowData());

        if (pos.isSell()) t->item(i, ACTION_COL)->setTextColor("#FF8C00");
        else if (pos.isBuy()) t->item(i, ACTION_COL)->setTextColor("#005500");
        else t->item(i, ACTION_COL)->setTextColor(Qt::red);
    }

    //m_tablePos->resizeByContents();

}
void BB_SpotHistoryPage::reinitWidgets()
{
    delete m_tableOrders;
    m_tableOrders = NULL;

    m_tablePos->setObjectName("spot_pos_table");
    m_tablePos->setTitle("Spot history");
    m_tablePos->setHeaderLabels(BB_HistorySpot::tableHeaders());

    viewTablesUpdate();
}
void BB_SpotHistoryPage::goExchange(const QJsonObject &jresult_obj)
{
   // qDebug()<<QString("BB_SpotHistoryPage::goExchange  h_stage=%1").arg(h_stage);
    if (h_stage == hsGetOrders)
    {
        qint64 ts1, ts2;
        getTSRange(ts1, ts2);
        if (ts1 < 0) return;

        m_reqData->req_type = userSign();
        m_reqData->params.insert("category", "spot");
        //m_reqData->params.insert("settleCoin", "USDT");
        m_reqData->params.insert("startTime", QString::number(ts1));
        if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
        else m_reqData->params.remove("endTime");
        //m_reqData->params.insert("cursor", QString::number(0));
        h_stage = hsWaitOrders;

        sendRequest(100);
    }
    else if (h_stage == hsWaitOrders)
    {
        h_stage = hsFinished;
      //  qDebug()<<QString("received response for spot history");
        const QJsonArray &j_arr = jresult_obj.value("list").toArray();
        if (j_arr.isEmpty())
        {
            emit signalMsg("j_arr.isEmpty()");
        }
        else
        {
            fillTable(j_arr);
            viewTablesUpdate();
        }
    }
}




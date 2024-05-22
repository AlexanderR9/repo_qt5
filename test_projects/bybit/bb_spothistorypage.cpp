#include "bb_spothistorypage.h"
#include "ltable.h"
#include "lfile.h"
#include "apiconfig.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QDir>


#define ACTION_COL                  2
#define FEE_TICKER_COL              6
#define REQ_LIMIT_RECORDS           100
//#define DAYS_SEPARATOR              6
#define RESULT_COL                  9



//Записи выполнения запросов пользователей, отсортированные по execTime в порядке убывания.
//Однако для классического спота они сортируются по execId в порядке убывания.
#define API_EXEC_ORDERS_URI         QString("v5/execution/list")

#define SPOT_FILE                   QString("spot_events.txt")


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
    if (m_spotOrders.isEmpty()) return;

    QTableWidget *t = m_tablePos->table();
    if (t->rowCount() > 0) return;

    //load positions
    for (int i=0; i<m_spotOrders.count(); i++)
    {
        const BB_HistorySpot &pos = m_spotOrders.at(i);
        LTable::addTableRow(t, pos.toTableRowData());
        updateLastRowColors(pos);
    }
    viewTablesUpdate();
}
void BB_SpotHistoryPage::loadContainers()
{
    m_spotOrders.clear();

    //loading events
    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(SPOT_FILE));
    if (LFile::fileExists(fname))
    {
        QStringList fdata;
        QString err = LFile::readFileSL(fname, fdata);
        if (err.isEmpty())
        {
            foreach (const QString &v, fdata)
            {
                if (v.trimmed().isEmpty()) continue;
                BB_HistorySpot pos;
                pos.fromFileLine(v);
                if (pos.invalid()) qWarning()<<QString("loadContainers() WARNING - invalid spot_event from line [%1]").arg(v);
                else insertSpotEvent(pos);
            }
        }
        else emit signalError(err);
    }
    else emit signalError(QString("spot file [%1] not found").arg(fname));

}
void BB_SpotHistoryPage::insertSpotEvent(const BB_HistorySpot &pos)
{
    if (hasPos(pos.uid)) {qWarning()<<QString("insertSpotEvent() WARNING - uid [%1] already exists").arg(pos.uid); return;}
    if (m_spotOrders.isEmpty()) {m_spotOrders.append(pos); return;}

    int n = m_spotOrders.count();
    for (int i=0; i<n; i++)
    {
        if (pos.triger_time > m_spotOrders.at(i).triger_time)
        {
            m_spotOrders.insert(i, pos);
            return;
        }
    }
    m_spotOrders.append(pos);
}
bool BB_SpotHistoryPage::hasSpotEvent(const QString &uid) const
{
    foreach (const BB_HistoryRecordBase &v, m_spotOrders)
        if (v.uid == uid) return true;
    return false;
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
        updateLastRowColors(pos);

        if (!hasSpotEvent(pos.uid))
        {
            m_spotOrders.append(pos);
            addToFile(m_spotOrders.last(), SPOT_FILE);
        }
    }
}
void BB_SpotHistoryPage::updateLastRowColors(const BB_HistorySpot &pos)
{
    QTableWidget *t = m_tablePos->table();
    int l_row = t->rowCount()-1;

    if (pos.isSell()) t->item(l_row, ACTION_COL)->setTextColor("#FF8C00");
    else if (pos.isBuy()) t->item(l_row, ACTION_COL)->setTextColor("#005500");
    else t->item(l_row, ACTION_COL)->setTextColor(Qt::red);

    if (t->item(l_row, FEE_TICKER_COL)->text() == "USDT")
    {
        t->item(l_row, FEE_TICKER_COL)->setTextColor("#C0C0C0");
        t->item(l_row, RESULT_COL)->setTextColor("#C0C0C0");
    }

    if (pos.pFee() < 0.09 || pos.pFee() > 0.11)
        t->item(l_row, FEE_TICKER_COL-1)->setTextColor("#EE0000");
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
    //qDebug()<<QString("BB_SpotHistoryPage::goExchange  h_stage=%1").arg(h_stage);
    switch (h_stage)
    {
        case hsGetOrders: {sendOrdersReq(); break;}
        case hsGetNextOrders: {sendNextOrdersReq(); break;}
        case hsWaitOrders: {parseOrders(jresult_obj); break;}
        case hsFinished:
        {
            viewTablesUpdate();
            emit signalMsg("SPOT_HISTORY finished!");
            break;
        }
        default: break;
    }
}
void BB_SpotHistoryPage::sendOrdersReq()
{
    h_stage = hsFinished;
    m_polledDays = 0;

    m_startDate = QDate::fromString(m_startDateEdit->text().trimmed(), APIConfig::userDateMask());
    if (!m_startDate.isValid())
    {
        emit signalError(QString("BB_SpotHistoryPage: invalid start date"));
        return;
    }

    qint64 ts1, ts2;
    getTSNextInterval(ts1, ts2);
    if (ts1 < 0) {emit signalError("getting time range"); return;}

    m_reqData->req_type = userSign();
    m_reqData->params.insert("category", "spot");
    m_reqData->params.insert("startTime", QString::number(ts1));
    if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
    else m_reqData->params.remove("endTime");
    h_stage = hsWaitOrders;

    sendRequest(REQ_LIMIT_RECORDS);
}
void BB_SpotHistoryPage::sendNextOrdersReq()
{
    h_stage = hsFinished;

    qint64 ts1, ts2;
    getTSNextInterval(ts1, ts2);
    if (ts1 < 0) {emit signalError("getting time range"); return;}

    m_reqData->params.insert("startTime", QString::number(ts1));
    if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
    else m_reqData->params.remove("endTime");
    h_stage = hsWaitOrders;

    sendRequest(REQ_LIMIT_RECORDS);
}
void BB_SpotHistoryPage::parseOrders(const QJsonObject &jresult_obj)
{
  //  qDebug()<<QString("received response for spot history");
    const QJsonArray &j_arr = jresult_obj.value("list").toArray();
    if (j_arr.isEmpty())
    {
        emit signalMsg("j_arr.isEmpty()");
    }
    else
    {
        qDebug()<<QString("received %1 records from server API").arg(j_arr.count());
        fillTable(j_arr);
    }

    if (m_polledDays >= needPollDays()) h_stage = hsFinished;
    else h_stage = hsGetNextOrders;
    goExchange(QJsonObject());
}



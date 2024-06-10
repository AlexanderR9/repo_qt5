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
#include <QLabel>
#include <QDir>
#include <QSplitter>
#include <QSettings>
#include <QGridLayout>


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
    :BB_HistoryPage(parent),
      m_needRewriteFile(false),
      m_amountEdit(NULL)
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
bool BB_SpotHistoryPage::hasSpotEvent(const BB_HistorySpot &other) const
{
    if (m_loadedDates.contains(other.triger_time.date())) return true;

    /*
    foreach (const BB_HistoryRecordBase &v, m_spotOrders)
    {
        if (v.uid == other.uid)
        {
            if ((static_cast<const BB_HistorySpot&>(v)).ts >= other.ts) return true;
        }
    }
    */
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

        if (!hasSpotEvent(pos))
        {
            m_spotOrders.append(pos);
            m_needRewriteFile = true;
        }
    }
}
void BB_SpotHistoryPage::bringTogetherEvents()
{
    while (2 > 1)
    {
        int n = m_spotOrders.count();
        if (n < 2) break;

        QString d_uid = hasDoubleUid();
        if (d_uid.isEmpty()) break;


        int f_index = -1;
        for (int i=0; i<n; i++)
            if (m_spotOrders.at(i).uid == d_uid) {f_index=i; break;}

        const BB_HistorySpot &pos = m_spotOrders.at(f_index);
        emit signalMsg(QString("FOUND DOUBLE UID: [%1]").arg(pos.toFileLine()));


        qDebug("####### bring together kit #########");
        qDebug()<<QString("%0: uid=%1  time=%2  price=%3  lot=%4 index=%5").arg(pos.ticker).arg(pos.uid).
                  arg(pos.ts).arg(pos.price).arg(pos.lot_size).arg(f_index);
        for (int i=n-1; i>=0; i--)
        {
            if (i == f_index) break;

            const BB_HistorySpot &pos = m_spotOrders.at(i);
            if (pos.uid != d_uid) continue;

            qDebug()<<QString("%0: uid=%1  time=%2  price=%3  lot=%4 index=%5").arg(pos.ticker).arg(pos.uid).
                      arg(pos.ts).arg(pos.price).arg(pos.lot_size).arg(i);


            if (pos.triger_time > m_spotOrders.at(f_index).triger_time)
                m_spotOrders[f_index].triger_time = pos.triger_time;

            m_spotOrders[f_index].lot_size += pos.lot_size;
            m_spotOrders[f_index].fee += pos.fee;

            m_spotOrders.removeAt(i);
        }
    }
}
void BB_SpotHistoryPage::rewriteHistoryFile()
{
    QStringList fdata;
    foreach (const BB_HistoryRecordBase &v, m_spotOrders)
        fdata << v.toFileLine();


    QString fname = QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(SPOT_FILE);
    QString err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg(QString("file [%1] was rewrited / rec_number=%2 !").arg(fname).arg(m_spotOrders.count()));

}
void BB_SpotHistoryPage::sortByExecTime()
{
    int n = m_spotOrders.count();
    if (n < 3) return;

    while (2 > 1)
    {
        bool has_replace = false;
        for (int i=1; i<n; i++)
        {
            if (m_spotOrders.at(i).triger_time > m_spotOrders.at(i-1).triger_time)
            {
                BB_HistorySpot replaced_pos(m_spotOrders.takeAt(i));
                m_spotOrders.insert(i-1, replaced_pos);
                has_replace = true;
                break;
            }
        }
        if (!has_replace) break;
    }
}
QString BB_SpotHistoryPage::hasDoubleUid() const
{
    QStringList uids;
    foreach (const BB_HistoryRecordBase &v, m_spotOrders)
    {
        if (uids.contains(v.uid)) return v.uid;
        uids.append(v.uid);
    }
    return QString();
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

    QLabel *amountLabel = new QLabel("Amount volume", this);
    m_amountEdit = new QLineEdit(this);
    m_amountEdit->setReadOnly(true);

    QGridLayout *g_lay = qobject_cast<QGridLayout*>(h_splitter->widget(1)->layout());
    if (g_lay)
    {
        qDebug()<<h_splitter->widget(1)->objectName();
        g_lay->addWidget(amountLabel, 3, 0);
        g_lay->addWidget(m_amountEdit, 3, 1);
    }

    viewTablesUpdate();

    connect(m_tablePos, SIGNAL(signalSearched()), this, SLOT(slotSearched()));
}
void BB_SpotHistoryPage::goExchange(const QJsonObject &jresult_obj)
{
    //qDebug()<<QString("BB_SpotHistoryPage::goExchange  h_stage=%1").arg(h_stage);
    switch (h_stage)
    {
        case hsGetOrders: {sendOrdersReq(); break;}
        case hsGetNextOrders: {sendNextOrdersReq(); break;}
        case hsWaitOrders: {parseOrders(jresult_obj); break;}
        case hsFinished: {finishReqScenario(); break;}
        default: break;
    }
}
void BB_SpotHistoryPage::finishReqScenario()
{
    viewTablesUpdate();

    if (m_needRewriteFile)
    {
        bringTogetherEvents();
        sortByExecTime();
        rewriteHistoryFile();
    }

    emit signalMsg("SPOT_HISTORY FINISHED!");
}
void BB_SpotHistoryPage::sendOrdersReq()
{
    h_stage = hsFinished;
    m_polledDays = 0;
    prepareLoadedDates();

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

    if (m_polledDays >= needPollDays() || m_polledDays < 0) h_stage = hsFinished;
    else h_stage = hsGetNextOrders;
    goExchange(QJsonObject());
}
void BB_SpotHistoryPage::load(QSettings &settings)
{
    BB_BasePage::load(settings);
    m_startDateEdit->setText(settings.value(QString("%1/startDate").arg(objectName()), QString()).toString());
    m_historyDaysCombo->setCurrentIndex(settings.value(QString("%1/historyDaysIndex").arg(objectName()), 0).toInt());
    loadContainers();

    emit signalMsg(QString("CONTAINERS STATE: spot_events %1").arg(m_spotOrders.count()));
}
void BB_SpotHistoryPage::prepareLoadedDates()
{
    m_needRewriteFile = false;
    m_loadedDates.clear();
    foreach (const BB_HistorySpot &v, m_spotOrders)
    {
        QDate d(v.triger_time.date());
        if (!m_loadedDates.contains(d)) m_loadedDates.append(d);
    }
}
void BB_SpotHistoryPage::slotSearched()
{
    QTableWidget *t = m_tablePos->table();
    float a = 0;
    for (int i=0; i<t->rowCount(); i++)
    {
        if (!t->isRowHidden(i)) a += m_spotOrders.at(i).lot_size;
    }
    m_amountEdit->setText(QString::number(a, 'f', 4));
}



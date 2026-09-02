#include "bb_optionhistorypage.h"
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


#define ACTION_COL                  4
#define FEE_TICKER_COL              8
#define REQ_LIMIT_RECORDS           100
#define RESULT_COL                  9



//Записи выполнения запросов пользователей, отсортированные по execTime в порядке убывания.
// Получение history опционов
#define API_OPTION_HISTORY_URI           QString("v5/position/get-closed-positions")


#define OPTION_FILE                   QString("option_events.txt")


//BB_OptionHistoryPage
BB_OptionHistoryPage::BB_OptionHistoryPage(QWidget *parent)
    :BB_HistoryPage(parent),
    m_needRewriteFile(false)
      //m_amountEdit(NULL)
{
    setObjectName("option_history_page");
    reinitWidgets();

    m_userSign = rtOptionHistory;
    m_reqData->params.clear();
    m_reqData->uri = API_OPTION_HISTORY_URI;

}
void BB_OptionHistoryPage::reinitWidgets()
{
    delete m_tableOrders;
    m_tableOrders = NULL;

    m_tablePos->setObjectName("option_pos_table");
    m_tablePos->setTitle("Option history");
    m_tablePos->setHeaderLabels(BB_HistoryOption::tableHeaders());

    //QLabel *amountLabel = new QLabel("Amount volume", this);
    //m_amountEdit = new QLineEdit(this);
    //m_amountEdit->setReadOnly(true);

    /*
    QGridLayout *g_lay = qobject_cast<QGridLayout*>(h_splitter->widget(1)->layout());
    if (g_lay)
    {
        qDebug()<<h_splitter->widget(1)->objectName();
        g_lay->addWidget(amountLabel, 3, 0);
        g_lay->addWidget(m_amountEdit, 3, 1);
    }
    */

    viewTablesUpdate();

    //connect(m_tablePos, SIGNAL(signalSearched()), this, SLOT(slotSearched()));
}
void BB_OptionHistoryPage::loadTablesByContainers()
{
    if (m_positions.isEmpty()) return;

    QTableWidget *t = m_tablePos->table();
    if (t->rowCount() > 0) return;

    //load positions
    for (int i=0; i<m_positions.count(); i++)
    {
        BB_HistoryOption pos(m_positions.at(i));       
        LTable::addTableRow(t, pos.toTableRowData());
        updateLastRowColors(pos);
    }
    viewTablesUpdate();
}
void BB_OptionHistoryPage::loadContainers()
{
    m_positions.clear();

    //loading events
    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(OPTION_FILE));
    if (LFile::fileExists(fname))
    {
        QStringList fdata;
        QString err = LFile::readFileSL(fname, fdata);
        if (err.isEmpty())
        {
            foreach (const QString &v, fdata)
            {
                if (v.trimmed().isEmpty()) continue;
                BB_HistoryOption pos;
                pos.fromFileLine(v);
                if (pos.invalid()) qWarning()<<QString("loadContainers() WARNING - invalid option_event from line [%1]").arg(v);
                else  m_positions.append(pos); //insertSpotEvent(pos);
            }
        }
        else emit signalError(err);
    }
    else emit signalError(QString("spot file [%1] not found").arg(fname));


}
bool BB_OptionHistoryPage::hasSpotEvent(const BB_HistoryOption &other) const
{
    foreach (const BB_HistoryOption &v, m_positions)
        if (v.uid == other.uid) return true;
    return false;
}
void BB_OptionHistoryPage::fillTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tablePos->table();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_OptionHistoryPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        BB_HistoryOption pos;
        pos.fromJson(j_el);
        if (pos.invalid())
        {
            qWarning()<<QString("WARNING: invalid pos from j_el: ")<<pos.toFileLine();
            continue;
        }
        //qDebug()<<QString("Received SPOT record: %1").arg(pos.toFileLine());


        LTable::addTableRow(t, pos.toTableRowData());
        updateLastRowColors(pos);

        if (!hasSpotEvent(pos))
        {
            m_positions.append(pos);
            m_needRewriteFile = true;
        }
        //else qDebug("   this record olready exists");
    }

}
void BB_OptionHistoryPage::rewriteHistoryFile()
{
    QStringList fdata;
    foreach (const BB_HistoryRecordBase &v, m_positions)
        fdata << v.toFileLine();

    QString fname = QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(OPTION_FILE);
    QString err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg(QString("file [%1] was rewrited / rec_number=%2 !").arg(fname).arg(m_positions.count()));
}
void BB_OptionHistoryPage::sortByCloseTime()
{
    int n = m_positions.count();
    if (n < 3) return;

    while (2 > 1)
    {
        bool has_replace = false;
        for (int i=1; i<n; i++)
        {
            if (m_positions.at(i).closed_time > m_positions.at(i-1).closed_time)
            {
                BB_HistoryOption replaced_pos(m_positions.takeAt(i));
                m_positions.insert(i-1, replaced_pos);
                has_replace = true;
                break;
            }
        }
        if (!has_replace) break;
    }
}
void BB_OptionHistoryPage::updateLastRowColors(const BB_HistoryOption &pos)
{
    QTableWidget *t = m_tablePos->table();
    int l_row = t->rowCount()-1;

    if (pos.isSell()) t->item(l_row, ACTION_COL)->setTextColor("#FF8C00");
    else if (pos.isBuy()) t->item(l_row, ACTION_COL)->setTextColor("#005500");
    else t->item(l_row, ACTION_COL)->setTextColor(Qt::red);

    t->item(l_row, FEE_TICKER_COL)->setTextColor("#C0C0C0");

    if (qAbs(pos.total_result) < 0.5) t->item(l_row, RESULT_COL)->setTextColor("#C0C0C0");
    else if (pos.total_result < 0) t->item(l_row, RESULT_COL)->setTextColor(Qt::darkRed);
    else t->item(l_row, RESULT_COL)->setTextColor("#0000EE");

}
void BB_OptionHistoryPage::goExchange(const QJsonObject &jresult_obj)
{
    qDebug()<<QString("BB_OptionHistoryPage::goExchange  h_stage=%1").arg(h_stage);
    switch (h_stage)
    {
        case hsGetOrders: {sendOrdersReq(); break;}
        case hsGetNextOrders: {sendNextOrdersReq(); break;}
        case hsWaitOrders: {parseOrders(jresult_obj); break;}
        case hsFinished: {finishReqScenario(); break;}
        default: break;
    }
}
void BB_OptionHistoryPage::finishReqScenario()
{
    viewTablesUpdate();
    if (m_needRewriteFile)
    {
        sortByCloseTime();
        rewriteHistoryFile();
    }
    emit signalMsg("SPOT_HISTORY FINISHED!");
}
void BB_OptionHistoryPage::sendOrdersReq()
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
    m_reqData->params.insert("category", "option");
    m_reqData->params.insert("startTime", QString::number(ts1));
    if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
    else m_reqData->params.remove("endTime");
    h_stage = hsWaitOrders;

    sendRequest(REQ_LIMIT_RECORDS);
}
void BB_OptionHistoryPage::sendNextOrdersReq()
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
void BB_OptionHistoryPage::parseOrders(const QJsonObject &jresult_obj)
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
void BB_OptionHistoryPage::load(QSettings &settings)
{
    BB_BasePage::load(settings);
    m_startDateEdit->setText(settings.value(QString("%1/startDate").arg(objectName()), QString()).toString());
    m_historyDaysCombo->setCurrentIndex(settings.value(QString("%1/historyDaysIndex").arg(objectName()), 0).toInt());
    loadContainers();

    emit signalMsg(QString("CONTAINERS STATE: option_events %1").arg(m_positions.count()));

}

/*
void BB_OptionHistoryPage::slotSearched()
{
    QTableWidget *t = m_tablePos->table();
    float a = 0;
    for (int i=0; i<t->rowCount(); i++)
    {
      //  if (!t->isRowHidden(i)) a += m_spotOrders.at(i).lot_size;
    }
    //m_amountEdit->setText(QString::number(a, 'f', 4));
}
*/





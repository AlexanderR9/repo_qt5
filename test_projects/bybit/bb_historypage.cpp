#include "bb_historypage.h"
#include "bb_apistruct.h"
#include "lhttp_types.h"
#include "ltable.h"
#include "apiconfig.h"
#include "lfile.h"

#include <QSplitter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDir>
#include <QSettings>

//Запросить историю заказов.
//Поскольку создание/отмена заказа является асинхронным, данные, возвращаемые из этой конечной точки, могут задерживаться.
#define API_ORDERS_URI      QString("v5/order/history")

//Записи выполнения запросов пользователей, отсортированные по execTime в порядке убывания.
//Однако для классического спота они сортируются по execId в порядке убывания.
#define API_EXEC_ORDERS_URI      QString("v5/execution/list")

//Запросить закрытые записи о прибылях и убытках пользователя
#define API_PL_URI          QString("v5/position/closed-pnl")

#define MAX_ORDERS_PAGE             49
#define COMMISION_COL               6
#define PROFIT_COL                  7
//#define DEVIATION_COL               4
#define ACTION_COL                  2

#define POS_FILE            QString("pos_events.txt")
#define ORDER_FILE          QString("order_events.txt")


//BB_HistoryPage
BB_HistoryPage::BB_HistoryPage(QWidget *parent)
    :BB_BasePage(parent, 32, rtHistory),
    m_tableOrders(NULL),
    m_tablePos(NULL),
    m_startDateEdit(NULL),
    m_historyDaysCombo(NULL),
    h_stage(-1)
{
    setObjectName("history_page");
    init();

    m_reqData->uri.clear();
    m_reqData->params.clear();

}
void BB_HistoryPage::checkReceivedRecord(const BB_HistoryRecordBase &json_rec)
{
    switch (h_stage)
    {
        case hsWaitOrders:
        {
            if (!hasOrder(json_rec.uid))
            {
                m_orderList.append(static_cast<const BB_HistoryOrder&>(json_rec));
                addToFile(m_orderList.last(), ORDER_FILE);
            }
            break;
        }
        case hsWaitPos:
        {
            if (!hasPos(json_rec.uid))
            {
                m_posList.append(static_cast<const BB_HistoryPos&>(json_rec));
                addToFile(m_posList.last(), POS_FILE);
            }
            break;
        }
        default: break;
    }
}
void BB_HistoryPage::addToFile(const BB_HistoryRecordBase &rec, QString fname)
{
    if (fname.trimmed().isEmpty()) {signalError("BB_HistoryPage: fname is empty"); return;}

    QString err;
    fname = QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(fname);
    if (LFile::fileExists(fname)) err = LFile::appendFile(fname, rec.toFileLine());
    else err = LFile::writeFile(fname, rec.toFileLine());
    if (!err.isEmpty()) emit signalError(err);
}
bool BB_HistoryPage::hasPos(const QString &uid) const
{
    foreach (const BB_HistoryRecordBase &v, m_posList)
        if (v.uid == uid) return true;
    return false;
}
bool BB_HistoryPage::hasOrder(const QString &uid) const
{
    foreach (const BB_HistoryRecordBase &v, m_orderList)
        if (v.uid == uid) return true;
    return false;
}
void BB_HistoryPage::loadContainers()
{
    m_posList.clear();
    m_orderList.clear();

    //loading positions
    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(POS_FILE));
    if (LFile::fileExists(fname))
    {
        QStringList fdata;
        QString err = LFile::readFileSL(fname, fdata);
        if (err.isEmpty())
        {
            foreach (const QString &v, fdata)
            {
                if (v.trimmed().isEmpty()) continue;
                BB_HistoryPos pos;
                pos.fromFileLine(v);
                if (pos.invalid()) qWarning()<<QString("loadContainers() WARNING - invalid pos from line [%1]").arg(v);
                else m_posList.append(pos);
            }
        }
        else emit signalError(err);
    }
    else emit signalError(QString("pos file [%1] not found").arg(fname));

    //loading orders
    fname = QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(ORDER_FILE);
    if (LFile::fileExists(fname))
    {
        QStringList fdata;
        QString err = LFile::readFileSL(fname, fdata);
        if (err.isEmpty())
        {
            foreach (const QString &v, fdata)
            {
                if (v.trimmed().isEmpty()) continue;
                BB_HistoryOrder order;
                order.fromFileLine(v);
                if (order.invalid()) qWarning()<<QString("loadContainers() WARNING - invalid pos from line [%1]").arg(v);
                else m_orderList.append(order);
            }
        }
        else emit signalError(err);
    }
    else emit signalError(QString("pos file [%1] not found").arg(fname));
}
void BB_HistoryPage::init()
{
    initOrdersTable();
    initPosTable();
    initFilterBox();

    v_splitter->addWidget(m_tablePos);
    v_splitter->addWidget(m_tableOrders);
}
void BB_HistoryPage::initOrdersTable()
{
    m_tableOrders = new LSearchTableWidgetBox(this);
    m_tableOrders->setObjectName("orders_table");
    m_tableOrders->setTitle("Orders");
    m_tableOrders->vHeaderHide();
    m_tableOrders->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableOrders->setHeaderLabels(BB_HistoryOrder::tableHeaders());
    m_tableOrders->resizeByContents();
    m_tableOrders->searchEditHide();

}
void BB_HistoryPage::initPosTable()
{
    m_tablePos = new LSearchTableWidgetBox(this);
    m_tablePos->setObjectName("pos_table");
    m_tablePos->setTitle("Positions");
    m_tablePos->vHeaderHide();
    m_tablePos->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tablePos->setHeaderLabels(BB_HistoryPos::tableHeaders());
    m_tablePos->resizeByContents();
    m_tablePos->searchEditHide();
}
void BB_HistoryPage::initFilterBox()
{
    QGroupBox *filterBox = new QGroupBox(this);
    filterBox->setTitle("Filter parameters");
    h_splitter->addWidget(filterBox);

    if (filterBox->layout()) delete filterBox->layout();
    QGridLayout *g_lay = new QGridLayout(filterBox);

    quint8 lay_row = 0;
    g_lay->addWidget(new QLabel("Search", this), lay_row, 0);
    QLineEdit *search_edit = new QLineEdit(this);
    g_lay->addWidget(search_edit, lay_row, 1);
    lay_row++;

    g_lay->addWidget(new QLabel("Start date", this), lay_row, 0);
    m_startDateEdit = new QLineEdit(this);
    g_lay->addWidget(m_startDateEdit, lay_row, 1);
    lay_row++;

    g_lay->addWidget(new QLabel("History days", this), lay_row, 0);
    m_historyDaysCombo = new QComboBox(this);
    g_lay->addWidget(m_historyDaysCombo, lay_row, 1);
    for (int i=1; i<=10; i++) m_historyDaysCombo->addItem(QString::number(i));
    for (int i=1; i<=10; i++) m_historyDaysCombo->addItem(QString::number(i*30));
    lay_row++;


    g_lay->addItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding), lay_row,0);
    m_tableOrders->setSearchEdit(search_edit);
    m_tablePos->setSearchEdit(search_edit);
}
QStringList BB_HistoryPage::tableHeaders(QString obj_name) const
{
    QStringList list;
    list << "Create time" << "Ticker" << "Action" << "Volume" ;
    if (obj_name.contains("order"))
    {
        list << "Type" << "Price" << "Status";
    }
    else if (obj_name.contains("pos"))
    {
        list << "Price" << "Commision" << "Total result" << "Exec type" << "Order type" << "Leverage";
        list.replace(0, "Close time");
    }
    return list;
}
void BB_HistoryPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    BB_BasePage::slotJsonReply(req_type, j_obj);
    if (req_type != m_userSign) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_HistoryPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_HistoryPage: list QJsonValue not found"); return;}

    goExchange(jv.toObject());
}
void BB_HistoryPage::goExchange(const QJsonObject &jresult_obj)
{
    switch (h_stage)
    {
        case hsGetOrders: {prepareOrdersReq(); break;}
        case hsWaitOrders: {waitOrders(jresult_obj); return;}
        case hsGetPos: {preparePosReq(); break;}
        case hsWaitPos: {waitPos(jresult_obj); return;}
        case hsFinished: {finished(); break;}
        default: return;
    }

    if (!m_reqData->uri.isEmpty())
        sendRequest(MAX_ORDERS_PAGE);
}
void BB_HistoryPage::preparePosReq()
{
    //qDebug("BB_HistoryPage::preparePosReq()");
    m_reqData->uri.clear();
    viewTablesUpdate();

    m_reqData->uri = API_PL_URI;
    m_reqData->params.remove("settleCoin");
    m_reqData->params.insert("cursor", QString::number(0));
    //m_reqData->params.insert("symbol", "USDT");
    h_stage = hsWaitPos;
}
void BB_HistoryPage::prepareOrdersReq()
{
    //qDebug("BB_HistoryPage::prepareOrdersReq()");
    m_reqData->uri.clear();
    qint64 ts1, ts2;
    getTSRange(ts1, ts2);
    if (ts1 < 0) return;

    m_reqData->uri = API_ORDERS_URI;
    m_reqData->params.insert("category", "linear");
    m_reqData->params.insert("settleCoin", "USDT");
    m_reqData->params.insert("startTime", QString::number(ts1));
    if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
    else m_reqData->params.remove("endTime");
    m_reqData->params.insert("cursor", QString::number(0));
    h_stage = hsWaitOrders;
}
void BB_HistoryPage::finished()
{
    //qDebug("BB_HistoryPage::finished()");
    emit signalMsg("#######################FINISHED!#############################");
    m_reqData->uri.clear();
    viewTablesUpdate();
}
void BB_HistoryPage::waitPos(const QJsonObject &jresult_obj)
{
    //qDebug("BB_HistoryPage::waitPos()");
    const QJsonArray &j_arr = jresult_obj.value("list").toArray();
    if (j_arr.isEmpty())
    {
        emit signalMsg("getting position finished");
        h_stage = hsFinished;
        goExchange(jresult_obj);
        return;
    }

    fillPosTable(j_arr);
    //h_stage = hsFinished;    goExchange(jresult_obj);    return; //for test

    //next request (pos: next page)
    QString next_cursor = jresult_obj.value("nextPageCursor").toString().trimmed();
    //qDebug()<<QString("nextPageCursor: [%1]").arg(next_cursor);
    m_reqData->params.insert("cursor", next_cursor);
    sendRequest(MAX_ORDERS_PAGE);
}
void BB_HistoryPage::waitOrders(const QJsonObject &jresult_obj)
{
    //qDebug("BB_HistoryPage::waitOrders()");
    const QJsonArray &j_arr = jresult_obj.value("list").toArray();
    if (j_arr.isEmpty())
    {
        emit signalMsg("getting orders finished");
        h_stage = hsGetPos;
        goExchange(jresult_obj);
        return;
    }

    fillOrdersTable(j_arr);
    //return;

    //next request (orders: next page)
    QString next_cursor = jresult_obj.value("nextPageCursor").toString().trimmed();
   // qDebug()<<QString("nextPageCursor: [%1]").arg(next_cursor);
    m_reqData->params.insert("cursor", next_cursor);
    sendRequest(MAX_ORDERS_PAGE);
}
void BB_HistoryPage::updateDataPage(bool force)
{
    if (!updateTimeOver(force)) return;

    qDebug("Start BB_HistoryPage::updateDataPage");
    m_tablePos->removeAllRows();
    m_tableOrders->removeAllRows();
    h_stage = hsGetOrders;
    goExchange(QJsonObject());
}
void BB_HistoryPage::getTSRange(qint64 &ts1, qint64 &ts2)
{
    ts1 = ts2 = -1;
    QDate start_date(QDate::fromString(m_startDateEdit->text().trimmed(), APIConfig::userDateMask()));
    if (!start_date.isValid())
    {
        emit signalError(QString("HistoryPage: invalid start date"));
        return;
    }

    quint8 dd = m_historyDaysCombo->currentText().toUInt();
    QDate end_date(start_date.addDays(dd));

    ts1 = APIConfig::toTimeStamp(start_date.day(), start_date.month(), start_date.year());
    if (dd <= 7) ts2 = APIConfig::toTimeStamp(end_date.day(), end_date.month(), end_date.year());
    emit signalMsg(QString("HISTORY INTERVAL: [%1 - %2]").arg(start_date.toString(APIConfig::userDateMask())).arg(end_date.toString(APIConfig::userDateMask())));
}
void BB_HistoryPage::fillOrdersTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tableOrders->table();
   // qDebug()<<QString("fillOrdersTable  arr_size=%1").arg(j_arr.count());
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_PositionsPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        BB_HistoryOrder order;
        order.fromJson(j_el);
        if (order.invalid())
        {
            qWarning()<<QString("WARNING: invalid order from j_el: ")<<order.toFileLine();
            continue;
        }
        LTable::addTableRow(t, order.toTableRowData());
        checkReceivedRecord(order);


        /*
        QStringList row_data;
        row_data << APIConfig::fromTimeStamp(j_el.value("createdTime").toString().toLong());
        row_data << j_el.value("symbol").toString();
        row_data << j_el.value("side").toString().trimmed().toUpper();
        row_data << j_el.value("qty").toString();

        QString o_type = j_el.value("stopOrderType").toString().toUpper().trimmed();
        if (o_type.isEmpty()) o_type = j_el.value("orderType").toString().toUpper().trimmed();
        row_data << o_type;

        QString s_price("-");
        if (o_type == "LIMIT")
        {
            float p = j_el.value("price").toString().toFloat();
            s_price = QString::number(p, 'f', 2);
        }
        else if (o_type == "TAKEPROFIT" || o_type == "STOPLOSS")
        {
            float p = j_el.value("triggerPrice").toString().toFloat();
            s_price = QString::number(p, 'f', 2);
        }
        row_data << s_price;
        row_data << j_el.value("orderStatus").toString();

        LTable::addTableRow(t, row_data);
        QString s_id = j_el.value("orderId").toString();
        t->item(i, 0)->setData(Qt::UserRole, s_id);
        //qDebug()<<s_id;
        */
    }
}
void BB_HistoryPage::fillPosTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tablePos->table();
    //qDebug()<<QString("fillPosTable  arr_size=%1").arg(j_arr.count());
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_PositionsPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        BB_HistoryPos pos;
        pos.fromJson(j_el);
        if (pos.invalid())
        {
            qWarning()<<QString("WARNING: invalid pos from j_el: ")<<pos.toFileLine();
            continue;
        }


        /*

        QStringList row_data;
        row_data << APIConfig::fromTimeStamp(j_el.value("updatedTime").toString().toLong());
        row_data << j_el.value("symbol").toString();

        QString act(j_el.value("side").toString().trimmed().toUpper());
        if (act == "SELL") act = "LONG";
        else if (act == "BUY") act = "SHORT";
        row_data << act;
        row_data << j_el.value("qty").toString();

        float p1 = j_el.value("avgEntryPrice").toString().toFloat();
        float p2 = j_el.value("avgExitPrice").toString().toFloat();
        float deviation = -1;
        if (act == "LONG") deviation = float(100)*(p2-p1)/p1;
        else if (act == "SHORT") deviation = float(100)*(p1-p2)/p1;

        row_data << QString("%1 / %2 (%3%)").arg(QString::number(p1, 'f', 2)).arg(QString::number(p2, 'f', 2)).arg(QString::number(deviation, 'f', 1));
        float profit = j_el.value("closedPnl").toString().toFloat();

        float lot_size = j_el.value("qty").toString().toFloat();
        float cm = -1;
        if (act == "LONG") cm = profit - lot_size*(p2-p1);
        else if (act == "SHORT") cm = profit - lot_size*(p1-p2);

        row_data << QString::number(cm, 'f', 4);
        row_data << QString::number(profit, 'f', 3);
        row_data << j_el.value("execType").toString();
        row_data << j_el.value("orderType").toString();
        row_data << j_el.value("leverage").toString();
        */

        LTable::addTableRow(t, pos.toTableRowData());

        if (pos.total_result < -0.1) t->item(i, PROFIT_COL)->setTextColor("#AA0000");
        else if (pos.total_result > 1.1) t->item(i, PROFIT_COL)->setTextColor("#0000DD");
        if (pos.paidFee() < -0.1) t->item(i, COMMISION_COL)->setTextColor("#AA0000");
        else if (pos.paidFee() > 0.1) t->item(i, COMMISION_COL)->setTextColor("#0000DD");
        //if (deviation < -5) t->item(i, DEVIATION_COL)->setTextColor("#AA0000");
        //else if (deviation > 20) t->item(i, DEVIATION_COL)->setTextColor("#0000DD");
        if (pos.isShort()) t->item(i, ACTION_COL)->setTextColor("#FF8C00");
        else if (pos.isLong()) t->item(i, ACTION_COL)->setTextColor("#005500");
        else t->item(i, ACTION_COL)->setTextColor(Qt::red);

        //QString s_id = j_el.value("orderId").toString();
        //t->item(i, 0)->setData(Qt::UserRole, s_id);
        //qDebug()<<s_id;

        checkReceivedRecord(pos);
    }
}

void BB_HistoryPage::viewTablesUpdate()
{
    m_tableOrders->resizeByContents();
    m_tableOrders->searchExec();
    m_tablePos->resizeByContents();
    m_tablePos->searchExec();
}
void BB_HistoryPage::load(QSettings &settings)
{
    BB_BasePage::load(settings);
    m_startDateEdit->setText(settings.value(QString("%1/startDate").arg(objectName()), QString()).toString());
    m_historyDaysCombo->setCurrentIndex(settings.value(QString("%1/historyDaysIndex").arg(objectName()), 0).toInt());
    loadContainers();

    emit signalMsg(QString("CONTAINERS STATE: positions %1,  orders %2").arg(m_posList.count()).arg(m_orderList.count()));
}
void BB_HistoryPage::save(QSettings &settings)
{
    BB_BasePage::save(settings);
    settings.setValue(QString("%1/startDate").arg(objectName()), m_startDateEdit->text());
    settings.setValue(QString("%1/historyDaysIndex").arg(objectName()), m_historyDaysCombo->currentIndex());
}

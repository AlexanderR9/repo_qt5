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
    h_stage(-1),
    m_startDate(QDate()),
    m_polledDays(-1)
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
                else insertPos(pos);
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
                else insertOrder(order);
            }
        }
        else emit signalError(err);
    }
    else emit signalError(QString("pos file [%1] not found").arg(fname));
}
void BB_HistoryPage::insertPos(const BB_HistoryPos &pos)
{
    if (hasPos(pos.uid)) {qWarning()<<QString("insertPos() WARNING - uid [%1] already exists").arg(pos.uid); return;}
    if (m_posList.isEmpty()) {m_posList.append(pos); return;}

    int n = m_posList.count();
    for (int i=0; i<n; i++)
    {
        if (pos.closed_time > m_posList.at(i).closed_time)
        {
            m_posList.insert(i, pos);
            return;
        }
    }
    m_posList.append(pos);
}
int BB_HistoryPage::needPollDays() const
{
    return m_historyDaysCombo->currentText().toInt();
}
void BB_HistoryPage::insertOrder(const BB_HistoryOrder &order)
{
    if (hasOrder(order.uid)) {qWarning()<<QString("insertOrder() WARNING - uid [%1] already exists").arg(order.uid); return;}
    if (m_orderList.isEmpty()) {m_orderList.append(order); return;}

    int n = m_orderList.count();
    for (int i=0; i<n; i++)
    {
        if (order.create_time > m_orderList.at(i).create_time)
        {
            m_orderList.insert(i, order);
            return;
        }
    }
    m_orderList.append(order);
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

    m_tablePos->addSortingData(1, LSearchTableWidgetBox::sdtString);
    m_tablePos->addSortingData(2, LSearchTableWidgetBox::sdtString);
    m_tablePos->addSortingData(3, LSearchTableWidgetBox::sdtNumeric);
    m_tablePos->addSortingData(4, LSearchTableWidgetBox::sdtNumeric);
    m_tablePos->sortingOn();

}
void BB_HistoryPage::initFilterBox()
{
    QGroupBox *filterBox = new QGroupBox(this);
    filterBox->setObjectName("filter_box");
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

    g_lay->addItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding), lay_row+1,0);
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
    if (req_type != m_userSign) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_HistoryPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_HistoryPage: list QJsonValue not found"); return;}

    goExchange(jv.toObject());
}
void BB_HistoryPage::slotGetHistoryState(BB_HistoryState &hs)
{
    hs.reset();
    hs.closed_pos = m_posList.count();
    hs.closed_orders = m_orderList.count();

    foreach (const BB_HistoryOrder &order, m_orderList)
        if (order.isCancelled()) hs.canceled_orders++;

    foreach (const BB_HistoryPos &pos, m_posList)
    {
        hs.paid_commission += pos.paidFee();
        hs.total_pnl += pos.total_result;
    }
}
void BB_HistoryPage::goExchange(const QJsonObject &jresult_obj)
{
    qDebug("");
    qDebug()<<QString("BB_HistoryPage::goExchange  h_stage=%1").arg(h_stage);

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
    m_reqData->uri.clear();
    //viewTablesUpdate();
    qint64 ts1, ts2;
    getTSNextInterval(ts1, ts2);
    if (ts1 < 0) {emit signalError("getting time range"); return;}


    m_reqData->uri = API_PL_URI;
    m_reqData->params.remove("settleCoin");
    m_reqData->params.insert("startTime", QString::number(ts1));
    if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
    else m_reqData->params.remove("endTime");

    //m_reqData->params.insert("cursor", QString::number(0));
    h_stage = hsWaitPos;
}
void BB_HistoryPage::prepareOrdersReq()
{
    qDebug()<<QString("BB_HistoryPage::prepareOrdersReq()");
    m_reqData->uri.clear();

    qint64 ts1, ts2;
    getTSNextInterval(ts1, ts2);
    if (ts1 < 0) {emit signalError("getting time range"); return;}

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
    emit signalMsg("#######################FINISHED!#############################");
    m_reqData->uri.clear();
    viewTablesUpdate();
}
void BB_HistoryPage::nextTSRequest()
{
    qint64 ts1, ts2;
    getTSNextInterval(ts1, ts2);
    m_reqData->params.insert("startTime", QString::number(ts1));
    if (ts2 > 0) m_reqData->params.insert("endTime", QString::number(ts2));
    else m_reqData->params.remove("endTime");

    sendRequest(MAX_ORDERS_PAGE);
}
void BB_HistoryPage::waitPos(const QJsonObject &jresult_obj)
{
    const QJsonArray &j_arr = jresult_obj.value("list").toArray();
    qDebug()<<QString("BB_HistoryPage::waitPos  j_arr %1, m_polledDays=%2").arg(j_arr.count()).arg(m_polledDays);
    if (j_arr.count() >= MAX_ORDERS_PAGE)
        signalError(QString("BB_HistoryPage::waitPos:  j_arr size over MAX_ORDERS_PAGE(%1)").arg(MAX_ORDERS_PAGE));

    fillPosTable(j_arr);
    if (m_polledDays < 0 || m_polledDays >= needPollDays())
    {
        emit signalMsg("getting positions finished");
        h_stage = hsFinished;
        goExchange(jresult_obj);
    }
    else nextTSRequest();
}
void BB_HistoryPage::waitOrders(const QJsonObject &jresult_obj)
{
    const QJsonArray &j_arr = jresult_obj.value("list").toArray();
    qDebug()<<QString("BB_HistoryPage::waitOrders  j_arr %1, m_polledDays=%2").arg(j_arr.count()).arg(m_polledDays);
    if (j_arr.count() >= MAX_ORDERS_PAGE)
        signalError(QString("BB_HistoryPage::waitOrders:  j_arr size over MAX_ORDERS_PAGE(%1)").arg(MAX_ORDERS_PAGE));

    fillOrdersTable(j_arr);
    if (m_polledDays < 0 || m_polledDays >= needPollDays())
    {
        emit signalMsg("getting orders finished");
        h_stage = hsGetPos;
        m_polledDays = 0;
        goExchange(jresult_obj);
    }
    else nextTSRequest();
}
void BB_HistoryPage::updateDataPage(bool force)
{
    if (!force)
    {
        loadTablesByContainers();
        return;
    }

    if (!updateTimeOver(force)) return;

    if (m_tablePos) m_tablePos->removeAllRows();
    if (m_tableOrders) m_tableOrders->removeAllRows();
    h_stage = hsGetOrders;


    m_polledDays = -1;
    m_startDate = QDate::fromString(m_startDateEdit->text().trimmed(), APIConfig::userDateMask());
    if (!m_startDate.isValid())
    {
        emit signalError(QString("BB_HistoryPage: invalid start date"));
        return;
    }
    else m_polledDays = 0;

    //qDebug("\n\n\n\n\n\n");
    goExchange(QJsonObject());
}
void BB_HistoryPage::getTSNextInterval(qint64 &ts1, qint64 &ts2)
{
    ts1 = ts2 = -1;
    if (m_polledDays < 0) {qWarning("BB_HistoryPage::getTSNextInterval WARNING m_polledDays < 0"); return;}

    QDate cur_d(QDate::currentDate());

    QDate d1 = m_startDate;
    if (m_polledDays > 0) d1 = m_startDate.addDays(m_polledDays);
    ts1 = APIConfig::toTimeStamp(d1.day(), d1.month(), d1.year());
    m_polledDays += daysSeparator();

    QDate d2 = d1.addDays(daysSeparator());
    if (needPollDays() < daysSeparator()) {d2 = d1.addDays(needPollDays()); m_polledDays = needPollDays();}

    qDebug()<<QString("BB_SpotHistoryPage::getTSNextInterval  %1 / %2").arg(d1.toString(APIConfig::userDateMask())).
              arg((d2 >= cur_d) ? "-1" : d2.toString(APIConfig::userDateMask()));

    if (d2 >= cur_d) m_polledDays = -1;
    else ts2 = APIConfig::toTimeStamp(d2.day(), d2.month(), d2.year());
}
void BB_HistoryPage::fillOrdersTable(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) return;

    QTableWidget *t = m_tableOrders->table();
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
    }
}
void BB_HistoryPage::fillPosTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tablePos->table();
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

        LTable::addTableRow(t, pos.toTableRowData());

        if (pos.total_result < -0.1) t->item(i, PROFIT_COL)->setTextColor("#AA0000");
        else if (pos.total_result > 1.1) t->item(i, PROFIT_COL)->setTextColor("#0000DD");
        if (pos.paidFee() < -0.1) t->item(i, COMMISION_COL)->setTextColor("#AA0000");
        else if (pos.paidFee() > 0.1) t->item(i, COMMISION_COL)->setTextColor("#0000DD");
        if (pos.isShort()) t->item(i, ACTION_COL)->setTextColor("#FF8C00");
        else if (pos.isLong()) t->item(i, ACTION_COL)->setTextColor("#005500");
        else t->item(i, ACTION_COL)->setTextColor(Qt::red);

        checkReceivedRecord(pos);
    }
}
void BB_HistoryPage::viewTablesUpdate()
{
    if (m_tableOrders)
    {
        m_tableOrders->resizeByContents();
        m_tableOrders->searchExec();
    }
    if (m_tablePos)
    {
        m_tablePos->resizeByContents();
        m_tablePos->searchExec();
    }
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
void BB_HistoryPage::loadTablesByContainers()
{
    if (m_tablePos->table()->rowCount() > 0 || m_tableOrders->table()->rowCount() > 0) return;

    //load positions
    QTableWidget *t = m_tablePos->table();
    for (int i=0; i<m_posList.count(); i++)
    {
        const BB_HistoryPos &pos = m_posList.at(i);
        LTable::addTableRow(t, pos.toTableRowData());

        if (pos.total_result < -0.1) t->item(i, PROFIT_COL)->setTextColor("#AA0000");
        else if (pos.total_result > 1.1) t->item(i, PROFIT_COL)->setTextColor("#0000DD");
        if (pos.paidFee() < -0.1) t->item(i, COMMISION_COL)->setTextColor("#AA0000");
        else if (pos.paidFee() > 0.1) t->item(i, COMMISION_COL)->setTextColor("#0000DD");
        if (pos.isShort()) t->item(i, ACTION_COL)->setTextColor("#FF8C00");
        else if (pos.isLong()) t->item(i, ACTION_COL)->setTextColor("#005500");
        else t->item(i, ACTION_COL)->setTextColor(Qt::red);
    }
    m_tablePos->searchExec();

    //load orders
    t = m_tableOrders->table();
    for (int i=0; i<m_orderList.count(); i++)
    {
        const BB_HistoryOrder &order = m_orderList.at(i);
        LTable::addTableRow(t, order.toTableRowData());
    }
    m_tableOrders->searchExec();
}

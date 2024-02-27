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

//Запросить историю заказов.
//Поскольку создание/отмена заказа является асинхронным, данные, возвращаемые из этой конечной точки, могут задерживаться.
#define API_ORDERS_URI      QString("v5/order/history")

//Записи выполнения запросов пользователей, отсортированные по execTime в порядке убывания.
//Однако для классического спота они сортируются по execId в порядке убывания.
#define API_EXEC_ORDERS_URI      QString("v5/execution/list")

//Запросить закрытые записи о прибылях и убытках пользователя
#define API_PL_URI          QString("v5/position/closed-pnl")

#define MAX_ORDERS_PAGE             50



//BB_HistoryPage
BB_HistoryPage::BB_HistoryPage(QWidget *parent)
    :BB_BasePage(parent, 20, rtHistory),
      m_table(NULL)
{
    setObjectName("history_page");
    init();

    m_reqData->params.insert("category", "linear");
    //m_reqData->params.insert("settleCoin", "USDT");

}
void BB_HistoryPage::init()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setTitle("Current positions");
    m_table->vHeaderHide();
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_table->setHeaderLabels(tableHeaders());
    m_table->resizeByContents();

    m_table->addSortingData(0, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(1, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(2, LSearchTableWidgetBox::sdtString);
    m_table->addSortingData(3, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(4, LSearchTableWidgetBox::sdtString);
    //m_table->addSortingData(5, LSearchTableWidgetBox::sdtNumeric);
    m_table->addSortingData(6, LSearchTableWidgetBox::sdtString);
    m_table->sortingOn();


    QGroupBox *filterBox = new QGroupBox(this);
    filterBox->setTitle("Filter parameters");

    h_splitter->addWidget(m_table);
    h_splitter->addWidget(filterBox);
}
QStringList BB_HistoryPage::tableHeaders() const
{
    QStringList list;
    list << "Create time" << "Ticker" << "Action" << "Volume" << "Type" << "Price" << "Status";
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
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_HistoryPage: j_arr QJsonArray is empty"); return;}

    fillOrdersTable(j_arr);
    //return;

    //next request (orders: next page)
    QString next_cursor = jv.toObject().value("nextPageCursor").toString().trimmed();
    if (!next_cursor.isEmpty())
    {
        qDebug()<<QString("nextPageCursor: [%1]").arg(next_cursor);
        m_reqData->params.insert("cursor", next_cursor);
        sendRequest(MAX_ORDERS_PAGE);
    }

}
void BB_HistoryPage::updateDataPage(bool force)
{
    if (!updateTimeOver(force)) return;

    m_table->removeAllRows();

    qint64 ts1 = APIConfig::toTimeStamp(17, 02, 2024);
    qint64 ts2 = APIConfig::toTimeStamp(9, 12, 2023);
    qDebug()<<QString("ts=%1").arg(ts1);


    //get history request
    m_reqData->uri = API_ORDERS_URI;
    //m_reqData->uri = API_EXEC_ORDERS_URI;

    m_reqData->params.insert("settleCoin", "USDT");
    m_reqData->params.insert("startTime", QString::number(ts1));
    //m_reqData->params.insert("endTime", QString::number(ts2));
    //m_reqData->params.insert("orderStatus", "Cancelled");
    m_reqData->params.insert("cursor", QString::number(0));
    sendRequest(MAX_ORDERS_PAGE);

}
void BB_HistoryPage::fillOrdersTable(const QJsonArray &j_arr)
{
    qDebug()<<QString("fillOrdersTable  arr_size=%1").arg(j_arr.count());
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_PositionsPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

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

        LTable::addTableRow(m_table->table(), row_data);

        QString s_id = j_el.value("orderId").toString();
        qDebug()<<s_id;

    }

    m_table->resizeByContents();
    m_table->searchExec();

}


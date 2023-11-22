#include "apiorderspage.h"
#include "ltable.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
//#include <QSplitter>
#include <QDebug>
//#include <QLabel>
//#include <QDate>

#define KIND_COL            0


//APIOrdersPage
APIOrdersPage::APIOrdersPage(QWidget *parent)
    :APITablePageBase(parent)
{
    setObjectName("api_orders_page");
    m_userSign = aptOrders;

    QStringList headers;
    headers << "Kind" << "Company" << "Ticker" << "Paper type" << "Currency" << "Date" << "Lots" << "Price";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Orders list");
    m_tableBox->setObjectName("table_box");

    //reinitWidgets();
    //initFilterBox();
    //connect(m_tableBox, SIGNAL(signalSearched()), this, SLOT(slotSearched()));


}
void APIOrdersPage::slotLoadOrders(const QJsonObject &j_obj)
{
    qDebug("APIOrdersPage::slotLoadOrders");

    m_orders.clear();
    if (j_obj.isEmpty()) return;

    const QJsonValue &j_orders = j_obj.value("orders");
    if (!j_orders.isArray()) return;

    const QJsonArray &j_arr = j_orders.toArray();
    int n = j_arr.count();
    qDebug()<<QString(" APIEventsPage::slotLoadOrders arr_size %1").arg(n);
    for (int i=0; i<n; i++)
    {
        OrderData order;
        order.fromJson(j_arr.at(i));
        if (!order.invalid()) m_orders.append(order);
    }

    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_orders.count()));
    reloadTableByData();

}
void APIOrdersPage::reloadTableByData()
{
    resetPage();
    if (m_orders.isEmpty())
    {
        emit signalMsg(QString("Orders list is empty!"));
        return;
    }

    foreach (const OrderData &rec, m_orders)
    {
        QPair<QString, QString> pair;
        emit signalGetPaperInfoByFigi(rec.uid, pair);

        emit signalGetTickerByFigi(rec.uid, pair.second);
        if (pair.second.length() < 2) pair.second = "---"; //ticker
        if (pair.first.length() < 5) pair.first = "---"; //company name

        addRowRecord(rec, pair);
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#D3D3D3", "#2F4F4F");
    m_tableBox->searchExec();
}
void APIOrdersPage::addRowRecord(const OrderData &rec, const QPair<QString, QString> &info)
{
    QString p_type;
    emit signalGetPaperTypeByUID(rec.uid, p_type);

    QStringList row_data;
    row_data << rec.type << info.first << info.second << p_type << rec.currency;
    row_data << rec.time.toString(QString("%1  %2").arg(InstrumentBase::userDateMask()).arg(InstrumentBase::userTimeMask()));
    row_data << rec.strLots() << QString::number(rec.price, 'f', 1);

    LTable::addTableRow(m_tableBox->table(), row_data);

    int l_row = m_tableBox->table()->rowCount() - 1;
    if (rec.type.toLower().contains("sell")) m_tableBox->table()->item(l_row, KIND_COL)->setTextColor(Qt::darkRed);
    else if (rec.type.toLower().contains("buy")) m_tableBox->table()->item(l_row, KIND_COL)->setTextColor(Qt::darkGreen);
    //LTable::setTableRowColor(m_tableBox->table(), l_row, row_color);
}




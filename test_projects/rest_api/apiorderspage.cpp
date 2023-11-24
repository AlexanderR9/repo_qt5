#include "apiorderspage.h"
#include "ltable.h"
#include "apicommonsettings.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
//#include <QSplitter>
#include <QDebug>
#include <QPoint>
#include <QMenu>

#define KIND_COL            0
#define NAME_COL            1
#define TICKER_COL          2


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
    m_tableBox->table()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_tableBox->table(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
}
void APIOrdersPage::slotContextMenu(QPoint p)
{
    qDebug()<<QString("APIOrdersPage::slotContextMenu  %1/%2").arg(p.x()).arg(p.y());
    QTableWidget *t = m_tableBox->table();
    if (LTable::selectedRows(t).count() != 1) return;

    QMenu *menu = new QMenu(this);
    QIcon act_icon(QString(":/icons/images/ball_red.svg"));
    QAction *del_act = new QAction(act_icon, QString("Cancel order"), this);
    menu->addAction(del_act);
    connect(del_act, SIGNAL(triggered()), this, SLOT(slotCancelOrder()));

    // Вызываем контекстное меню
    menu->popup(m_tableBox->table()->viewport()->mapToGlobal(p));

}
void APIOrdersPage::slotCancelOrder()
{
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    QString id = t->item(row, 1)->data(Qt::UserRole).toString();
    bool is_stop = t->item(row, 0)->data(Qt::UserRole).toBool();
    qDebug()<<QString("slotCancelOrder  order_id=[%1],  is_stop=[%2]").arg(id).arg(is_stop);
    QString p_name = QString("%1 / %2").arg(t->item(row, NAME_COL)->text()).arg(t->item(row, TICKER_COL)->text());

    QString src = QString();
    foreach(const QString &v, api_commonSettings.services)
    {
        if (v.toLower().contains("cancel"))
        {
            if (is_stop)
            {
                if (v.toLower().contains("stop")) {src = v; break;}
            }
            else
            {
                if (!v.toLower().contains("stop")) {src = v; break;}
            }
        }
    }

    if (src.isEmpty())
    {
        emit signalError(QString("Not found API metod for cancel order, id=[%1]").arg(id));
        return;
    }

    QStringList req_data;
    req_data << src << id << p_name;
    emit signalMsg(QString("Try cancel order id: [%1], paper: [%2]").arg(id).arg(p_name));
    emit signalCancelOrder(req_data);
}
void APIOrdersPage::clearData()
{
    qDeleteAll(m_orders);
    m_orders.clear();
}
void APIOrdersPage::clearDataByKind(bool is_stop)
{
    qDebug()<<QString("APIOrdersPage::clearDataByKind  n_before=%1").arg(m_orders.count());
    //look container
    int n = m_orders.count();
    for (int i=n-1; i>=0; i--)
    {
        if (m_orders.at(i)->is_stop == is_stop)
        {
            OrderData *order = m_orders.takeAt(i);
            delete order;
            order = NULL;
        }
    }
    qDebug()<<QString("APIOrdersPage::clearDataByKind  n_after=%1").arg(m_orders.count());

    //look table
    QTableWidget *t = m_tableBox->table();
    n = t->rowCount();
    for (int i=n-1; i>=0; i--)
    {
        if (t->item(i, 0)->data(Qt::UserRole).toBool() == is_stop)
            t->removeRow(i);
    }
}
void APIOrdersPage::slotLoadOrders(const QJsonObject &j_obj)
{
    clearDataByKind(false);
    if (j_obj.isEmpty()) return;

    const QJsonValue &j_orders = j_obj.value("orders");
    if (!j_orders.isArray()) return;

    const QJsonArray &j_arr = j_orders.toArray();
    for (int i=0; i<j_arr.count(); i++)
    {
        OrderData *order = new OrderData();
        order->fromJson(j_arr.at(i));
        if (!order->invalid()) m_orders.append(order);
    }
    emit signalMsg(QString("loaded validity ORDER records: %1/%2 \n").arg(j_arr.count()).arg(m_orders.count()));
    updateTableByData(false);
}
void APIOrdersPage::slotLoadStopOrders(const QJsonObject &j_obj)
{
    clearDataByKind(true);
    if (j_obj.isEmpty()) return;

    const QJsonValue &j_orders = j_obj.value("stopOrders");
    if (!j_orders.isArray()) return;

    const QJsonArray &j_arr = j_orders.toArray();
    for (int i=0; i<j_arr.count(); i++)
    {
        StopOrderData *order = new StopOrderData();
        order->fromJson(j_arr.at(i));
        if (!order->invalid()) m_orders.append(order);
    }
    emit signalMsg(QString("loaded validity STOP_ORDER records: %1/%2 \n").arg(j_arr.count()).arg(m_orders.count()));
    updateTableByData(true);
}
void APIOrdersPage::updateTableByData(bool is_stop)
{
    if (m_orders.isEmpty())
    {
        emit signalMsg(QString("Orders list is empty!"));
        return;
    }

    foreach (const OrderData *rec, m_orders)
    {
        if (!rec) continue;
        if (rec->is_stop != is_stop) continue;

        QPair<QString, QString> pair;
        emit signalGetPaperInfoByFigi(rec->uid, pair);

        emit signalGetTickerByFigi(rec->uid, pair.second);
        if (pair.second.length() < 2) pair.second = "---"; //ticker
        if (pair.first.length() < 5) pair.first = "---"; //company name

        addRowRecord(rec, pair);
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#D3D3D3", "#2F4F4F");
    m_tableBox->searchExec();
}
void APIOrdersPage::addRowRecord(const OrderData *rec, const QPair<QString, QString> &info)
{
    QString p_type;
    emit signalGetPaperTypeByUID(rec->uid, p_type);
    float real_price = rec->price;
    if (p_type == "bond" && !rec->is_stop)
    {
        float b_nominal = 0;
        emit signalGetBondNominalByUID(rec->uid, b_nominal);
        if (b_nominal > 0) real_price = (b_nominal * rec->price / float(100));
    }

    QStringList row_data;
    row_data << rec->type << info.first << info.second << p_type << rec->currency;
    row_data << rec->time.toString(QString("%1  %2").arg(InstrumentBase::userDateMask()).arg(InstrumentBase::userTimeMask()));
    quint8 p_prec = ((rec->price < 10) ? 2 : 1);
    row_data << rec->strLots() << QString::number(real_price, 'f', p_prec);

    QTableWidget *t = m_tableBox->table();
    LTable::addTableRow(t, row_data);
    int l_row = t->rowCount() - 1;
    t->item(l_row, 0)->setData(Qt::UserRole, rec->is_stop);
    t->item(l_row, 1)->setData(Qt::UserRole, rec->order_id);
    if (rec->type.toLower().contains("sell")) t->item(l_row, KIND_COL)->setTextColor(Qt::darkRed);
    else if (rec->type.toLower().contains("buy")) t->item(l_row, KIND_COL)->setTextColor(Qt::darkGreen);
    if (rec->is_stop) LTable::setTableRowColor(t, l_row, QColor("#FEEBFE"));
}



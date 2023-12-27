#include "apiprofitabilitypage.h"
#include "ltable.h"
#include "apicommonsettings.h"
#include "instrument.h"
#include "apitradedialog.h"
#include "lcommonsettings.h"

#include <QTableWidget>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QTimer>

#define PROFITABILITY_LIMIT             1.35
#define PROFITABILITY_LIMIT2            1.12
#define PROFITABILITY_LIMIT_MIN         0.9

#define NAME_COL            0
#define TICKER_COL          1
#define PRICE_COL           4
#define COUPON_COL          3
#define DATE_COL            2



//APIProfitabilityPage
APIProfitabilityPage::APIProfitabilityPage(QWidget *parent)
    :APITablePageBase(parent),
      m_tick(0)
{
    setObjectName("profitability_page");
    m_userSign = aptProfitability;
    m_buyData.reset();

    QStringList headers;
    headers << "Company" << "Ticker" << "Finish date" << "Coupon" << "Price/Nominal" << "Finish coupon" << "Finish profit" << "Profitability_M, %";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Profitability by finish date");
    m_tableBox->sortingOn();
    m_tableBox->table()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableBox->table(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(slotResizeTimer()));
    t->start(1000);
}
void APIProfitabilityPage::slotResizeTimer()
{
    if (m_tick < 0) return;
    m_tick = -1;
    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->searchExec();
}
void APIProfitabilityPage::slotContextMenu(QPoint p)
{
   // qDebug()<<QString("APIProfitabilityPage::slotContextMenu  %1/%2").arg(p.x()).arg(p.y());
    QTableWidget *t = m_tableBox->table();
    if (LTable::selectedRows(t).count() != 1) return;

    QMenu *menu = new QMenu(this);
    QIcon act_icon(QString(":/icons/images/ball_green.svg"));
    QAction *del_act = new QAction(act_icon, QString("Buy order"), this);
    menu->addAction(del_act);
    connect(del_act, SIGNAL(triggered()), this, SLOT(slotBuyOrder()));

    // Вызываем контекстное меню
    menu->popup(m_tableBox->table()->viewport()->mapToGlobal(p));
}
void APIProfitabilityPage::slotBuyOrder()
{
    m_buyData.reset();
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
  //  qDebug()<<QString("APIProfitabilityPage::slotBuyOrder()  sel row %1").arg(row);
    m_buyData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    m_buyData.kind = "buy";
    QString p_name = QString("%1 / %2").arg(t->item(row, NAME_COL)->text()).arg(t->item(row, TICKER_COL)->text());

    //prepare dialog
    TradeOperationData t_data(totBuyLimit);
    t_data.company = p_name;
    t_data.paper_type = "bond";
    t_data.finish_date = t->item(row, DATE_COL)->text();
    t_data.price = getCurrentPrice(row);
    t_data.coupon = curAccumulatedCoupon(row);
    t_data.lots = 1;
    APITradeDialog d(t_data, this);
    d.exec();
    if (!d.isApply())
    {
        //qDebug()<<QString("APIProfitabilityPage::slotBuyOrder() operation was canceled");
        emit signalMsg("operation was canceled!");
        return;
    }
    m_buyData.lots = t_data.lots;
    m_buyData.price = t_data.price;
    emit signalMsg(QString("Try set BUY order id: [%1], paper: [%2]").arg(m_buyData.uid).arg(p_name));
    emit signalBuyOrder(m_buyData);
}
float APIProfitabilityPage::getCurrentPrice(int row) const
{
    if (row < 0 || row >= m_tableBox->table()->rowCount()) return -1;
    QString s_price = m_tableBox->table()->item(row, PRICE_COL)->text();

    int pos = s_price.indexOf("/");
    if (pos > 0)
    {
        s_price = s_price.left(pos);
        bool ok;
        float p = s_price.toFloat(&ok);
        if (ok && p > 0) return p;
    }
    qWarning("APIProfitabilityPage::getCurrentPrice WARNING invalid current price");
    return -1;
}
float APIProfitabilityPage::curAccumulatedCoupon(int row) const
{
    if (row < 0 || row >= m_tableBox->table()->rowCount()) return -1;
    QString s_price = m_tableBox->table()->item(row, COUPON_COL)->text();

    int pos = s_price.indexOf("/");
    if (pos > 0)
    {
        s_price = s_price.left(pos);
        bool ok;
        float p = s_price.toFloat(&ok);
        if (ok && p > 0) return p;
    }
    qWarning("APIProfitabilityPage::curAccumulatedCoupon WARNING invalid current AccumulatedCoupon");
    return -1;
}
void APIProfitabilityPage::slotRecalcProfitability(const BondDesc &bond_rec, float price)
{
    if (bond_rec.invalid() || price <= 0) return;
    //qDebug()<<QString("APIProfitabilityPage::slotRecalcProfitability  [%1]").arg(bond_rec.isin);

    const BCoupon *c_rec = NULL;
    emit signalGetCouponRec(bond_rec.figi, c_rec);
    if (!c_rec ) {qWarning()<<QString("APIProfitabilityPage::slotRecalcProfitability WARNING - not found coupon record by figi [%1]").arg(bond_rec.figi); return;}
    if (c_rec->invalid()) return;

    QStringList row_data;
    row_data << bond_rec.name << bond_rec.isin;
    row_data << QString("%1 (%2)").arg(bond_rec.finish_date.toString(InstrumentBase::userDateMask())).arg(bond_rec.daysToFinish());
    float accumulated = c_rec->size*float(c_rec->period-c_rec->daysTo())/float(c_rec->period);
    row_data << QString("%1/%2/%3").arg(QString::number(accumulated, 'f', 2)).arg(QString::number(c_rec->size-accumulated, 'f', 2)).arg(QString::number(c_rec->size, 'f', 2));
    row_data << QString("%1/%2 (%3)").arg(QString::number(price, 'f', 1)).arg(QString::number(bond_rec.nominal, 'f', 1)).arg(QString::number(bond_rec.nominal-price, 'f', 1));
    float c_finish = c_rec->daySize()*bond_rec.daysToFinish();
    row_data << QString::number(c_finish, 'f', 2);
    float p_finish = c_finish + (bond_rec.nominal-price);
    row_data << QString::number(p_finish, 'f', 2);
    float v_month = ((p_finish*100/(price+accumulated))/float(bond_rec.daysToFinish()))*30;
    row_data << QString("%1/%2").arg(QString::number(v_month, 'f', 3)).arg(QString::number(v_month*12, 'f', 1));

    //sync by table
    syncTableData(bond_rec, row_data, v_month);
}
void APIProfitabilityPage::syncTableData(const BondDesc &bond_rec, const QStringList &row_data, float v_month)
{
    QTableWidget *t = m_tableBox->table();
    int row = indexOf(bond_rec.figi);

    //update table data
    if (row >= 0)
    {
        LTable::setTableRow(row, t, row_data);
    }
    else
    {
        row = t->rowCount();
        LTable::addTableRow(t, row_data);
        t->item(row, 0)->setData(Qt::UserRole, bond_rec.figi);
        t->item(row, 1)->setData(Qt::UserRole, bond_rec.uid);
    }

    //set cells color
    if (v_month > PROFITABILITY_LIMIT)
        t->item(row, priceCol())->setTextColor(Qt::blue);
    else if (v_month > PROFITABILITY_LIMIT2)
        t->item(row, priceCol())->setTextColor(Qt::darkGreen);
    else if (v_month < PROFITABILITY_LIMIT_MIN)
        t->item(row, priceCol())->setTextColor(Qt::lightGray);

    if (bond_rec.nominal < 900)
        t->item(row, PRICE_COL)->setTextColor("#8B0000");

    m_tick++;
    sortByProfitability(row, v_month);
}
void APIProfitabilityPage::sortByProfitability(int row, float value)
{
    //qDebug()<<QString("sortByProfitability row=%1  value=%2").arg(row).arg(value);
    if (!lCommonSettings.paramValue("sort_profit").toBool()) return;
    QTableWidget *t = m_tableBox->table();
    if (t->rowCount() < 2) return;
    if (row < 0 || row >= t->rowCount()) return;

    for (int i=0; i<t->rowCount(); i++)
    {
        if (i >= row) break;

        QString s_price = t->item(i, priceCol())->text().trimmed();
        int pos = s_price.indexOf("/");
        if (pos > 0)
        {
            if (value > s_price.left(pos).trimmed().toFloat())
            {
                //qDebug()<<QString("need replace: i=%1, vprof=%2").arg(i).arg(s_price.left(pos).trimmed().toFloat());
                LTable::shiftTableRow(t, row, i-row);
                break;
            }
        }
    }
}
int APIProfitabilityPage::indexOf(const QString &figi) const
{
    int n = m_tableBox->table()->rowCount();
    for (int i=0; i<n; i++)
    {
        QString u_data = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toString();
        if (u_data == figi) return i;
    }
    return -1;
}


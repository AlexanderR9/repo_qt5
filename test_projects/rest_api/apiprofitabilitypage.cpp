#include "apiprofitabilitypage.h"
#include "ltable.h"
#include "apicommonsettings.h"
#include "instrument.h"
#include "apitradedialog.h"

#include <QTableWidget>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QTimer>

#define PROFITABILITY_LIMIT             1.4
#define PROFITABILITY_LIMIT2            1.15
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
    qDebug()<<QString("APIProfitabilityPage::slotContextMenu  %1/%2").arg(p.x()).arg(p.y());
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
    qDebug()<<QString("APIProfitabilityPage::slotBuyOrder()  sel row %1").arg(row);
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
        qDebug()<<QString("APIProfitabilityPage::slotBuyOrder() operation was canceled");
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
        if (ok & p > 0) return p;
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
        if (ok & p > 0) return p;
    }
    qWarning("APIProfitabilityPage::curAccumulatedCoupon WARNING invalid current AccumulatedCoupon");
    return -1;
}
void APIProfitabilityPage::slotRecalcProfitability(const BondDesc &bond_rec, float price)
{
    if (bond_rec.invalid() || price <= 0) return;
    qDebug("APIProfitabilityPage::slotRecalcProfitability");

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
    int row = -1;
    int n = m_tableBox->table()->rowCount();
    if (n > 0)
    {
        for (int i=0; i<n-1; i++)
        {
            QString u_data = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toString();
            if (u_data == bond_rec.figi) {row = i; break;}
        }
    }

    //update table data
    if (row >= 0)
    {
        LTable::setTableRow(row, m_tableBox->table(), row_data);
    }
    else
    {
        LTable::addTableRow(m_tableBox->table(), row_data);
        m_tableBox->table()->item(n, 0)->setData(Qt::UserRole, bond_rec.figi);
        m_tableBox->table()->item(n, 1)->setData(Qt::UserRole, bond_rec.uid);
        row = n;
    }

    if (v_month > PROFITABILITY_LIMIT)
        m_tableBox->table()->item(row, priceCol())->setTextColor(Qt::blue);
    else if (v_month > PROFITABILITY_LIMIT2)
        m_tableBox->table()->item(row, priceCol())->setTextColor(Qt::darkGreen);
    else if (v_month < PROFITABILITY_LIMIT_MIN)
        m_tableBox->table()->item(row, priceCol())->setTextColor(Qt::lightGray);

    if (bond_rec.nominal < 900)
        m_tableBox->table()->item(row, PRICE_COL)->setTextColor("#8B0000");

    m_tick++;
}


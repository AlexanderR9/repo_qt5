#include "apiprofitabilitypage.h"
#include "ltable.h"
#include "apicommonsettings.h"
#include "instrument.h"
#include "apitradedialog.h"
#include "lcommonsettings.h"
#include "lstring.h"

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
    connect(m_tableBox->table(), SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotTableClicked(QTableWidgetItem*)));


    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(slotResizeTimer()));
    t->start(1000);

    //init asset info box
    initAssetInfoBox();

}
void APIProfitabilityPage::slotTableClicked(QTableWidgetItem *item)
{
    if (!item) return;
    int row = item->row();
    QList<int> sel_list(LTable::selectedRows(m_tableBox->table()));
    if (sel_list.contains(row))
    {
        QString ticker = m_tableBox->table()->item(row, TICKER_COL)->text().trimmed();
        emit signalUpdateInfoBox(ticker);
    }
}
void APIProfitabilityPage::slotGetCurrentPriceBySelected(float &cur_price)
{
    cur_price = -1;
    QList<int> sel_list(LTable::selectedRows(m_tableBox->table()));
    if (!sel_list.isEmpty())
    {
        QString s_price = m_tableBox->table()->item(sel_list.first(), PRICE_COL)->text().trimmed();
        int pos = s_price.indexOf("/");
        if (pos > 0) cur_price = s_price.left(pos).trimmed().toFloat();
    }
}
void APIProfitabilityPage::initAssetInfoBox()
{
    m_filterBox->setTitle("Asset information");
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QVBoxLayout *v_lay = new QVBoxLayout(0);
    v_lay->setSpacing(0);
    m_filterBox->setLayout(v_lay);

    AssetInfoWidget *aiw = new AssetInfoWidget(m_filterBox);
    v_lay->addWidget(aiw);

    connect(m_tableBox->table(), SIGNAL(itemSelectionChanged()), aiw, SLOT(slotReset()));
    connect(this, SIGNAL(signalUpdateInfoBox(const QString&)), aiw, SLOT(slotRunUpdate(QString)));
    connect(aiw, SIGNAL(signalGetBondRiskByTicker(const QString&, QString&)), this, SIGNAL(signalGetBondRiskByTicker(const QString&, QString&)));
    connect(aiw, SIGNAL(signalGetPaperCountByTicker(const QString&, int&, float&)), this, SIGNAL(signalGetPaperCountByTicker(const QString&, int&, float&)));
    connect(aiw, SIGNAL(signalGetCouponInfoByTicker(const QString&, QDate&, float&)), this, SIGNAL(signalGetCouponInfoByTicker(const QString&, QDate&, float&)));
    connect(aiw, SIGNAL(signalGetEventsHistoryByTicker(const QString&, QStringList&)), this, SIGNAL(signalGetEventsHistoryByTicker(const QString&, QStringList&)));
    connect(aiw, SIGNAL(signalGetCurrentPriceBySelected(float&)), this, SLOT(slotGetCurrentPriceBySelected(float&)));


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
    if (bond_rec.daysToFinish() < 30)
        LTable::setTableRowColor(t, row, QString("#FFEFD5"));

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

///////////////AssetInfoWidget////////////////
AssetInfoWidget::AssetInfoWidget(QWidget *parent)
    :QWidget(parent)
{
        setupUi(this);
        setObjectName(QString("asset_info_widget"));

        init();
        refreshTable();
}
void AssetInfoWidget::init()
{
    LTable::fullClearTable(historyTable);

    QStringList headers;
    headers << "Date" << "Operation" << "Count" << "Result";
    LTable::setTableHeaders(historyTable, headers);
}
void AssetInfoWidget::refreshTable()
{
    LTable::resizeTableContents(historyTable);
    rowsLabel->setText(QString("Rows: %1").arg(historyTable->rowCount()));
}
void AssetInfoWidget::slotReset()
{
    bagLineEdit->clear();
    couponDateLineEdit->clear();
    couponSizeLineEdit->clear();
    priceLineEdit->clear();
    riskLineEdit->clear();
    LTable::removeAllRowsTable(historyTable);
    refreshTable();
}
void AssetInfoWidget::slotRunUpdate(const QString &ts)
{
    QString ticker(ts.trimmed());

    QString risk;
    emit signalGetBondRiskByTicker(ticker, risk);
    updateRisk(risk);

    int n_papers = 0;
    float cur_profit = 0;
    emit signalGetPaperCountByTicker(ticker, n_papers, cur_profit);
    updateBag(n_papers, cur_profit);

    QDate d;
    float c_size = 0;
    emit signalGetCouponInfoByTicker(ticker, d, c_size);
    updateCoupon(d, c_size);

    QStringList events_data;
    emit signalGetEventsHistoryByTicker(ticker, events_data);
    updateHistory(events_data);

    float p = 0;
    emit signalGetCurrentPriceBySelected(p);
    updatePrice(p);
}
void AssetInfoWidget::updateRisk(QString s)
{
    riskLineEdit->setText(s);

    QPalette palette(riskLineEdit->palette());
    palette.setColor (QPalette :: Text, Qt::red);
    if (s == "LOW" || s == "MID") palette.setColor (QPalette :: Text, Qt::darkGreen);
    else if (s.contains("?")) palette.setColor (QPalette :: Text, Qt :: gray);
    riskLineEdit->setPalette(palette);
}
void AssetInfoWidget::updateBag(int n_papers, float cur_profit)
{
    QPalette palette(riskLineEdit->palette());
    palette.setColor(QPalette::Text, Qt::black);

    if (n_papers > 0)
    {
        bagLineEdit->setText(QString("%1 (profit=%2)").arg(n_papers).arg(QString::number(cur_profit, 'f', 1)));
        if (cur_profit < 0) palette.setColor (QPalette::Text, Qt::red);
    }
    else
    {
       bagLineEdit->setText("---");
       palette.setColor(QPalette::Text, Qt::gray);
    }

    bagLineEdit->setPalette(palette);
}
void AssetInfoWidget::updateCoupon(const QDate &d, float size)
{
    if (d.isValid())
    {
        int day_to = QDate::currentDate().daysTo(d);
        couponDateLineEdit->setText(QString("%1 (%2 days)").arg(d.toString(InstrumentBase::userDateMask())).arg(day_to));
        couponSizeLineEdit->setText(QString::number(size, 'f', 2));
    }
    else couponSizeLineEdit->setText("-1");
}
void AssetInfoWidget::updatePrice(float p)
{
    QPalette palette(priceLineEdit->palette());
    priceLineEdit->setText(QString::number(p, 'f', 2));

    if (p > 0 && historyTable->rowCount() > 0)
    {
        QString s_price = historyTable->item(0, 3)->text().trimmed();
        int pos = s_price.indexOf("/");
        float t_price = -1;
        if (pos > 0) t_price = s_price.left(pos).trimmed().toFloat();

        if (t_price > 0 && t_price < p) palette.setColor(QPalette::Text, Qt::darkRed);
        if (t_price > 0 && t_price > p) palette.setColor(QPalette::Text, Qt::darkBlue);
    }
    priceLineEdit->setPalette(palette);
}
void AssetInfoWidget::updateHistory(const QStringList &data)
{
    int n_cols = historyTable->columnCount();
    foreach (const QString &v, data)
    {
        QStringList list(LString::trimSplitList(v, ";"));
        if (list.count() == n_cols)
        {
            int l_row = historyTable->rowCount();
            LTable::addTableRow(historyTable, list);
            if (v.contains("SELL")) historyTable->item(l_row, 1)->setTextColor(Qt::red);
            else if (v.contains("BUY")) historyTable->item(l_row, 1)->setTextColor(Qt::darkGreen);
        }
        else qWarning()<<QString("AssetInfoWidget::updateHistory WARNING list.count(%1) != table cols(%2)").arg(list.count()).arg(n_cols);
    }
    refreshTable();
}



#include "apibagpage.h"
#include "bagstate.h"
#include "apicommonsettings.h"
#include "ltable.h"
#include "apitradedialog.h"
#include "lstring.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QPoint>
#include <QMenu>

#define NAME_COL            1
#define TICKER_COL          2
#define PAPER_COUNT_COL     3
#define IN_ORDERS_COL       4
#define PRICE_COL           5
#define MARGIN_COL          6
#define PROFIT_COL          7
#define PAPER_TYPE_COL      8
#define TO_COMPLETE_COL     9


//APIBagPage
APIBagPage::APIBagPage(QWidget *parent)
    :APITablePageBase(parent),
      m_bag(NULL)
{
    setObjectName("api_bag_page");
    m_userSign = aptBag;
    m_orderData.reset();

    m_bag = new BagState(this);
    m_tableBox->setHeaderLabels(m_bag->tableHeaders());
    m_tableBox->setTitle("Positions");
    m_tableBox->vHeaderHide();
    m_tableBox->table()->setContextMenuPolicy(Qt::CustomContextMenu);
    initFilterBox();

    connect(this, SIGNAL(signalLoadPortfolio(const QJsonObject&)), m_bag, SLOT(slotLoadPortfolio(const QJsonObject&)));
    connect(this, SIGNAL(signalLoadPositions(const QJsonObject&)), m_bag, SLOT(slotLoadPositions(const QJsonObject&)));
    connect(m_bag, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_bag, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_bag, SIGNAL(signalBagUpdate()), this, SLOT(slotBagUpdate()));
    connect(m_tableBox->table(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(m_bag, SIGNAL(signalGetBondEndDateByUID(const QString&, QDate&)), this, SIGNAL(signalGetBondEndDateByUID(const QString&, QDate&)));
}
void APIBagPage::slotContextMenu(QPoint p)
{
    qDebug()<<QString("APIBagPage::slotContextMenu  %1/%2").arg(p.x()).arg(p.y());
    QTableWidget *t = m_tableBox->table();
    if (LTable::selectedRows(t).count() != 1) return;

    m_orderData.reset();
    QMenu *menu = new QMenu(this);
    QAction *act = new QAction(QIcon(APITradeDialog::iconByOrderType(totBuyLimit)), QString("Buy order"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotBuyOrder()));

    act = new QAction(QIcon(APITradeDialog::iconByOrderType(totSellLimit)), QString("Sell order"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotSellOrder()));

    menu->addSeparator();

    act = new QAction(QIcon(APITradeDialog::iconByOrderType(totTakeProfit)), QString("Set take profit"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotSetTakeProfit()));

    act = new QAction(QIcon(APITradeDialog::iconByOrderType(totStopLoss)), QString("Set stop loss"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotSetStopLoss()));

    // Вызываем контекстное меню
    menu->popup(m_tableBox->table()->viewport()->mapToGlobal(p));
}
void APIBagPage::initFilterBox()
{
    m_filterBox->setTitle("General state");
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QGridLayout *g_lay = new QGridLayout(m_filterBox);

    int l_row = 0;
    addGeneralEdit(QString("User ID"), l_row);
    addGeneralEdit(QString("Blocked"), l_row);
    addGeneralEdit(QString("Free"), l_row);
    addGeneralEdit(QString("Total, (10^3)"), l_row);

    QLineEdit *w_line = new QLineEdit(this);
    w_line->setReadOnly(true);
    w_line->setMaximumHeight(2);
    g_lay->addWidget(w_line, l_row, 0, 1, 2); l_row++;

    addGeneralEdit(QString("Full cost of papers"), l_row);
    addGeneralEdit(QString("Current profit"), l_row);

    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, l_row, 0, 2, 2);
}
void APIBagPage::addGeneralEdit(QString caption, int& row)
{
    QGridLayout *g_lay = qobject_cast<QGridLayout*>(m_filterBox->layout());
    if (g_lay)
    {
        g_lay->addWidget(new QLabel(caption), row, 0);
        caption = caption.toLower().remove(LString::spaceSymbol());
        QLineEdit *edit = new QLineEdit(this);
        edit->setReadOnly(true);
        edit->setObjectName(QString("%1_edit").arg(caption));
        g_lay->addWidget(edit, row, 1);
        row++;
    }
    else qWarning(" APIBagPage::addGeneralEdit WARNING - grid layout is NULL");
}
void APIBagPage::slotBagUpdate()
{
    const QObjectList children(m_filterBox->children());
    foreach (QObject *child, children)
    {
        if (!child) continue;
        if (child->objectName().contains("blocked"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strBlocked());
        }
        else if (child->objectName().contains("free"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strFree());
        }
        else if (child->objectName().contains("total"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strTotal());
        }
        else if (child->objectName().contains("userid"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(QString::number(api_commonSettings.user_id));
        }
        else if (child->objectName().contains("papers"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strPapersCost());
        }
        else if (child->objectName().contains("profit"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strCurProfit());
        }
    }
    reloadPosTable();
}
void APIBagPage::reloadPosTable()
{
    qDebug()<<QString("APIBagPage::reloadPosTable() posCount=%1").arg(m_bag->posCount());
    resetPage();
    if (!m_bag->hasPositions())
    {
        emit signalMsg(QString("Bag positions list is empty!"));
        return;
    }

    //fill table
    for (quint16 i=0; i<m_bag->posCount(); i++)
    {
        const BagPosition &pos = m_bag->posAt(i);
        addPosition(pos);

        int l_row = m_tableBox->table()->rowCount() - 1;
        m_tableBox->table()->item(l_row, 1)->setData(Qt::UserRole, pos.uid);
        updateCellsColors(l_row, pos);
    }

    //set selection mode for table
    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#DEFFBF", "#6B8E23");
    m_tableBox->searchExec();
}
void APIBagPage::addPosition(const BagPosition &pos)
{
    QTableWidget *t = m_tableBox->table();

    QStringList p_info;
    p_info << pos.paper_type << pos.uid;
    emit signalGetPaperInfo(p_info);

    QStringList row_data;
    row_data << QString::number(t->rowCount()+1);
    if (p_info.count() != 5) row_data << QString("?") << QString("?");
    else row_data << p_info.at(2) << p_info.at(3);
    row_data << QString::number(pos.count) << QString("-");
    row_data << pos.strPrice() << QString::number(int(pos.margin())) << pos.strProfit() << pos.paper_type;

    if (pos.isBond())
    {
        if (p_info.count() >= 5)
        {
            QDate fd = QDate::fromString(p_info.at(4), InstrumentBase::userDateMask());
            if (fd.isValid())
            {
                int dn = QDate::currentDate().daysTo(fd);
                if (dn < 0) dn = -1;
                row_data << QString::number(dn);
            }
            else row_data << "???";
        }
        else row_data << "????";
    }
    else
    {
        if (p_info.count() >= 5) row_data << p_info.at(4);
        else row_data << "?????";
    }

    LTable::addTableRow(t, row_data);
}
void APIBagPage::updateCellsColors(int l_row, const BagPosition &pos)
{
    QTableWidget *t = m_tableBox->table();

    //brush paper type cell
    if (!pos.isBond()) LTable::setTableRowColor(t, l_row, "#FFFFE0");

    //brush profit cell
    if (pos.curProfit() > 0) t->item(l_row, PROFIT_COL)->setTextColor("#006400");
    else if (pos.curProfit() < 0) t->item(l_row, PROFIT_COL)->setTextColor("#A52A2A");

    //brush days to finish cell
    bool ok;
    int to_complete = t->item(l_row, TO_COMPLETE_COL)->text().toInt(&ok);
    if (ok)
    {
        if (to_complete <= 0) LTable::setTableTextRowColor(t, l_row, QColor("#C0C0C0"));
        else if (to_complete < 20) t->item(l_row, TO_COMPLETE_COL)->setTextColor(QColor("#0000CD"));
        else if (to_complete > 180) t->item(l_row, TO_COMPLETE_COL)->setTextColor(QColor("#D2691E"));
    }
}
void APIBagPage::slotSetBagStocks(QStringList &list)
{
    list.clear();
    QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        QString p_type = t->item(i, PAPER_TYPE_COL)->text().trimmed();
        if (p_type.toLower() == "share")
            list.append(t->item(i, TICKER_COL)->text().trimmed());
    }
}
void APIBagPage::slotGetPaperCountByTicker(const QString &ticker, int &n_papers, float &cur_profit)
{
    n_papers = -1;
    cur_profit = 0;

    QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        if (ticker == t->item(i, TICKER_COL)->text().trimmed())
        {
            n_papers = t->item(i, PAPER_COUNT_COL)->text().trimmed().toInt();
            cur_profit = t->item(i, PROFIT_COL)->text().trimmed().toFloat();
            break;
        }
    }
}
void APIBagPage::slotUpdateOrdersInfo(const QMap<QString, QString> &map)
{
    QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        QColor cl = "#800000";
        QString uid = t->item(i, 1)->data(Qt::UserRole).toString();
        if (map.contains(uid))
        {
            QString s_count = map.value(uid).trimmed();
            t->item(i, IN_ORDERS_COL)->setText(s_count);
            if (s_count.contains("?")) cl = "#B22222";
            else if (s_count.contains("B(")) cl = "#004400";
        }
        else
        {
            t->item(i, IN_ORDERS_COL)->setText("---");
            cl = "#D3D3D3";
        }
        t->item(i, IN_ORDERS_COL)->setTextColor(QColor(cl));
    }
    m_tableBox->resizeByContents();
}
float APIBagPage::getCurrentPrice(int row) const
{
    if (row < 0 || row >= m_tableBox->table()->rowCount()) return -1;
    QString s_price = m_tableBox->table()->item(row, PRICE_COL)->text();

    int pos = s_price.indexOf("/");
    if (pos > 0)
    {
        s_price = LString::strTrimLeft(s_price, pos+1).trimmed();
        bool ok;
        float p = s_price.toFloat(&ok);
        if (ok && p > 0) return p;
    }
    qWarning("APIBagPage::getCurrentPrice WARNING invalid current price");
    return -1;
}
quint16 APIBagPage::paperCount(int row) const
{
    if (row < 0 || row >= m_tableBox->table()->rowCount()) return -1;
    QString s_count = m_tableBox->table()->item(row, PAPER_COUNT_COL)->text();

    bool ok;
    quint16 n = s_count.toUInt(&ok);
    if (!ok)
    {
        qWarning()<<QString("APIBagPage::paperCount WARNING invalid paper count (%1)").arg(s_count);
        n = 0;
    }
    return n;
}
void APIBagPage::tryPlaceOrder(TradeOperationData &t_data)
{
    APITradeDialog d(t_data, this);
    d.exec();
    if (d.isApply())
    {
        m_orderData.lots = t_data.lots;
        m_orderData.price = t_data.price;
        emit signalMsg(QString("Try set order(%1) id: [%2], paper: [%3], price=%4").
                       arg(APITradeDialog::captionByOrderType(t_data.order_type)).arg(m_orderData.uid).arg(t_data.company).arg(m_orderData.price));

        if (m_orderData.nominal > 0)
            m_orderData.price = float(qRound((10000*m_orderData.price)/m_orderData.nominal))/float(100);

        qDebug()<<QString("nominal=%1  result price: %2").arg(m_orderData.nominal).arg(m_orderData.price);

        emit signalSendOrderCommand(m_orderData);
    }
    else emit signalMsg("operation was canceled!");
}
void APIBagPage::prepareOrderData(TradeOperationData &t_data, int max_p)
{
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();

    t_data.company = QString("%1 / %2").arg(t->item(row, NAME_COL)->text()).arg(t->item(row, TICKER_COL)->text());
    t_data.paper_type = t->item(row, PAPER_TYPE_COL)->text();
    if (t_data.isBond())
    {
        bool ok;
        int d = t->item(row, TO_COMPLETE_COL)->text().toInt(&ok);
        if (!ok || d < 3)
        {
            emit signalError(QString("can't operation for this bond(%1), finish date too near to current date").arg(t_data.company));
            return;
        }
        t_data.finish_date = QString("%1 (%2)").arg(QDate::currentDate().addDays(d).toString(InstrumentBase::userDateMask())).arg(d);

        if (m_orderData.isStop())
            emit signalGetBondNominalByUID(m_orderData.uid, m_orderData.nominal);
    }
    else emit signalGetLotSize(m_orderData.uid, t_data.in_lot);
    t_data.price = getCurrentPrice(row);
    t_data.lots = 1;
    t_data.maxLot_ps = max_p;
}


////////////////ORDERS SLOTS/////////////////////////
void APIBagPage::slotBuyOrder()
{
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    m_orderData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    m_orderData.kind = "buy";

    //prepare data
    TradeOperationData t_data(totBuyLimit);
    prepareOrderData(t_data);

    //send order
    tryPlaceOrder(t_data);
}
void APIBagPage::slotSellOrder()
{
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    m_orderData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    m_orderData.kind = "sell";

    //prepare data
    TradeOperationData t_data(totSellLimit);
    prepareOrderData(t_data, paperCount(row));

    //send order
    tryPlaceOrder(t_data);
}
void APIBagPage::slotSetTakeProfit()
{
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    m_orderData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    m_orderData.is_stop = 1;
    m_orderData.kind = "sell";

    //prepare data
    TradeOperationData t_data(totTakeProfit);
    prepareOrderData(t_data, paperCount(row));

    //send order
    tryPlaceOrder(t_data);
}
void APIBagPage::slotSetStopLoss()
{
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    m_orderData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    m_orderData.is_stop = 2;
    m_orderData.kind = "sell";

    //prepare data
    TradeOperationData t_data(totStopLoss);
    prepareOrderData(t_data, paperCount(row));

    //send order
    tryPlaceOrder(t_data);
}


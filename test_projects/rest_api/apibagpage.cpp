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

#define MARGIN_COL          5
#define PROFIT_COL          6
#define PRICE_COL           4
#define NAME_COL            1
#define TICKER_COL          2
#define TO_COMPLETE_COL     8
#define PAPER_TYPE_COL      7


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
    connect(m_bag, SIGNAL(signalBagUpdate()), this, SLOT(slotBagUpdate()));
    connect(m_tableBox->table(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

}
void APIBagPage::slotContextMenu(QPoint p)
{
    qDebug()<<QString("APIBagPage::slotContextMenu  %1/%2").arg(p.x()).arg(p.y());
    QTableWidget *t = m_tableBox->table();
    if (LTable::selectedRows(t).count() != 1) return;

    m_orderData.reset();
    QMenu *menu = new QMenu(this);
    QAction *act = new QAction(QIcon(":/icons/images/ball_green.svg"), QString("Buy order"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotBuyOrder()));

    act = new QAction(QIcon(":/icons/images/ball_red.svg"), QString("Sell order"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotSellOrder()));

    menu->addSeparator();
    act = new QAction(QIcon(":/icons/images/ball_gray.svg"), QString("Sell stop order"), this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(slotSellStopOrder()));

    // Вызываем контекстное меню
    menu->popup(m_tableBox->table()->viewport()->mapToGlobal(p));
}
void APIBagPage::slotBuyOrder()
{
    qDebug("APIBagPage::slotBuyOrder()");
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    m_orderData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    QString p_name = QString("%1 / %2").arg(t->item(row, NAME_COL)->text()).arg(t->item(row, TICKER_COL)->text());
    m_orderData.is_stop = false;
    m_orderData.kind = "buy";

    //prepare dialog
    TradeOperationData t_data(totBuyLimit);
    t_data.company = p_name;
    t_data.paper_type = t->item(row, PAPER_TYPE_COL)->text();
    if (t_data.isBond())
    {
        bool ok;
        int d = t->item(row, TO_COMPLETE_COL)->text().toInt(&ok);
        if (!ok || d < 3)
        {
            emit signalError(QString("invalid finish data for bond (%1)").arg(p_name));
            return;
        }
        t_data.finish_date = QString("%1 (%2)").arg(QDate::currentDate().addDays(d).toString(InstrumentBase::userDateMask())).arg(d);
    }
    else emit signalGetLotSize(m_orderData.uid, t_data.in_lot);
    t_data.price = getCurrentPrice(row);
    t_data.lots = 1;

    APITradeDialog d(t_data, this);
    d.exec();
    if (d.isApply())
    {
        m_orderData.lots = t_data.lots;
        m_orderData.price = t_data.price;
        emit signalMsg(QString("Try set BUY order id: [%1], paper: [%2]").arg(m_orderData.uid).arg(p_name));
        emit signalSendOrderCommand(m_orderData);
    }
    else emit signalMsg("operation was canceled!");


/*
    //prepare request data
    QString src = QString();
    foreach(const QString &v, api_commonSettings.services)
    {
        if (v.toLower().contains("post") && v.toLower().contains("order"))
        {
            src = v;
            break;
        }
    }
    if (src.isEmpty())
    {
        emit signalError(QString("Not found API metod for buy order, uid=[%1]").arg(uid));
        return;
    }

    QStringList req_data;
    req_data << src << uid << QString::number(t_data.lots) << QString::number(t_data.price, 'f', 2) << "buy";
    emit signalMsg(QString("Try set %1 order id: [%2], paper: [%3]").arg(req_data.last()).arg(uid).arg(p_name));
    emit signalSendOrderCommand(req_data);
    */

}
void APIBagPage::slotSellOrder()
{
    qDebug("APIBagPage::slotSellOrder()");
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    m_orderData.uid = t->item(row, 1)->data(Qt::UserRole).toString();
    QString p_name = QString("%1 / %2").arg(t->item(row, NAME_COL)->text()).arg(t->item(row, TICKER_COL)->text());
    m_orderData.is_stop = false;
    m_orderData.kind = "sell";

    //prepare dialog
    TradeOperationData t_data(totSellLimit);
    t_data.company = p_name;
    t_data.paper_type = t->item(row, PAPER_TYPE_COL)->text();
    if (t_data.isBond())
    {
        bool ok;
        int d = t->item(row, TO_COMPLETE_COL)->text().toInt(&ok);
        if (!ok || d < 3)
        {
            emit signalError(QString("invalid finish data for bond (%1)").arg(p_name));
            return;
        }
        t_data.finish_date = QString("%1 (%2)").arg(QDate::currentDate().addDays(d).toString(InstrumentBase::userDateMask())).arg(d);
    }
    else emit signalGetLotSize(m_orderData.uid, t_data.in_lot);
    t_data.price = getCurrentPrice(row);
    t_data.lots = 1;

    APITradeDialog d(t_data, this);
    d.exec();
    if (d.isApply())
    {
        m_orderData.lots = t_data.lots;
        m_orderData.price = t_data.price;
        emit signalMsg(QString("Try set SELL order id: [%1], paper: [%2]").arg(m_orderData.uid).arg(p_name));
        emit signalSendOrderCommand(m_orderData);
    }
    else emit signalMsg("operation was canceled!");


    /*
    QTableWidget *t = m_tableBox->table();
    int row = LTable::selectedRows(t).first();
    QString uid = t->item(row, 1)->data(Qt::UserRole).toString();
    QString p_name = QString("%1 / %2").arg(t->item(row, NAME_COL)->text()).arg(t->item(row, TICKER_COL)->text());

    //prepare dialog
    TradeOperationData t_data(totSellLimit);
    t_data.company = p_name;
    t_data.paper_type = t->item(row, PAPER_TYPE_COL)->text();
    if (t_data.isBond())
    {
        bool ok;
        int d = t->item(row, TO_COMPLETE_COL)->text().toInt(&ok);
        if (!ok || d < 3)
        {
            emit signalError(QString("invalid finish data for bond (%1)").arg(p_name));
            return;
        }
        t_data.finish_date = QString("%1 (%2)").arg(QDate::currentDate().addDays(d).toString(InstrumentBase::userDateMask())).arg(d);
    }
    else
    {
        emit signalGetLotSize(uid, t_data.in_lot);
    }
    t_data.price = getCurrentPrice(row);
    t_data.lots = 1;

    APITradeDialog d(t_data, this);
    d.exec();
    if (!d.isApply())
    {
        emit signalMsg("operation was canceled!");
        return;
    }


    //prepare request data
    QString src = QString();
    foreach(const QString &v, api_commonSettings.services)
    {
        if (v.toLower().contains("post") && v.toLower().contains("order"))
        {
            src = v;
            break;
        }
    }
    if (src.isEmpty())
    {
        emit signalError(QString("Not found API metod for buy order, uid=[%1]").arg(uid));
        return;
    }

    QStringList req_data;
    req_data << src << uid << QString::number(t_data.lots) << QString::number(t_data.price, 'f', 2) << "sell";
    emit signalMsg(QString("Try set %1 order id: [%2], paper: [%3]").arg(req_data.last()).arg(uid).arg(p_name));
    emit signalSendOrderCommand(req_data);
    */
}
void APIBagPage::slotSellStopOrder()
{
    qDebug("APIBagPage::slotSellStopOrder()");

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
    addGeneralEdit(QString("Total"), l_row);

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
        //qDebug()<<edit->objectName();
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
    resetPage();
    if (!m_bag->hasPositions())
    {
        emit signalMsg(QString("Bag positions list is empty!"));
        return;
    }


    qDebug()<<QString("APIBagPage: m_bag->posCount() %2").arg(m_bag->posCount());
    bool ok;
    for (quint16 i=0; i<m_bag->posCount(); i++)
    {
        const BagPosition &pos = m_bag->posAt(i);
        qDebug()<< pos.uid;
        QStringList p_info;
        p_info << pos.paper_type << pos.uid;
        emit signalGetPaperInfo(p_info);

        QStringList row_data;
        row_data << QString::number(i+1);
        if (p_info.count() != 5) row_data << QString("?") << QString("?");
        else row_data << p_info.at(2) << p_info.at(3);
        row_data << QString::number(pos.count) << pos.strPrice() << QString::number(int(pos.margin())) << pos.strProfit() << pos.paper_type;

        if (pos.paper_type == "bond")
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

        LTable::addTableRow(m_tableBox->table(), row_data);
        int l_row = m_tableBox->table()->rowCount() - 1;
        m_tableBox->table()->item(l_row, 1)->setData(Qt::UserRole, pos.uid);
        if (pos.paper_type != "bond") LTable::setTableRowColor(m_tableBox->table(), l_row, "#FFFFE0");

        if (pos.curProfit() > 0) m_tableBox->table()->item(l_row, PROFIT_COL)->setTextColor("#006400");
        else if (pos.curProfit() < 0) m_tableBox->table()->item(l_row, PROFIT_COL)->setTextColor("#A52A2A");

        int to_complete = m_tableBox->table()->item(l_row, TO_COMPLETE_COL)->text().toInt(&ok);
        if (!ok) continue;

        if (to_complete <= 0) LTable::setTableTextRowColor(m_tableBox->table(), l_row, QColor("#C0C0C0"));
        else if (to_complete < 20) m_tableBox->table()->item(l_row, TO_COMPLETE_COL)->setTextColor(QColor("#0000CD"));
        else if (to_complete > 180) m_tableBox->table()->item(l_row, TO_COMPLETE_COL)->setTextColor(QColor("#D2691E"));

    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#DEFFBF", "#6B8E23");
    m_tableBox->searchExec();
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
        if (ok & p > 0) return p;
    }
    qWarning("APIBagPage::getCurrentPrice WARNING invalid current price");
    return -1;
}


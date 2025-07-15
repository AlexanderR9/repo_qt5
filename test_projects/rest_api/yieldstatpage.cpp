#include "yieldstatpage.h"
#include "ltable.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QTableWidget>

#define YIELD_COL           6
#define TIKER_COL           2
#define PAPER_TYPE_COL      3
#define DATE_COL            0
#define TOTAL_COL           7


//YieldStatPage
YieldStatPage::YieldStatPage(QWidget *parent)
    :APITablePageBase(parent),
      m_bondYield(0),
      m_stockYield(0)
{
    setObjectName("api_events_page");
    m_userSign = aptYield;

    initFilterBox();

    QStringList headers;
    headers << "Last deal" << "Company" << "Ticker" << "Paper type" << "N papers" << "N events" << "Yield" << "Total yield";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Statistic");
    m_tableBox->setObjectName("table_box");
    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    resetPage();
}
void YieldStatPage::resetPage()
{
    APITablePageBase::resetPage();
    m_tableBox->removeAllRows();

    QStringList list;
    list << "involved_edit" << "start_edit" << "life_time_edit" << "y_bonds_edit" << "y_stocks_edit" << "total_edit";
    foreach (QLineEdit *obj, m_editList)
    {
        if (!obj) continue;
        //qDebug()<<QString("child [%1]").arg(obj->objectName());
        if (!list.contains(obj->objectName())) continue;
        obj->setText("?");
    }

    m_tableBox->searchExec();
}
void YieldStatPage::initFilterBox()
{
    m_filterBox->setTitle("General information");
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    m_filterBox->setObjectName("filter_box");

    int i = 0;
    QGridLayout *g_lay = new QGridLayout(m_filterBox);
    g_lay->addWidget(new QLabel("Papers involved"), i, 0); i++;
    g_lay->addWidget(new QLabel("First operations"), i, 0); i++;
    g_lay->addWidget(new QLabel("Bag life time, days"), i, 0); i++;
    g_lay->addWidget(new QLabel("Yield by bonds, R"), i, 0); i++;
    g_lay->addWidget(new QLabel("Yield by stocks, R"), i, 0); i++;
    g_lay->addWidget(new QLabel("Total, kR"), i, 0); i++;

    QStringList list;
    list << "involved_edit" << "start_edit" << "life_time_edit" << "y_bonds_edit" << "y_stocks_edit" << "total_edit";
    i = 0;
    foreach (const QString &v, list)
    {
        QLineEdit *edit = new QLineEdit(this);
        edit->setObjectName(v);
        edit->setReadOnly(true);
        edit->setMaximumWidth(170);
        g_lay->addWidget(edit, i, 1);
        m_editList.append(edit);
        i++;
    }

    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, i, 0, 2, 2);
}
void YieldStatPage::setChildEditText(QString child_name, QString text)
{
   // qDebug()<<QString("setChildEditText  child_name [%1],  text [%2]").arg(child_name).arg(text);
    foreach (QLineEdit *obj, m_editList)
    {
        if (!obj) continue;
        if (obj->objectName() == child_name)
        {
            obj->setText(text);
            break;
        }
    }
}
void YieldStatPage::slotReceivedEvents(const QList<EventOperation> &data)
{
    qDebug()<<QString("YieldStatPage::slotReceivedEvents  data size %1").arg(data.count());
    resetPage();
    if (data.isEmpty()) return;

   // qDebug()<<QString("first record: %1").arg(data.first().toStr());
    findStartDate(data);
    recalcYield(data);
    sortTableByDate();
    recalcTotalYield();
    recolorTable();

    setChildEditText("involved_edit", QString::number(m_tableBox->table()->rowCount()));
    setChildEditText("y_bonds_edit", QString::number(int(m_bondYield)));
    setChildEditText("y_stocks_edit", QString::number(int(m_stockYield)));
    setChildEditText("total_edit", QString::number(totalYield_k(), 'f', 1));

    m_tableBox->resizeByContents();
    m_tableBox->searchExec();
    m_tableBox->table()->scrollToBottom();
}
void YieldStatPage::findStartDate(const QList<EventOperation> &data)
{
    QDateTime dt = QDateTime::currentDateTime();
    QDate start_date = QDate::currentDate();
    foreach (const EventOperation &rec, data)
    {
        if (rec.date.date() < start_date) start_date = rec.date.date();
    }
    setChildEditText("start_edit", start_date.toString(InstrumentBase::userDateMask()));
    int n_days = start_date.daysTo(dt.date());
    setChildEditText("life_time_edit", QString::number(n_days));
}
QDate YieldStatPage::bagStartDate() const
{
    QDate d;
    foreach (QLineEdit *obj, m_editList)
    {
        if (!obj) continue;
        if (obj->objectName() == "start_edit")
        {
            QString s = obj->text().trimmed();
            if (s.length() == InstrumentBase::userDateMask().length())
            {
                d = QDate::fromString(s, InstrumentBase::userDateMask());
            }
            break;
        }
    }
    return d;
}
void YieldStatPage::recalcYield(const QList<EventOperation> &data)
{
    qDebug("YieldStatPage::recalcYield");
    int n = data.count();
    int start_index = n-1;
    QStringList uid_list;

    int n_recalc = 0;
    while (2 > 1)
    {        
        QString next_uid = QString();
        //qDebug()<<QString("YieldStatPage::recalcYield  start_index=%1  n_recalc=%2").arg(start_index).arg(n_recalc);
        for (int i=start_index; i>=0; i--)
        {
            QString cur_uid = data.at(i).uid.trimmed();
            if (cur_uid.length() < 20) {qDebug("NOTICE: cur_uid.length() < 20"); continue;}

            if (!uid_list.contains(cur_uid))
            {
                next_uid = cur_uid;
                uid_list.append(next_uid);
                start_index = i-1;
               // qDebug()<<QString("   -----------------  find next_uid[%1] -----------------------").arg(next_uid);
                break;
            }
        }
        if (next_uid.isEmpty()) break;

       // qDebug()<<data.at(start_index-1).toStr();
        recalcYieldByUID(next_uid, data);
        n_recalc++;
        if (start_index < 0) break;
        //if (n_recalc > 30) break;
    }
}
void YieldStatPage::recalcYieldByUID(const QString &uid, const QList<EventOperation> &data)
{
    //qDebug()<<QString("recalcYieldByUID[%1]").arg(uid);
    int n_sell = 0;
    int n_buy = 0;
    bool is_repayment = false;

    YieldRowData row_data;
    foreach (const EventOperation &rec, data)
    {
        if (rec.uid == uid)
        {
            row_data.n_events++;
            parseNextRec(rec, row_data, n_buy, n_sell);
            if (rec.kind == EventOperation::etRepayment) is_repayment = true;
        }
    }

    if (n_buy == 0) return;
    row_data.n_papers = n_buy;

    QStringList p_info;
    p_info << row_data.paper_type << uid;
    emit signalGetPaperInfo(p_info);

    if (p_info.count() > 3)  {row_data.name = p_info.at(2); row_data.ticker = p_info.at(3);}
   // qDebug()<<QString("found %1 events for this UID,  n_buy/n_sell: %2/%3,  TICKER(%4)").arg(row_data.n_events).arg(n_buy).arg(n_sell).arg(row_data.ticker);
    //if (is_repayment) qDebug("***************** WAS Repayment ***************");
    if (n_buy == n_sell || is_repayment) //finished paper
    {
       // qDebug()<<QString("UID[%1]  n_papers=%2  yield=%3  divs=%4  coupons=%5").arg(uid).arg(row_data.n_papers).arg(row_data.yield).arg(row_data.div).arg(row_data.coupon);
        if (row_data.isStock()) row_data.yield += row_data.div;
        if (row_data.isBond()) row_data.yield += row_data.coupon;
        addYieldRowData(row_data);
    }
    else
    {
      //  qDebug("NOT FINISHED!!!!!");
    }
}
void YieldStatPage::parseNextRec(const EventOperation &rec, YieldRowData &row_data, int &n_buy, int &n_sell)
{
    if (row_data.paper_type.isEmpty())
    {
        row_data.paper_type = rec.paper_type.trimmed();
        row_data.last_date = rec.date.date();
    }
    else if (row_data.last_date < rec.date.date()) row_data.last_date = rec.date.date();

    //check kind
    switch (rec.kind)
    {
        case EventOperation::etSell:
        {
            row_data.yield += rec.amount;
            n_sell += rec.n_papers;
            break;
        }
        case EventOperation::etBuy:
        {
            row_data.yield += rec.amount;
            n_buy += rec.n_papers;
            break;
        }
        case EventOperation::etCommission:
        {
            row_data.yield += rec.amount;
            break;
        }
        case EventOperation::etRepayment:
        {
            row_data.yield += rec.amount;
            n_sell = -100000;
            break;
        }
        case EventOperation::etCoupon:
        {
            if (row_data.isBond()) row_data.coupon += rec.amount;
            break;
        }
        case EventOperation::etDiv:
        case EventOperation::etTax:
        {
            if (row_data.isStock()) row_data.div += rec.amount;
            break;
        }
        default: break;
    }
}
void YieldStatPage::addYieldRowData(const YieldRowData &row_data)
{
    QStringList list;
    list << row_data.last_date.toString(InstrumentBase::userDateMask());
    list << row_data.name << row_data.ticker << row_data.paper_type;
    list << QString::number(row_data.n_papers) << QString::number(row_data.n_events);
    list << QString::number(row_data.yield, 'f', 1) << "?";

    //if (row_data.paper_type == "bond") m_bondYield += row_data.yield;
    //else if (row_data.paper_type == "share") m_stockYield += row_data.yield;
    //else emit signalError(QString("YieldStatPage: invalid paper_type: %1").arg(row_data.paper_type));
    //list << QString(QString::number(totalYield_k(), 'f', 2) + "k");

    LTable::addTableRow(m_tableBox->table(), list);
}
void YieldStatPage::recalcTotalYield()
{
    m_bondYield = m_stockYield = 0;
    QTableWidget *t = m_tableBox->table();
    int n_rows = t->rowCount();
    if (n_rows == 0) return;

    for (int i=0; i<n_rows; i++)
    {
        float yield = t->item(i, YIELD_COL)->text().toFloat();
        QString p_type = t->item(i, PAPER_TYPE_COL)->text().trimmed().toLower();
        if (p_type == "bond") m_bondYield += yield;
        if (p_type == "share") m_stockYield += yield;
        t->item(i, TOTAL_COL)->setText(QString(QString::number(totalYield_k(), 'f', 2) + "k"));
    }
}
void YieldStatPage::recolorTable()
{
    qDebug("YieldStatPage::sortTableByDate()");

    QTableWidget *t = m_tableBox->table();
    int n_rows = t->rowCount();
    if (n_rows == 0) return;

    for (int i=0; i<n_rows; i++)
    {
        float yield = t->item(i, YIELD_COL)->text().toFloat();
        if (yield < 0) t->item(i, YIELD_COL)->setTextColor(Qt::red);
        else if (yield < 100) t->item(i, YIELD_COL)->setTextColor(Qt::lightGray);
        else if (yield > 2000) t->item(i, YIELD_COL)->setTextColor(Qt::blue);
        else if (yield > 8000) LTable::setTableTextRowColor(t, i, Qt::yellow);

        QString ticker = t->item(i, TIKER_COL)->text().trimmed();
        if (ticker.length() < 4) LTable::setTableTextRowColor(t, i, Qt::lightGray);

        QString p_type = t->item(i, PAPER_TYPE_COL)->text().trimmed().toLower();
        if (p_type == "bond") t->item(i, PAPER_TYPE_COL)->setTextColor(Qt::darkGreen);
        else if (p_type == "share") t->item(i, PAPER_TYPE_COL)->setTextColor("#FF4500");
        else t->item(i, PAPER_TYPE_COL)->setTextColor(Qt::red);

    }

    // color month groups
    QDate start_date(bagStartDate());
    if (!start_date.isValid()) {emit signalError("bagStartDate is invalid"); return;}

    bool even  = false;
    int i = 0;
    //int k = 0;
    while (2 > 1)
    {
        //k++;
        //if (k > 300) break;

        QColor row_color = even ? QColor("#F0E6DC") : QColor("#D0F0F6");
        QString s_date = t->item(i, DATE_COL)->text().trimmed();
        QDate row_date = QDate::fromString(s_date, InstrumentBase::userDateMask());

        //qDebug()<<QString("i=%1, row_date: %2,  start_date: %3").arg(i).arg(s_date).arg(start_date.toString(InstrumentBase::userDateMask()));
        if (row_date.month() != start_date.month())
        {
            //qDebug("FIND CHANGE MONTH:  row_date.month() > start_date.month()");
            even = !even;
            i--;
            start_date = start_date.addMonths(1);
        }
        else LTable::setTableRowColor(t, i, row_color);

        i++;
        if (i >= n_rows) break;
    }
}
void YieldStatPage::sortTableByDate()
{
    qDebug("YieldStatPage::sortTableByDate()");
    QTableWidget *t = m_tableBox->table();
    int n_rows = t->rowCount();
    if (n_rows == 0) return;

    int start_row = 1;
    while (2>1)
    {
        if (start_row >= n_rows) break;
        QString s_date = t->item(start_row-1, DATE_COL)->text().trimmed();
        QDate row_date = QDate::fromString(s_date, InstrumentBase::userDateMask());

       // qDebug()<<QString("start_row %1, cur_date: %2").arg(start_row).arg(s_date);

        int less_date_pos = -1;
        for (int i=start_row; i<n_rows; i++)
        {
            s_date = t->item(i, DATE_COL)->text().trimmed();
            if (QDate::fromString(s_date, InstrumentBase::userDateMask()) < row_date) //find less date
            {
                less_date_pos = i;
                //qDebug()<<QString("FOUND_LESS_DATE: less_date_pos %1, less_date: %2").arg(less_date_pos).arg(s_date);
                break;
            }
        }

        //check find result
        if (less_date_pos > 0) //need replace rows and find less again
        {
            int shift_size = (start_row - 1) - less_date_pos;
            LTable::shiftTableRow(t, less_date_pos, shift_size);
        }
        else //row_date is smallest,  follow to next row
        {
            start_row++;
        }
    }
}


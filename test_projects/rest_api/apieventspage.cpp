#include "apieventspage.h"
#include "ltable.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QLabel>
#include <QDate>
#include <QComboBox>
#include <QGridLayout>


#define AMOUNT_COL          9
#define KIND_COL            0
#define PAPER_TYPE_COL      3
#define DATE_COL            5
#define COUPON_COL          8


#define COLOR_DAY_EVEN      QString("#FFFFF2")
#define COLOR_DAY_ODD       QString("#FAFFFD")
#define NONE_FILTER         QString("none")


//APICouponPageAbstract
APIEventsPage::APIEventsPage(QWidget *parent)
    :APITablePageBase(parent),
    m_statBox(NULL),
    m_paperTypeFilterControl(NULL),
    m_kindFilterControl(NULL),
    m_dateFilterControl(NULL)
{
    setObjectName("api_events_page");
    m_userSign = aptEvent;

    QStringList headers;
    headers << "Kind" << "Company" << "Ticker" << "Paper type" << "Currency" << "Date" << "N" << "Size" << "Coupon" << "Amount";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Bag operations");
    m_tableBox->setObjectName("table_box");

    reinitWidgets();
    initFilterBox();
}
void APIEventsPage::reinitWidgets()
{
    if (m_filterBox) {m_filterBox->deleteLater(); m_filterBox = NULL;}

    qDebug("APIEventsPage::reinitWidgets");
    LSimpleWidget *stat_widget = new LSimpleWidget(this, 10);
    stat_widget->setObjectName("events_stat_widget");
    //stat_widget->setSpacing(2);

    m_filterBox = new QGroupBox(this);
    m_filterBox->setTitle("Filter parameters");
    stat_widget->addWidgetToSplitter(m_filterBox, Qt::Vertical);
    m_statBox = new LTableWidgetBox(this);
    m_statBox->setTitle("Total statistic");
    stat_widget->addWidgetToSplitter(m_statBox, Qt::Vertical);
    m_statBox->setObjectName("stat_box");

    QStringList headers;
    headers.append("Value");
    m_statBox->setHeaderLabels(headers);
    headers.clear();
    headers << "Coupons" << "Divs" << "Commission" << "Tax" << "Input" << "Out";
    m_statBox->setHeaderLabels(headers, Qt::Vertical);
    for (int i=0; i<m_statBox->table()->rowCount(); i++)
        LTable::createTableItem(m_statBox->table(), i, 0, "-");


    h_splitter->insertWidget(1, stat_widget);
    m_statBox->resizeByContents();
}
void APIEventsPage::slotLoadEvents(const QJsonObject &j_obj)
{
    m_events.clear();
    if (j_obj.isEmpty()) return;

    const QJsonValue &j_operations = j_obj.value("operations");
    if (!j_operations.isArray()) return;

    const QJsonArray &j_arr = j_operations.toArray();
    int n = j_arr.count();
    qDebug()<<QString(" APIEventsPage::slotLoadEvents arr_size %1").arg(n);
    for (int i=0; i<n; i++)
    {
        EventOperation e;
        e.fromJson(j_arr.at(i));
        if (!e.invalid()) m_events.append(e);
    }

    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_events.count()));
    reloadTableByData();
    recalcStat();
}
void APIEventsPage::reloadTableByData()
{
    resetPage();
    if (m_events.isEmpty())
    {
        emit signalMsg(QString("Event operations is empty!"));
        return;
    }

    QDate cur_date;
    QString row_color = COLOR_DAY_EVEN;
    foreach (const EventOperation &rec, m_events)
    {
        QPair<QString, QString> pair;
        emit signalGetPaperInfoByFigi(rec.uid, pair);

        emit signalGetTickerByFigi(rec.uid, pair.second);
        if (pair.second.length() < 2) pair.second = "---"; //ticker
        if (pair.first.length() < 5) pair.first = "---"; //company name

        if (!cur_date.isValid() || cur_date != rec.date)
        {
            if (row_color == COLOR_DAY_EVEN) row_color = COLOR_DAY_ODD;
            else row_color = COLOR_DAY_EVEN;
            cur_date = rec.date;
        }

        addRowRecord(rec, pair, QColor(row_color));

        //if (rec.kind == EventOperation::etTax || rec.kind == EventOperation::etCommission)
            //LTable::setTableRowColor(m_tableBox->table(), l_row, "#FFCDFB");
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#D3D3D3", "#2F4F4F");
    m_tableBox->searchExec();    
}
void APIEventsPage::addRowRecord(const EventOperation &rec, const QPair<QString, QString> &info, QColor row_color)
{
    QStringList row_data;
    row_data << rec.strKind() << info.first << info.second << rec.paper_type << rec.currency;
    row_data << rec.date.toString(InstrumentBase::userDateMask()) << QString::number(rec.n_papers);

    if (rec.kind == EventOperation::etBuy || rec.kind == EventOperation::etSell)
    {
        row_data << QString("%1/%2").arg(QString::number(rec.size, 'f', 2)).arg(QString::number(rec.size*rec.n_papers, 'f', 1));
        float saved_coupons = qAbs(rec.amount) - (rec.size*rec.n_papers);
        row_data << QString("%1/%2").arg(QString::number(saved_coupons/rec.n_papers, 'f', 2)).arg(QString::number(saved_coupons, 'f', 1));
    }
    else
    {
        row_data << QString::number(rec.size, 'f', 2) << QString("-");

    }
    row_data << QString::number(rec.amount, 'f', 1);
    LTable::addTableRow(m_tableBox->table(), row_data);

    int l_row = m_tableBox->table()->rowCount() - 1;
    if (rec.amount < 0) m_tableBox->table()->item(l_row, AMOUNT_COL)->setTextColor(Qt::darkRed);
    LTable::setTableRowColor(m_tableBox->table(), l_row, row_color);


}
void APIEventsPage::load(QSettings &settings)
{
    APITablePageBase::load(settings);
    for(int i=0; i<children().count(); i++)
    {
        if (h_splitter->children().at(i)->objectName() == "events_stat_widget")
        {
            LSimpleWidget *stat_widget = qobject_cast<LSimpleWidget*>(h_splitter->children().at(i));
            if (stat_widget) {qDebug("loading stat_widget"); stat_widget->load(settings);}
            else qWarning("APIEventsPage::load WARNING  stat_widget is NULL");
            break;
        }
    }
}
void APIEventsPage::save(QSettings &settings)
{
    APITablePageBase::save(settings);
    for(int i=0; i<h_splitter->children().count(); i++)
    {
        if (h_splitter->children().at(i)->objectName() == "events_stat_widget")
        {
            LSimpleWidget *stat_widget = qobject_cast<LSimpleWidget*>(h_splitter->children().at(i));
            if (stat_widget) {qDebug("saving stat_widget"); stat_widget->save(settings);}
            else qWarning("APIEventsPage::load WARNING  stat_widget is NULL");
            break;
        }
    }
}
void APIEventsPage::initFilterBox()
{
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QGridLayout *g_lay = new QGridLayout(m_filterBox);
    g_lay->addWidget(new QLabel("Paper type"), 0, 0);
    g_lay->addWidget(new QLabel("Operation kind"), 1, 0);
    g_lay->addWidget(new QLabel("Last months"), 2, 0);

    QStringList list;

    m_paperTypeFilterControl = new QComboBox(this);
    list << NONE_FILTER << "bond" << "share";
    m_paperTypeFilterControl->addItems(list);
    g_lay->addWidget(m_paperTypeFilterControl, 0, 1);

    list.clear();
    m_kindFilterControl = new QComboBox(this);
    list << NONE_FILTER << "CANCELED" << "COMMISSION" << "BUY" << "SELL" << "COUPON" << "DIV" << "TAX" << "INPUT" << "OUT" << "REPAYMENT";
    m_kindFilterControl->addItems(list);
    g_lay->addWidget(m_kindFilterControl, 1, 1);

    list.clear();
    m_dateFilterControl = new QComboBox(this);
    list << NONE_FILTER;
    for (int i=1; i<=12; i++) list << QString::number(i);
    for (int i=3; i<=6; i++) list << QString::number(i*6);
    m_dateFilterControl->addItems(list);
    g_lay->addWidget(m_dateFilterControl, 2, 1);

    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, 3, 0, 2, 2);

    connect(m_paperTypeFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter()));
    connect(m_kindFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter()));
    connect(m_dateFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter()));
}
void APIEventsPage::slotFilter()
{
    if (!sender()) return;
    qDebug()<<QString("slotFilter");

    int n = m_tableBox->table()->rowCount();
    if (n <= 0) return;

    m_tableBox->searchExec();
    for (int i=0; i<n; i++)
    {
        if (m_tableBox->table()->isRowHidden(i)) continue;

        bool need_hide = false;
        paperTypeFilter(i, need_hide);
        if (need_hide) continue;
        kindFilter(i, need_hide);
        if (need_hide) continue;
        dateFilter(i, need_hide);
        if (need_hide) continue;
    }

    recalcStat();
}
void APIEventsPage::recalcStat()
{
    m_stat.reset();

    //calc params
    QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        if (t->isRowHidden(i)) continue;
        updateStatStruct(m_events.at(i));
    }

    //update stat_table items
    updateStatTable();
}
void APIEventsPage::paperTypeFilter(int row, bool &need_hide)
{
    QString text = m_paperTypeFilterControl->currentText().trimmed();
    if (text == NONE_FILTER) return;
    if (m_tableBox->table()->item(row, PAPER_TYPE_COL)->text() != text)
    {
        m_tableBox->table()->hideRow(row);
        need_hide = true;
    }
}
void APIEventsPage::kindFilter(int row, bool &need_hide)
{
    QString text = m_kindFilterControl->currentText().trimmed();
    if (text == NONE_FILTER) return;

    if (m_tableBox->table()->item(row, KIND_COL)->text() != text)
    {
        m_tableBox->table()->hideRow(row);
        need_hide = true;
    }
}
void APIEventsPage::dateFilter(int row, bool &need_hide)
{
    QString text = m_dateFilterControl->currentText().trimmed();
    if (text == NONE_FILTER) return;

    bool ok;
    quint8 n_month = text.toUInt(&ok);
    if (!ok) return;

    QDate limit_date = QDate::currentDate().addDays(-1*n_month*30);
    if (m_events.at(row).date < limit_date)
    {
        m_tableBox->table()->hideRow(row);
        need_hide = true;
    }
}
void APIEventsPage::updateStatStruct(const EventOperation &rec)
{
    switch (rec.kind)
    {
        case EventOperation::etCoupon:
        {
            m_stat.coupon += rec.amount;
            break;
        }
        case EventOperation::etCommission:
        {
            m_stat.commission += rec.amount;
            break;
        }
        case EventOperation::etDiv:
        {
            m_stat.div += rec.amount;
            break;
        }
        case EventOperation::etTax:
        {
            m_stat.tax += rec.amount;
            break;
        }
        case EventOperation::etInput:
        {
            m_stat.input += rec.amount;
            break;
        }
        case EventOperation::etOut:
        {
            m_stat.out += rec.amount;
            break;
        }
        default: break;
    }
}
void APIEventsPage::updateStatTable()
{
    QTableWidget *t = m_statBox->table();
    t->item(0, 0)->setText(QString::number(m_stat.coupon, 'f', 1));
    t->item(1, 0)->setText(QString::number(m_stat.div, 'f', 1));
    t->item(2, 0)->setText(QString::number(m_stat.commission, 'f', 1));
    t->item(3, 0)->setText(QString::number(m_stat.tax, 'f', 1));
    t->item(4, 0)->setText(QString::number(m_stat.input, 'f', 0));
    t->item(5, 0)->setText(QString::number(m_stat.out, 'f', 0));
}
void APIEventsPage::resetPage()
{
    APITablePageBase::resetPage();
    m_paperTypeFilterControl->setCurrentIndex(0);
    m_kindFilterControl->setCurrentIndex(0);
    m_dateFilterControl->setCurrentIndex(0);
}




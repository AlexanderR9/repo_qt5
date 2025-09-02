#include "apieventspage.h"
#include "ltable.h"
#include "lfile.h"
#include "apicommonsettings.h"


#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QLabel>
#include <QDate>
#include <QDir>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>


#define AMOUNT_COL          9
#define KIND_COL            0
#define PAPER_TYPE_COL      3
#define DATE_COL            5
#define COUPON_COL          8
#define COMPANY_COL         1
#define N_COL               6
#define TICKER_COL          2



#define COLOR_DAY_EVEN      QString("#FFFFE0")
#define COLOR_DAY_ODD       QString("#EEE8AA")
#define NONE_FILTER         QString("none")


//APIEventsPage
APIEventsPage::APIEventsPage(QWidget *parent)
    :APITablePageBase(parent),
    m_statBox(NULL),
    m_paperTypeFilterControl(NULL),
    m_kindFilterControl(NULL),
    m_dateFilterControl(NULL),
    m_paperResultEdit(NULL)
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

    connect(m_tableBox, SIGNAL(signalSearched()), this, SLOT(slotFilter()));
}
void APIEventsPage::reinitWidgets()
{
    if (m_filterBox) {m_filterBox->deleteLater(); m_filterBox = NULL;}

    //qDebug("APIEventsPage::reinitWidgets");
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
    headers << "Coupons" << "Divs" << "Commission" << "Tax" << "Input, 10^3" << "Out, 10^3" << "CL Input";
    headers << "N repayments" << "N coupons" << "N records";
    m_statBox->setHeaderLabels(headers, Qt::Vertical);
    int n = m_statBox->table()->rowCount();
    for (int i=0; i<n; i++)
        LTable::createTableItem(m_statBox->table(), i, 0, "-");

    for (int i=0; i<3; i++)
        m_statBox->table()->item(n-i-1, 0)->setBackground(QColor("#E0FFFF"));

    h_splitter->insertWidget(1, stat_widget);
    m_statBox->resizeByContents();


    ///////////////init calc result paper lineedit////////////////////
    initPaperResultWidget();

}
void APIEventsPage::initPaperResultWidget()
{
    QHBoxLayout *h_lay = new QHBoxLayout(0);
    qobject_cast<QBoxLayout*>(m_statBox->layout())->addLayout(h_lay);
    h_lay->addWidget(new QLabel("Result of paper", this));

    m_paperResultEdit = new QLineEdit(this);
    m_paperResultEdit->setReadOnly(true);
    h_lay->addWidget(m_paperResultEdit);

    recalcPaperResult();
}
void APIEventsPage::slotGetEventsHistoryByTicker(const QString &ticker, QStringList &list)
{
    qDebug("APIEventsPage::slotGetEventsHistoryByTicker");
    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    for (int i=0; i<n; i++)
    {
        if (ticker == t->item(i, TICKER_COL)->text().trimmed())
        {
            QString kind = t->item(i, KIND_COL)->text().trimmed();
            if (kind == "SELL" || kind == "BUY")
            {
                //headers table (for asset info box) << "Date" << "Operation" << "Count" << "Result";
                QString s_date = t->item(i, DATE_COL)->text().trimmed();
                int pos = s_date.indexOf(LString::spaceSymbol());
                if (pos > 0) s_date = s_date.left(pos);
                QString line_row = QString("%1;%2").arg(s_date).arg(kind);
                line_row = QString("%1;%2").arg(line_row).arg(t->item(i, N_COL)->text().trimmed());
                line_row = QString("%1;%2").arg(line_row).arg(t->item(i, N_COL+1)->text().trimmed());
                list.append(line_row);
            }
        }
    }
}
void APIEventsPage::slotLoadEvents(const QJsonObject &j_obj)
{
    m_events.clear();
    if (j_obj.isEmpty()) return;
    loadDataFromFile();

    const QJsonValue &j_operations = j_obj.value("operations");
    if (!j_operations.isArray()) return;

    const QJsonArray &j_arr = j_operations.toArray();
    int n = j_arr.count();
    //qDebug()<<QString(" APIEventsPage::slotLoadEvents json_arr_size %1").arg(n);

    emit signalMsg(QString("received records from API-server: %1").arg(n));
    for (int i=0; i<n; i++)
    {
        EventOperation e;
        e.fromJson(j_arr.at(i));
        if (e.invalid())
        {
            qWarning()<<QString("APIEventsPage::slotLoadEvents - WARNING: invalid event record, i=%1").arg(i);
            qWarning()<<"    "<<e.toStr();
        }
        else
        {
            checkCloneUid(e);
            syncRecByFile(e);
        }
    }
    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_events.count()));

    sortByDate();
    reloadTableByData();
    findUnknownUID();
    recalcStat();

    m_kindFilterControl->setCurrentIndex(3); //default filter by [!COMMISION]

    ////////////send data to yieldpage/////////////
    emit signalSendDataToYieldStat(m_events);
}
void APIEventsPage::checkCloneUid(EventOperation &e)
{
    if (api_commonSettings.isCloneUid(e.uid))
    {
        emit signalMsg(QString("find clone UID - [%1]").arg(e.uid));
        QString orig_uid = api_commonSettings.getOrigUidByClone(e.uid).trimmed();
        if (orig_uid.isEmpty()) qWarning()<<QString("APIEventsPage::checkCloneUid WARNING orig_uid is empty by clone [%1]").arg(e.uid);
        else e.uid = orig_uid;
    }
}
void APIEventsPage::findUnknownUID()
{
    if (m_events.isEmpty()) return;

    QStringList umknown_uids;
    QStringList orig_uids;

    QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        if (t->item(i, TICKER_COL)->text() == "---")
        {
            QString s_kind = t->item(i, KIND_COL)->text();
            if (s_kind != "OUT" && s_kind != "INPUT" && s_kind != "TAX")
            {
                umknown_uids << t->item(i, 0)->data(Qt::UserRole).toString();
                LTable::setTableRowColor(t, i, Qt::red);

                QString s_orig = "???";
                QString s_date = t->item(i, DATE_COL)->text();
                QString s_sum = t->item(i, AMOUNT_COL)->text();

                for (int j=0; j<t->rowCount(); j++)
                {
                    if (j == i) continue;
                    if (t->item(j, KIND_COL)->text() != s_kind) continue;
                    if (t->item(j, DATE_COL)->text() != s_date) continue;
                    if (t->item(j, AMOUNT_COL)->text() != s_sum) continue;
                    s_orig = t->item(j, 0)->data(Qt::UserRole).toString();
                    s_orig += QString(" / %1 / %2").arg(t->item(j, TICKER_COL)->text()).arg(t->item(j, COMPANY_COL)->text());
                    break;
                }
                orig_uids.append(s_orig);
            }
        }
    }

    if (!umknown_uids.isEmpty())
    {
        emit signalError(QString("APIEventsPage: finded %1 unknown UID events").arg(umknown_uids.count()));
        for (int i=0; i<umknown_uids.count(); i++)
            emit signalError(QString("%1 : [%2]").arg(umknown_uids.at(i)).arg(orig_uids.at(i)));
    }
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

        if (!cur_date.isValid() || cur_date != rec.date.date())
        {
            if (row_color == COLOR_DAY_EVEN) row_color = COLOR_DAY_ODD;
            else row_color = COLOR_DAY_EVEN;
            cur_date = rec.date.date();
        }

        addRowRecord(rec, pair, QColor(row_color));
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#D3D3D3", "#2F4F4F");
    m_tableBox->searchExec();    
}
void APIEventsPage::addRowRecord(const EventOperation &rec, const QPair<QString, QString> &info, QColor row_color)
{

    QStringList row_data(rec.toTableRowData());
    row_data.insert(1, info.second);
    row_data.insert(1, info.first);

    QTableWidget *t = m_tableBox->table();
    LTable::addTableRow(t, row_data);
    int l_row = t->rowCount() - 1;
    if (rec.amount < 0) t->item(l_row, AMOUNT_COL)->setTextColor(Qt::darkRed);

    if (rec.date.date() == QDate::currentDate()) row_color = QColor("#00FFFF");
    LTable::setTableRowColor(t, l_row, row_color);

    //kind operation
    if (rec.strKind().toLower().trimmed() == "sell")
        t->item(l_row, KIND_COL)->setTextColor(QColor("#8B0000"));

    t->item(l_row, 0)->setData(Qt::UserRole, rec.uid);
}
void APIEventsPage::load(QSettings &settings)
{
    APITablePageBase::load(settings);
    for(int i=0; i<children().count(); i++)
    {
        if (h_splitter->children().at(i)->objectName() == "events_stat_widget")
        {
            LSimpleWidget *stat_widget = qobject_cast<LSimpleWidget*>(h_splitter->children().at(i));
            if (stat_widget) stat_widget->load(settings);
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
            if (stat_widget) stat_widget->save(settings);
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
    list << NONE_FILTER << "CANCELED" << "COMMISSION" << "!COMMISSION" << "BUY" << "SELL" << "COUPON" << "DIV" << "TAX" << "INPUT" << "OUT" << "REPAYMENT";
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

    connect(m_paperTypeFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSearched()));
    connect(m_kindFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSearched()));
    connect(m_dateFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSearched()));
}

/*
bool APIEventsPage::isSatisfiesFilter(int row) const
{
    const QTableWidget *t = m_tableBox->table();
    if (row < 0 || row >= t->rowCount()) return false;

    bool need_hide = false;
    paperTypeFilter(row, need_hide);
    if (need_hide) return false;
    kindFilter(row, need_hide);
    if (need_hide) return false;
    dateFilter(row, need_hide);
    if (need_hide) return false;

    return true;
}
*/
void APIEventsPage::applyFilterRow(int row)
{
    if (row < 0 || row >= m_tableBox->table()->rowCount()) return;

    bool need_hide = false;
    paperTypeFilter(row, need_hide);
    if (need_hide) return;
    kindFilter(row, need_hide);
    if (need_hide) return;
    dateFilter(row, need_hide);
}
/*
void APIEventsPage::slotFilter()
{
    if (!sender()) return;
    qDebug()<<QString("APIEventsPage::slotFilter()");

    const QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n < 5) return;

    m_tableBox->searchExec();
    for (int i=0; i<n; i++)
    {
        if (t->isRowHidden(i)) continue;
        applyFilter(i);


        bool was_hiden = false;
        paperTypeFilter(i, was_hiden);
        if (was_hiden) continue;
        kindFilter(i, was_hiden);
        if (was_hiden) continue;
        dateFilter(i, was_hiden);
        if (was_hiden) continue;

    }

    updateSearchLabel();
    recalcStat();
    recalcPaperResult();
}
*/
void APIEventsPage::slotFilter()
{
    //qDebug("APIEventsPage::slotSearched()");
    filterAfterSearch();
    updateSearchLabel();
    recalcStat();
    recalcPaperResult();
}
void APIEventsPage::slotSearched()
{
    if (m_tableBox)
        m_tableBox->searchExec();
}
void APIEventsPage::filterAfterSearch()
{
    const QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        if (t->isRowHidden(i)) continue;
        applyFilterRow(i);
    }
}
void APIEventsPage::updateSearchLabel()
{
    QTableWidget *t = this->m_tableBox->table();
    quint32 n = 0;
    for (int i=0; i<t->rowCount(); i++)
        if (!t->isRowHidden(i)) n++;

    QString s = QString("Record number: %1/%2").arg(n).arg(t->rowCount());
    m_tableBox->setTextLabel(s);
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

    bool invert = (text.left(1) == "!");
    if (invert)
    {
        text = LString::strTrimLeft(text, 1);
        if (m_tableBox->table()->item(row, KIND_COL)->text() == text)
        {
            m_tableBox->table()->hideRow(row);
            need_hide = true;
        }
    }
    else
    {
        if (m_tableBox->table()->item(row, KIND_COL)->text() != text)
        {
            m_tableBox->table()->hideRow(row);
            need_hide = true;
        }
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
    if (m_events.at(row).date.date() < limit_date)
    {
        m_tableBox->table()->hideRow(row);
        need_hide = true;
    }
}
void APIEventsPage::updateStatStruct(const EventOperation &rec)
{
    m_stat.n_records++;

    switch (rec.kind)
    {
        case EventOperation::etCoupon:
        {
            m_stat.coupon += rec.amount;
            m_stat.n_coupons++;
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
        case EventOperation::etRepayment:
        {
            m_stat.n_repayments++;
            break;
        }
        default: break;
    }
}
void APIEventsPage::updateStatTable()
{
    int row = 0;
    QTableWidget *t = m_statBox->table();
    t->item(row, 0)->setText(QString::number(m_stat.coupon, 'f', 1)); row++;
    t->item(row, 0)->setText(QString::number(m_stat.div, 'f', 1)); row++;
    t->item(row, 0)->setTextColor(Qt::darkRed);
    t->item(row, 0)->setText(QString::number(qAbs(m_stat.commission), 'f', 1)); row++;
    t->item(row, 0)->setTextColor(Qt::darkRed);
    t->item(row, 0)->setText(QString::number(qAbs(m_stat.tax), 'f', 1)); row++;

    t->item(row, 0)->setText(QString::number(m_stat.input/float(1000), 'f', 1)); row++;
    t->item(row, 0)->setText(QString::number(qAbs(m_stat.out/float(1000)), 'f', 1)); row++;
    t->item(row, 0)->setText(QString::number((m_stat.input/float(1000)) - qAbs(m_stat.out/float(1000)), 'f', 1)); row++;

    t->item(row, 0)->setText(QString::number(m_stat.n_repayments)); row++;
    t->item(row, 0)->setText(QString::number(m_stat.n_coupons)); row++;
    t->item(row, 0)->setText(QString::number(m_stat.n_records)); row++;

    m_statBox->resizeByContents();
}
void APIEventsPage::resetPage()
{
    APITablePageBase::resetPage();
    m_paperTypeFilterControl->setCurrentIndex(0);
    m_kindFilterControl->setCurrentIndex(0);
    m_dateFilterControl->setCurrentIndex(0);
}
void APIEventsPage::recalcPaperResult()
{
    //qDebug("APIEventsPage::recalcPaperResult()");
    QPalette plt;
    QPalette::ColorRole role = QPalette::Base;
    plt.setColor(role, Qt::lightGray);
    m_paperResultEdit->clear();
    m_paperResultEdit->setPalette(plt);

    QString kks;
    QTableWidget *t = this->m_tableBox->table();
    if (t->rowCount() <= 0) return;

    bool ok;
    float result = 0;
    quint16 n_ps = 0;
    for (int i=0; i<t->rowCount(); i++)
    {
        if (t->isRowHidden(i)) continue;
        float a = t->item(i, AMOUNT_COL)->text().toFloat(&ok);
        if (ok) result += a;
        if (t->item(i, KIND_COL)->text().trimmed() == "BUY")
            n_ps += t->item(i, N_COL)->text().toUInt();


        if (kks.isEmpty()) {kks = t->item(i, TICKER_COL)->text().trimmed(); continue;}
        if (kks != t->item(i, TICKER_COL)->text()) return;
    }
    if (result == 0) return;

    qDebug()<<QString("APIEventsPage::recalcPaperResult() result=%1").arg(result);

    plt.setColor(role, Qt::yellow);
    m_paperResultEdit->setPalette(plt);
    m_paperResultEdit->setText(QString("%1 (%2ps)").arg(QString::number(result, 'f', 1)).arg(n_ps));
}
QString APIEventsPage::dataFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(EventOperation::dataFile());
}
void APIEventsPage::loadDataFromFile()
{
    emit signalMsg(QString("OPEN FILE: %1").arg(APIEventsPage::dataFile()));
    QStringList list;
    QString err = LFile::readFileSL(APIEventsPage::dataFile(), list);
    if (!err.isEmpty()) {emit signalError(err); return;}
    emit signalMsg(QString("FILE_LIST_SIZE: %1").arg(list.count()));


    int n_invalid = 0;
    foreach (const QString &v, list)
    {
        if (v.trimmed().isEmpty()) continue;
        if (!v.contains("/")) continue;

        EventOperation rec;
        rec.fromFileLine(v);
        if (!rec.invalid()) m_events.append(rec);
        else
        {
            qWarning() << QString("INVALID EVENTS LINE: [%1]").arg(v); n_invalid++;
            qDebug()<<QString("kind=%1  date validity: %2,  time validity: %3").arg(rec.kind).
                      arg(rec.date.date().isValid()?"ok":"invalid").arg(rec.date.time().isValid()?"ok":"invalid");
        }
    }

    emit signalMsg(QString("found invalid lines: %1/%2").arg(n_invalid).arg(m_events.count()));
}
void APIEventsPage::syncRecByFile(const EventOperation &rec)
{
    //check contains rec in m_events
    bool is_new = true;
    foreach (const EventOperation &v, m_events)
        if (v.isSame(rec)) {is_new=false; break;}
    if (!is_new) return;

    //check filedata
    QString err;
    QString fname(dataFile());
    if (!LFile::fileExists(fname))
    {
        emit signalMsg(QString("Need create datafile: %1").arg(fname));
        err = LFile::writeFile(fname, QString("OPERATIONS HISTORY: \n"));
        if (!err.isEmpty()) {emit signalError(err); return;}
    }

    //add data
    m_events.append(rec);

    //check date
    if (rec.date.date() == QDate::currentDate()) return; // it is rec was received today (fresh)

    QString fline(rec.toStr());
    emit signalMsg(QString("Add new EVENT record: %1").arg(fline));
    err = LFile::appendFile(fname, QString("%1.  %2 \n").arg(m_events.count()).arg(fline));
    if (!err.isEmpty()) {emit signalError(err); return;}
}
void APIEventsPage::sortByDate()
{
    if (m_events.count() < 3) return;

    int n = m_events.count();
    while (1 == 1)
    {
        bool has_replace = false;
        for (int i=0; i<n-1; i++)
        {
            const QDateTime &dt = m_events.at(i).date;
            const QDateTime &dt_next = m_events.at(i+1).date;
            if (dt_next > dt)
            {
                EventOperation rec = m_events.takeAt(i+1);
                m_events.insert(i, rec);
                has_replace = true;
            }
        }
        if (!has_replace) break;
    }
}



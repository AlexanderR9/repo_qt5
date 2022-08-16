#include "divpage.h"
#include "ltable.h"
#include "lsearch.h"
#include "logpage.h"
#include "lstatic.h"
#include "lfile.h"
#include "cfdcalcobj.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QDir>

#define TICKER_COL          1
#define SIZE_COL            2
#define PRICE_COL           3
#define DAYS_COL            4
#define INSTA_COL           5
#define INSTA_COLOR         QColor(200, 100, 30)
#define DATE_PREV_COLOR     QColor(Qt::lightGray)
#define EX_DATE_MASK        QString("dd_MM_yyyy")




//DivPage
DivPage::DivPage(QWidget *parent)
    :BasePage(parent),
    m_search(NULL),
    m_table(NULL),
    m_timer(NULL),
    m_interval(-1),
    m_shownHistory(200),
    m_lookDays(30),
    m_tablePriceIndex(0)
{
    setupUi(this);

    initSearch();
    initTable();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    setTickTimerInterval(90);
}
void DivPage::setTickTimerInterval(int t)
{
    if (t < 10 || t > 1000) t = 150;
    m_timer->setInterval(t*1000);
}
void DivPage::tickTimerStart()
{
    m_timer->start();
}
void DivPage::tickTimerStop()
{
    m_timer->stop();
}
void DivPage::testDivDataFromFile()
{
    QString f_name = QString("/home/roman/tmp/div.txt");
    QString plain_data;
    QString err = LFile::readFileStr(f_name, plain_data);
    if (!err.isEmpty())
    {
        qWarning()<<QString("DivPage::testDivDataFromFile() WARNING: %1").arg(err);
        return;
    }

    slotDivDataReceived(plain_data);
}
void DivPage::slotTimer()
{
    if (invalidParams()) return;

    if (!m_lastDT.isValid())
    {
        qDebug("m_lastDT invalid");
        m_lastDT = QDateTime::currentDateTime();
        updateTable();
        //testDivDataFromFile();
        return;
    }
    qint64 t = m_lastDT.secsTo(QDateTime::currentDateTime());
    qDebug()<<QString("cur d_time: %1").arg(t);
    if (t < m_interval)
    {
        updateTableNextPrice();
        return;
    }

    //////////// send request //////////////////
    sendLog("Try get div data", 0);
    emit signalMsg(QString("Try get div data, URL: [%1] ....................").arg(m_url));
    emit signalGetDivData(m_url);
}
void DivPage::slotDivDataReceived(const QString &plain_data)
{
    //qDebug()<<QString("DivPage::slotDivDataReceived: plain_data_size %1").arg(plain_data.size());
    m_lastDT = QDateTime::currentDateTime();
    QStringList list = LStatic::trimSplitList(plain_data);
    if (list.count() < 10)
    {
        sendLog("Plain data too small", 2);
        return;
    }

    QList<DivRecord> div_data;
    QDate cur_date = QDate::currentDate();
    QDate last_div_date;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        s = LStatic::removeLongSpaces(s, true);

        QDate div_date;
        parseDate(s, div_date);
        if (div_date.isValid())
        {
            last_div_date = div_date;
            if (cur_date.daysTo(div_date) > m_lookDays)
            {
                qDebug()<<QString("ex_date is over:  %1").arg(div_date.toString("dd.MM.yyyy"));
                break;
            }
            continue;
        }

        if (!last_div_date.isValid() || last_div_date < cur_date) continue;
        if (s.indexOf("ISIN") == 0) continue;

        DivRecord rec;
        rec.ex_date = last_div_date.addDays(-1);
        if (rec.ex_date.dayOfWeek() == 7) rec.ex_date = rec.ex_date.addDays(-2);
        else if (rec.ex_date.dayOfWeek() == 6) rec.ex_date = rec.ex_date.addDays(-1);

        parseDivSize(s, rec);
        if (!rec.invalid()) div_data.append(rec);
    }

    updateLastPrices(div_data);
    divDataReceived(div_data);
}
void DivPage::divDataReceived(const QList<DivRecord> &data)
{
    if (data.isEmpty())
    {
        sendLog("Received empty data", 2);
        return;
    }

    sendLog(QString("Finded %1 validity div_records").arg(data.count()), 0);
    updateFileByReceivedData(data);
    updateTable();

}
void DivPage::updateFileByReceivedData(const QList<DivRecord> &received_data)
{
    if (received_data.isEmpty()) return;

    QList<DivRecord> cur_data;
    loadDivFile(cur_data);
    //qDebug()<<QString("DivPage::updateFileByReceivedData - loaded records from file: %1 ").arg(cur_data.count());


    int n_added = 0;
    for (int i=0; i<received_data.count(); i++)
    {
        const DivRecord &new_rec = received_data.at(i);
        if (cur_data.isEmpty())
        {
            addRecToFile(new_rec);
            n_added++;
            continue;
        }

        bool need_add = true;
        for (int j=0; j<cur_data.count(); j++)
        {
            if (cur_data.at(j).isEqual(new_rec))
            {
                need_add = false;
                break;
            }
        }

        if (need_add) {addRecToFile(new_rec); n_added++;}
    }

    if (n_added > 0)
        sendLog(QString("Add %1 new records to div_file").arg(n_added), 0);


    //qDebug()<<QString("DivPage::updateFileByReceivedData - was added records: %1 ").arg(n_added);
}
void DivPage::updateLastPrices(QList<DivRecord> &data)
{
    QMap<QString, double> map;
    emit signalGetCurrentPrices(map);

    for (int i=0; i<data.count(); i++)
    {
        QString ticker = data.at(i).ticker;
        if (map.contains(ticker))
            data[i].price = map.value(ticker);
    }
}
void DivPage::parseDivSize(const QString &s, DivRecord &rec)
{
    if (s.right(3) != "USD") return;
    QString res = LStatic::strTrimRight(s, 3).trimmed();

    bool ok;
    int n = sourcesListWidget->count();
    for (int i=0; i<n; i++)
    {
        QString ticker = sourcesListWidget->item(i)->text().trimmed();
        if (ticker.isEmpty()) continue;
        int ticker_pos = res.indexOf(QString(" %1 ").arg(ticker));
        if (ticker_pos < 0 || ticker_pos > 20) continue;

        int pos = res.lastIndexOf("%");
        if (pos < 0 || (pos - ticker_pos) < 12) continue;

        QString s_size = res.right(res.length()-pos-1).trimmed();
        s_size.replace(",", ".");
        res = res.left(pos).trimmed();

        pos = res.lastIndexOf(LStatic::spaceSymbol());
        if (pos < 0) continue;

        QString s_size_p = res.right(res.length()-pos).trimmed();
        s_size_p.replace(",", ".");

        rec.ticker = ticker;
        rec.size = s_size.toDouble(&ok); if (!ok) rec.size = -2;
        rec.size_p = s_size_p.toDouble(&ok); if (!ok) rec.size_p = -2;
        break;
    }
}
void DivPage::parseDate(const QString &s, QDate &date)
{
    date = QDate();
    if (s.trimmed().length() > 15) return;

    QDate cur_date = QDate::currentDate();
    QString cur_month = QDate::longMonthName(cur_date.month(), QDate::StandaloneFormat);
    cur_month = cur_month.toLower().trimmed();
    int nm = ((cur_date.month() == 12) ? 1 : (cur_date.month() + 1));
    QString next_month = QDate::longMonthName(nm, QDate::StandaloneFormat);

    int pos = s.indexOf(",");
    if (pos > 0)
    {
        bool ok;
        int day = s.left(pos).trimmed().toInt(&ok);
        if (ok && day > 0 && day < 32)
        {
            QString finded_month = LStatic::strTrimLeft(s, pos+1).trimmed().toLower();
            if (finded_month == cur_month) date.setDate(cur_date.year(), cur_date.month(), day);
            else if (finded_month == next_month) date.setDate(cur_date.year(), nm, day);
        }
    }

    if (date.isValid())
    {
        if (date <= cur_date) date = QDate();
    }
}
void DivPage::setReqParams(const QString &url, int t, quint16 days)
{
    m_url = url.trimmed();
    m_interval = t;
    m_lookDays = days;
    if (m_interval < 3600) m_interval = -1;

    //m_interval = 60*7;
    //qDebug()<<QString("DivPage::setReqParams()  interval=%1  look_days=%2  URL: %3").arg(m_interval).arg(m_lookDays).arg(url);
}
void DivPage::slotSelectionChanged()
{
    //qDebug()<<QString("DivPage::slotSelectionChanged()");
    QList<QListWidgetItem*> items = sourcesListWidget->selectedItems();
    int n = items.count();

    if (n == 0)
    {
        m_table->showAllRows();
        return;
    }

    QStringList sel_tickers;
    foreach (const QListWidgetItem *it, items)
        sel_tickers << it->text();

    int n_rows = m_table->rowCount();
    for (int i=0; i<n_rows; i++)
        m_table->updateRowVisible(i, sel_tickers);

    //qDebug()<<QString("DivPage::slotSelectionChanged()  selected tickers %1").arg(n);
}
void DivPage::initTable()
{
    m_table = new DivTable(this);

    chartBox->setTitle("Calendar");
    if (chartBox->layout()) delete chartBox->layout();
    QHBoxLayout *h_lay = new QHBoxLayout(0);
    chartBox->setLayout(h_lay);
    h_lay->addWidget(m_table);

    QStringList headers;
    headers << "ExDate" << "Ticker" << "Div size" << "Price" << "Days_to" << "Insta";
    LTable::setTableHeaders(m_table, headers);

    connect(m_table, SIGNAL(signalDoubleClicked()), sourcesListWidget, SLOT(clearSelection()));
    connect(m_table, SIGNAL(signalGetInstaPtr(const QString&, bool&)), this, SIGNAL(signalGetInstaPtr(const QString&, bool&)));

}
void DivPage::initSearch()
{
    m_search = new LSearch(searchLineEdit, this);
    m_search->addList(sourcesListWidget, countLabel);
    searchExec();

    QPalette p(sourcesListWidget->palette());
    p.setColor(QPalette::Foreground, Qt::red);
    p.setColor(QPalette::Text, QColor(200, 100, 30));
    sourcesListWidget->setPalette(p);
}
void DivPage::initSource()
{
    sourcesListWidget->clearSelection();
    QStringList list;
    emit signalGetSource(list);
    if (list.isEmpty())
    {
        emit signalError("DivPage: source list is empty");
        sendLog("Source list is empty", 1);
        return;
    }

    sourcesListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    sourcesListWidget->addItems(list);
    searchExec();
    sendLog(QString("Source list loaded OK"), 0);

    connect(sourcesListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
}
void DivPage::searchExec()
{
    m_search->exec();
    countLabel->setText(countLabel->text().remove("Record").trimmed());
}
void DivPage::sendLog(const QString &text, int result)
{
    LogStruct log(amtDivPage, result);
    log.msg = text;
    emit signalSendLog(log);
}
void DivPage::loadDivFile(QList<DivRecord> &data)
{
    data.clear();
    QString f_name = QString("%1%2%3").arg(CFDCalcObj::dataFolderPath()).arg(QDir::separator()).arg(divFile());
    if (!LFile::fileExists(f_name))
    {
        emit signalError(QString("div data file not found: %1").arg(f_name));
        sendLog("Div file not exist", 2);
        return;
    }

    QStringList list;
    QString err = LFile::readFileSL(f_name, list);
    if (!err.isEmpty())
    {
        emit signalError(QString("DIV_PAGE: %1").arg(err));
        sendLog("Can't read div file", 1);
        return;
    }

    for (int i=0; i<list.count(); i++)
    {
        QString f_line = list.at(i).trimmed();
        if (f_line.isEmpty()) continue;

        DivRecord rec;
        rec.fromFileLine(f_line);
        if (!rec.invalid()) data.append(rec);
        else qWarning() << "loaded invalid rec:  " << rec.toStr();
    }
}
void DivPage::updateTable()
{
    QList<DivRecord> cur_data;
    loadDivFile(cur_data);
    sortData(cur_data);
    if (cur_data.count() > m_shownHistory)
    {
        while (cur_data.count() > m_shownHistory)
            cur_data.removeLast();
    }

    reloadTable(cur_data);
}
void DivPage::addRecToFile(const DivRecord &rec)
{
    if (rec.invalid()) return;

    QString f_name = QString("%1%2%3").arg(CFDCalcObj::dataFolderPath()).arg(QDir::separator()).arg(divFile());
    QString f_line = QString("%1 \n").arg(rec.toFileLine());
    QString err = LFile::appendFile(f_name, f_line);
    if (!err.isEmpty())
        emit signalError(QString("DIV_PAGE: %1").arg(err));
}
void DivPage::sortData(QList<DivRecord> &data)
{
    int n = data.count();
    if (n < 3) return;

    int start_i = 0;
    while (start_i < (n-1))
    {
        QDate max_date;
        for (int i=start_i; i<n; i++)
        {
            if (!max_date.isValid()) max_date = data.at(i).ex_date;
            else if (data.at(i).ex_date > max_date) max_date = data.at(i).ex_date;
        }

        int n_replaced = 0;
        for (int i=start_i; i<n; i++)
        {
            if (data.at(i).ex_date == max_date)
            {
                replaceRecords(i, start_i+n_replaced, data);
                n_replaced++;
            }
        }

        start_i += n_replaced;
        if (n_replaced == 0)
        {
            qWarning()<<QString("DivPage::sortData WARNING: n_replaced == 0");
            break;
        }
    }
}
void DivPage::replaceRecords(int i1, int i2, QList<DivRecord> &data)
{
    if (i1 == i2 || data.isEmpty()) return;
    if (i1 < 0 || i1 >= data.count()) return;
    if (i2 < 0 || i2 >= data.count()) return;

    DivRecord rec1(data.at(i1));
    DivRecord rec2(data.at(i2));
    data.replace(i1, rec2);
    data.replace(i2, rec1);
}
void DivPage::reloadTable(const QList<DivRecord> &data)
{
    LTable::removeAllRowsTable(m_table);

    for (int i=0; i<data.count(); i++)
        m_table->addRecord(data.at(i));

    LTable::resizeTableContents(m_table);

    QString title;
    m_table->getTableTitle(title);
    this->chartBox->setTitle(title);
}
void DivPage::updateTableNextPrice()
{
    if (m_table->rowCount() == 0) return;
    if (m_tablePriceIndex >= m_table->rowCount()) m_tablePriceIndex = 0;

    double price = 0;
    int hours_ago;
    QString ticker = m_table->item(m_tablePriceIndex, TICKER_COL)->text();
    emit signalGetLastPrice(ticker, price, hours_ago);
    QString s_price = ((price > 5) ? QString::number(price, 'f', 2) : QString::number(-2));
    m_table->item(m_tablePriceIndex, PRICE_COL)->setText(s_price);
    //qDebug()<<QString("DivPage::updateTableNextPrice - %1:  price=%2/%3  table_row=%4").arg(ticker).arg(s_price).arg(QString::number(price, 'f', 2)).arg(m_tablePriceIndex);
    m_tablePriceIndex++;
}


//DivTable
DivTable::DivTable(QWidget *parent)
    :QTableWidget(parent),
      m_lightDivSize(0.6)
{
    setObjectName("div_table");
    verticalHeader()->hide();
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}
void DivTable::mouseDoubleClickEvent(QMouseEvent*)
{
    emit signalDoubleClicked();
}
void DivTable::addRecord(const DivRecord &rec)
{
    QStringList row_data;
    row_data.append(rec.ex_date.toString("dd.MM.yyyy"));
    row_data.append(rec.ticker);
    row_data << QString("%1 (%2%)").arg(QString::number(rec.size, 'f', 2)).arg(QString::number(rec.size_p, 'f', 2));
    row_data << QString::number(rec.price, 'f', 2);
    row_data << QString::number(daysTo(rec.ex_date));

    bool is_insta= false;
    emit signalGetInstaPtr(rec.ticker, is_insta);
    row_data.append(is_insta ? "yes" : "no");

    LTable::addTableRow(this, row_data);

    updateColors(rec);
}
void DivTable::updateColors(const DivRecord &rec)
{
    int cols = columnCount();
    int cur_row = rowCount() -1;
    int days = daysTo(rec.ex_date);
    if (days < 0)
    {
        for (int j=0; j<cols; j++)
            item(cur_row, j)->setTextColor(DATE_PREV_COLOR);
        return;
    }

    if (item(cur_row, INSTA_COL)->text() == "yes")
    {
        item(cur_row, INSTA_COL)->setTextColor(INSTA_COLOR);
        item(cur_row, TICKER_COL)->setTextColor(INSTA_COLOR);
    }

    if (days == 0)
    {
        item(cur_row, DAYS_COL)->setText("now");
        item(cur_row, DAYS_COL)->setTextColor(Qt::darkRed);
    }
    else if (days == 1)
    {
        item(cur_row, DAYS_COL)->setText("tomorrow");
        item(cur_row, DAYS_COL)->setTextColor(Qt::darkGreen);
    }

    if (rec.price > 150)
        item(cur_row, PRICE_COL)->setTextColor(Qt::red);

    if (rec.size_p > m_lightDivSize)
        item(cur_row, SIZE_COL)->setTextColor(Qt::blue);

}
int DivTable::daysTo(const QDate &d) const
{
    if (!d.isValid()) return -2;
    QDate cur_date = QDate::currentDate();
    if (d < cur_date) return -1;
    if (d == cur_date) return 0;
    return cur_date.daysTo(d);
}
void DivTable::getTableTitle(QString &s)
{
    s = QString("Calendar");
    int rows = rowCount();

    int n_prev = 0;
    for (int i=0; i<rows; i++)
        if (item(i, 0)->textColor() == DATE_PREV_COLOR) n_prev++;

    s = QString("%1  (records: %2/%3)  ").arg(s).arg(rows).arg(n_prev);
}
void DivTable::updateRowVisible(int row_index, const QStringList &list)
{
    if (row_index < 0 || row_index >= rowCount() || list.isEmpty()) return;
    if (list.contains(item(row_index, TICKER_COL)->text())) showRow(row_index);
    else hideRow(row_index);
}
void DivTable::showAllRows()
{
    int n_rows = rowCount();
    for (int i=0; i<n_rows; i++)
        showRow(i);
}



//DivRecord
DivRecord::DivRecord(const DivRecord &rec)
    :ex_date(rec.ex_date),
    ticker(rec.ticker),
    size(rec.size),
    size_p(rec.size_p),
    price(rec.price)
{

}
QString DivRecord::toFileLine() const
{
    QString s = QString("%1 / %2").arg(ex_date.toString(EX_DATE_MASK)).arg(ticker);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(QString::number(size)).arg(QString::number(size_p)).arg(QString::number(price));
    return s;
}
void DivRecord::fromFileLine(const QString &f_line)
{
    reset();
    if (f_line.trimmed().isEmpty()) return;

    QStringList list = LStatic::trimSplitList(f_line, QString("/"));
    if (list.count() != DivRecord::fieldsCount())
    {
        qWarning()<<QString("DivRecord::fromFileLine WARNING: list.count(%1)=fieldsCount(%2)  f_line: %3").arg(list.count()).arg(DivRecord::fieldsCount()).arg(f_line);
        return;
    }

    bool ok;
    ex_date = QDate::fromString(list.at(0).trimmed(), EX_DATE_MASK);
    ticker = list.at(1).trimmed();
    size = list.at(2).trimmed().toDouble(&ok);
    if (!ok) size = -1;
    size_p = list.at(3).trimmed().toDouble(&ok);
    if (!ok) size_p = -1;
    price = list.at(4).trimmed().toDouble(&ok);
    if (!ok) price = -1;
}
QString DivRecord::toStr() const
{
    QString s = QString("DIV_REC: %1  %2").arg(ex_date.toString("dd.MM.yyyy")).arg(ticker);
    s = QString("%1   size=%2(%3%)  price=%4").arg(s).arg(QString::number(size, 'f', 2)).arg(QString::number(size_p, 'f', 2)).arg(QString::number(price, 'f', 2));
    return s;
}


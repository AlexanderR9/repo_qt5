#include "cfdpage.h"
#include "lstatic.h"
#include "ltable.h"
#include "lsearch.h"

#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>


#define TICKER_COL      2
#define DAY_COL         3
#define WEEK_COL        4
#define MONTH_COL       5
#define PRICE_COL       6



//CFDPage statistic
CFDPage::CFDPage(QWidget *parent)
    :BasePage(parent),
      m_table(NULL),
      m_searchEdit(NULL),
      m_search(NULL)
{
    init();
    initSearch();

    QHeaderView *hv = m_table->horizontalHeader();
    if (hv)
    {
        for (int j=0; j<m_table->columnCount(); j++)
            m_table->horizontalHeaderItem(j)->setData(Qt::UserRole, int(0));

        connect(hv, SIGNAL(sectionClicked(int)), this, SLOT(slotSortByColumn(int)));
    }
}
void CFDPage::slotSortByColumn(int col)
{
    //qDebug()<<QString("slotSortByColumn  col=%1").arg(col);
    if (col > 2)
    {
        int sort_order = m_table->horizontalHeaderItem(col)->data(Qt::UserRole).toInt();
        if (sort_order != 0) m_table->horizontalHeaderItem(col)->setData(Qt::UserRole, int(0));
        else m_table->horizontalHeaderItem(col)->setData(Qt::UserRole, int(1));

        switch (sort_order)
        {
            case 0: {decreaseSortNum(col); break;}
            case 1: {increaseSortNum(col); break;}
            default:
            {
                qWarning()<<QString("CFDPage::slotSortByColumn: WARNING - invalid sort_order %1, col %2").arg(sort_order).arg(col);
                break;
            }
        }
    }
    else m_table->sortByColumn(col);

    LTable::resizeTableContents(m_table);
}
void CFDPage::initSearch()
{
    QLabel *c_label = new QLabel(this);
    layout()->addWidget(c_label);

    m_search = new LSearch(m_searchEdit, this);
    m_search->addTable(m_table, c_label);
    m_search->exec();
}
void CFDPage::slotNewPrice(const QStringList &row_data)
{
    if (row_data.count() != m_table->columnCount())
    {
        emit signalError(QString("CFDPage: newPriceData size(%1) != table columnCount(%2)").arg(row_data.count()).arg(m_table->columnCount()));
        return;
    }

    removeRowByTicker(row_data.at(TICKER_COL));
    LTable::insertTableRow(0, m_table, row_data);
    LTable::resizeTableContents(m_table);

    updatePage();
}
void CFDPage::slotSetCurrentPrices(QMap<QString, double> &map)
{
    map.clear();

    bool ok;
    int price_col = m_table->columnCount() - 1;
    for (int i=0; i<m_table->rowCount(); i++)
    {
        QString ticker = m_table->item(i, TICKER_COL)->text();
        double price = m_table->item(i, price_col)->text().toDouble(&ok);
        if (price > 10 && price < 500) map.insert(ticker, price);
    }
}
void CFDPage::updateCellColors()
{
    double value = 0;
    bool ok = false;
    QColor color = Qt::gray;
    int n = m_table->rowCount();
    for (int i=0; i<n; i++)
    {
        //day
        getCellValue(m_table->item(i, DAY_COL)->text(), value, ok);
        color = (ok ? getColorByLimits(value, 0.9) : Qt::gray);
        m_table->item(i, DAY_COL)->setTextColor(color);

        //week
        getCellValue(m_table->item(i, WEEK_COL)->text(), value, ok);
        color = (ok ? getColorByLimits(value, 6.9) : Qt::gray);
        m_table->item(i, WEEK_COL)->setTextColor(color);

        //month
        getCellValue(m_table->item(i, MONTH_COL)->text(), value, ok);
        color = (ok ? getColorByLimits(value, 15.5) : Qt::gray);
        m_table->item(i, MONTH_COL)->setTextColor(color);

        //check insta ticker
        emit signalGetInstaPtr(m_table->item(i, TICKER_COL)->text(), ok);
        color = (ok ? QColor(200, 100, 30) : Qt::black);
        m_table->item(i, TICKER_COL)->setTextColor(color);

        //price col
        color =QColor(160, 40, 40);
        m_table->item(i, PRICE_COL)->setTextColor(color);

    }
}
QColor CFDPage::getColorByLimits(const double &value, double limit) const
{
    if (value < -1*limit) return Qt::darkRed;
    else if (value > limit) return Qt::darkGreen;
    return Qt::black;
}
void CFDPage::getCellValue(const QString &cell_text, double &value, bool &ok)
{
    value = -1;
    ok = false;
    if (cell_text.right(1) != "%") return;

    QString s = LStatic::strTrimRight(cell_text, 1).trimmed();
    if (s.left(1) == "+") s = LStatic::strTrimLeft(s, 1).trimmed();
    value = s.toDouble(&ok);
}
void CFDPage::removeRowByTicker(const QString &ticker)
{
    int n = m_table->rowCount();
    for (int i=0; i<n ;i++)
    {
        if (m_table->item(i, TICKER_COL)->text() == ticker)
        {
            m_table->removeRow(i);
            break;
        }
    }
}
void CFDPage::updatePage()
{
    int n = m_table->rowCount();
    if (n <= 0) return;

    QStringList list;
    for (int i=0; i<n; i++)
        list.append(QString::number(i+1));
    LTable::setTableHeaders(m_table, list, Qt::Vertical);

    updateCellColors();
    m_search->exec();
}
QStringList CFDPage::headerLabels() const
{
    QStringList list;
    list << "Date" << "Time" << "Ticker" << "Day" << "Week" << "Month" << "Last price";
    return list;
}
void CFDPage::init()
{
    if (layout()) delete layout();
    QVBoxLayout *vlay = new QVBoxLayout(0);
    setLayout(vlay);

    m_searchEdit = new QLineEdit(this);
    vlay->addWidget(m_searchEdit);

    m_table = new QTableWidget(this);
    m_table->verticalHeader()->hide();
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    vlay->addWidget(m_table);


    LTable::fullClearTable(m_table);
    LTable::setTableHeaders(m_table, headerLabels());
    LTable::resizeTableContents(m_table);
}
void CFDPage::decreaseSortNum(int col)
{
    int n = m_table->rowCount();
    if (n < 2) return;

    double min = 0;
    Q_UNUSED(min);
    int row = -1;
    for (int i=0; i<n; i++)
    {
        min = LTable::minNumericColValue(m_table, col, row, i);
        if (row > 0) LTable::shiftTableRowToBegin(m_table, row);
    }
}
void CFDPage::increaseSortNum(int col)
{
    int n = m_table->rowCount();
    if (n < 2) return;

    double max = 0;
    Q_UNUSED(max);
    int row = -1;
    for (int i=0; i<n; i++)
    {
        max = LTable::maxNumericColValue(m_table, col, row, i);
        if (row > 0) LTable::shiftTableRowToBegin(m_table, row);
    }
}



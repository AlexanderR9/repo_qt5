#include "cfdpage.h"
#include "lstatic.h"
#include "ltable.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>


#define TICKER_COL      2
#define DAY_COL         3
#define WEEK_COL        4
#define MONTH_COL       5



//CFDPage statistic
CFDPage::CFDPage(QWidget *parent)
    :BasePage(parent),
      m_table(NULL)
{
    init();
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

    m_table = new QTableWidget(this);
    m_table->verticalHeader()->show();
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    vlay->addWidget(m_table);

    LTable::fullClearTable(m_table);
    LTable::setTableHeaders(m_table, headerLabels());
    LTable::resizeTableContents(m_table);
}



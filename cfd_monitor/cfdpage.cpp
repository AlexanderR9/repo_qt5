#include "cfdpage.h"
//#include "lstatic.h"
#include "ltable.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>


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

    removeRowByTicker(row_data.first());
    LTable::insertTableRow(0, m_table, row_data);
}
void CFDPage::removeRowByTicker(const QString &ticker)
{
    int index = -1;
    int n = m_table->rowCount();
    for (int i=0; i<n ;i++)
    {
        if (m_table->item(i, 0)->text() == ticker)
        {
            index = i;
            break;
        }
    }

    if (index >= 0)
    {
        m_table->removeRow(index);
        LTable::resizeTableContents(m_table);
    }
}
QStringList CFDPage::headerLabels() const
{
    QStringList list;
    list << "Ticker" << "Price" << "Day" << "Week" << "Month" << "Update datetime";
    return list;
}
void CFDPage::init()
{
    if (layout()) delete layout();
    QVBoxLayout *vlay = new QVBoxLayout(0);
    setLayout(vlay);

    m_table = new QTableWidget(this);
    m_table->verticalHeader()->hide();
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    vlay->addWidget(m_table);

    LTable::fullClearTable(m_table);
    LTable::setTableHeaders(m_table, headerLabels());
    LTable::resizeTableContents(m_table);
}



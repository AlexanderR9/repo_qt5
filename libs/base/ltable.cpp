#include "ltable.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QColor>
#include <QDebug>


void LTable::insertTableRow(int index, QTableWidget *table, const QStringList &list, int align, QColor cf, QColor cb)
{
    if (!table) return;
    if (index < 0 || index > table->rowCount()) return;
    if (table->columnCount() != list.count())
    {
        qWarning()<<QString("LTable::addTableRow - ERR: col count(%1) != list count(%2)").arg(table->columnCount()).arg(list.count());
        return;
    }

    if (table->rowCount() == index)
    {
        addTableRow(table, list, align, cf, cb);
        return;
    }

    table->insertRow(index);
    setTableRow(index, table, list, align, cf);
}
void LTable::addTableRow(QTableWidget *table, const QStringList &list, int align, QColor cf, QColor cb)
{
    if (!table) return;
    if (table->columnCount() != list.count())
    {
        qWarning()<<QString("LTable::addTableRow - ERR: col count(%1) != list count(%2)").arg(table->columnCount()).arg(list.count());
        return;
    }

    table->setRowCount(table->rowCount()+1);
    for (int i=0; i<table->columnCount(); i++)
    {
    QTableWidgetItem *item = new QTableWidgetItem(list.at(i));
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    if (align >= 0) item->setTextAlignment(align);
    item->setForeground(QBrush(cf));
    item->setBackground(QBrush(cb));
    table->setItem(table->rowCount()-1, i, item);
    }
}
void LTable::setTableRow(int row, QTableWidget *table, const QStringList &list, int align, QColor c)
{
    if (!table) return;

    if (table->columnCount() != list.count())
    {
        qWarning()<<QString("LTable::setTableRow - ERR: col count(%1) != list count(%2)").arg(table->columnCount()).arg(list.count());
        return;
    }
    if (row < 0 || row >= table->rowCount())
    {
        qWarning()<<QString("LTable::setTableRow - ERR: invalid row index: %1").arg(row);
        return;
    }

    for (int i=0; i<table->columnCount(); i++)
    {
        QTableWidgetItem *item = table->item(row, i);
        if (!item)
        {
            item = new QTableWidgetItem(list.at(i));
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            table->setItem(row, i, item);
        }

        item->setText(list.at(i));
        if (align >= 0) item->setTextAlignment(align);
        item->setForeground(QBrush(c));
    }
}
void LTable::createTableItem(QTableWidget *table, int row, int col, const QString &text, int flags, int align, QColor c)
{
    if (!table) return;

    if (table->columnCount() <= col || col < 0)
    {
        qWarning()<<QString("LTable::createTableItem - ERR: cols count(%1) , param col (%2)").arg(table->columnCount()).arg(col);
        return;
    }
    if (table->rowCount() <= row || row < 0)
    {
        qWarning()<<QString("LTable::createTableItem - ERR: rows count(%1) , param row (%2)").arg(table->rowCount()).arg(row);
        return;
    }

    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setFlags(Qt::ItemFlags(flags));
    item->setTextAlignment(align);
    item->setTextColor(c);
    table->setItem(row, col, item);
}
void LTable::resizeTableContents(QTableWidget *table)
{
    if (!table) return;
    table->resizeColumnsToContents();
    table->resizeRowsToContents();

}
void LTable::setTableRowColor(QTableWidget *table, int row, const QColor &c)
{
//    qDebug()<<QString("LTable::setTableRowColor  row=%0/%1, color=%2").arg(table->rowCount()).arg(row).arg(c.red());
    if (!table) return;
    if (row < 0 || row >= table->rowCount()) return;
    for (int i=0; i<table->columnCount(); i++)
    {
        QTableWidgetItem *item = table->item(row, i);
        if (item) item->setBackground(QBrush(c));
    }
}
void LTable::clearAllItemsText(QTableWidget *table)
{
    if (!table) return;
    for (int i=0; i<table->rowCount(); i++)
    {
        for (int j=0; j<table->columnCount(); j++)
        {
            QTableWidgetItem *item = table->item(i, j);
            if (item) item->setText(QString());
        }
    }
}
void LTable::createAllItems(QTableWidget *table, int align)
{
    if (!table) return;
    table->clearContents();

    for (int i=0; i<table->rowCount(); i++)
    {
        for (int j=0; j<table->columnCount(); j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(QString());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            if (align >= 0) item->setTextAlignment(align);
            table->setItem(i, j, item);
        }
    }
}
void LTable::setTableHeaders(QTableWidget *table, const QStringList &list, int orintation)
{
    if (!table || list.isEmpty()) return;

    if (orintation == Qt::Horizontal)
    {
        table->setColumnCount(list.count());
        table->setHorizontalHeaderLabels(list);
    }
    else
    {
        table->setRowCount(list.count());
        table->setVerticalHeaderLabels(list);
    }
}
void LTable::fullClearTable(QTableWidget *table)
{
    if (!table) return;
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
}
void LTable::removeAllRowsTable(QTableWidget *table)
{
    if (!table) return;
    int n = table->rowCount();
    for (int i=0; i<n; i++)	table->removeRow(0);
    table->setRowCount(0);
}
void LTable::shiftTableRowToBegin(QTableWidget *table, int row_index)
{
    if (row_index == 0) return;
    int shift = -1*(row_index+100);
    shiftTableRow(table, row_index, shift);
}
void LTable::shiftTableRowToEnd(QTableWidget *table, int row_index)
{
    if (!table) return;
    if (row_index == (table->rowCount() - 1)) return;
    int shift = (table->rowCount()+100);
    shiftTableRow(table, row_index, shift);
}
void LTable::shiftTableRow(QTableWidget *table, int row_index, int shift)
{
    if (!table || shift == 0 || row_index < 0 || row_index >= table->rowCount()) return;
    int new_index = row_index + shift;

    QList<QTableWidgetItem*> save_items;
    for (int j=0; j<table->columnCount(); j++)
        save_items.append(table->takeItem(row_index, j));
    table->removeRow(row_index);

    if (new_index < 0) new_index = 0;
    if (new_index > table->rowCount()) new_index = table->rowCount();
    table->insertRow(new_index);

    for (int j=0; j<table->columnCount(); j++)
        table->setItem(new_index, j, save_items.at(j));
}
QList<int> LTable::selectedRows(QTableWidget *table)
{
    QList<int> list;
    if (!table) return list;

    QList<QTableWidgetItem*> items(table->selectedItems());
    for (int i=0; i<items.count(); i++)
    {
        if (!list.contains(items.at(i)->row()))
            list.append(items.at(i)->row());
    }
    return list;
}
QList<int> LTable::selectedCols(QTableWidget *table)
{
    QList<int> list;
    if (!table) return list;

    QList<QTableWidgetItem*> items(table->selectedItems());
    for (int i=0; i<items.count(); i++)
    {
        int col = items.at(i)->column();
        if (!list.contains(col)) list.append(col);
    }
    return list;
}
double LTable::minNumericColValue(QTableWidget *table, int col, int &value_row, int row_first)
{
    value_row = -1;
    double min = -9999;
    if (!table || col < 0 || col >= table->columnCount()) return min;
    int n = table->rowCount();
    if (n <= 0 || row_first >= n)  return min;

    int start_row = 0;
    if (row_first > 0) start_row = row_first;

    bool ok;
    bool find_first = false;
    for (int i=start_row; i<n; i++)
    {
        QString s = table->item(i, col)->text().trimmed();
        s.replace(QString("%"), QString());
        s.replace(QString("+"), QString());
        double v = s.toDouble(&ok);
        if (!ok) qWarning()<<QString("LTable::minNumericColValue WARNING - invalid numeric value %1, row %2").arg(table->item(i, col)->text()).arg(i);
        else if (!find_first) {min = v; value_row = i; find_first = true;}
        else if (v < min) {min = v; value_row = i;}
    }
    return min;
}
double LTable::maxNumericColValue(QTableWidget *table, int col, int &value_row, int row_first)
{
    value_row = -1;
    double max = 0;
    if (!table || col < 0 || col >= table->columnCount()) return max;
    int n = table->rowCount();
    if (n <= 0 || row_first >= n)  return max;

    int start_row = 0;
    if (row_first > 0) start_row = row_first;

    bool ok;
    bool find_first = false;
    for (int i=start_row; i<n; i++)
    {
        QString s = table->item(i, col)->text().trimmed();
        s.replace(QString("%"), QString());
        s.replace(QString("+"), QString());
        double v = s.toDouble(&ok);
        if (!ok) qWarning()<<QString("LTable::maxNumericColValue WARNING - invalid numeric value %1, row %2").arg(table->item(i, col)->text()).arg(i);
        else if (!find_first) {max = v; value_row = i; find_first = true;}
        else if (v > max) {max = v; value_row = i;}
    }
    return max;
}




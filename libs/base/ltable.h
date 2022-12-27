 #ifndef LTABLE_H
 #define LTABLE_H

#include <QObject>
#include <QColor>


class QTableWidget;
class QTableWidgetItem;
class QStringList;

//статический класс с набором функций для работы с объектов QTableWidget

///////////LTable//////////////////////////
class LTable
{
public:
    static void addTableRow(QTableWidget*, const QStringList&, int align = Qt::AlignCenter, QColor cf = Qt::black, QColor cb = Qt::white);
    static void insertTableRow(int, QTableWidget*, const QStringList&, int align = Qt::AlignCenter, QColor cf = Qt::black, QColor cb = Qt::white);
    static void setTableRow(int, QTableWidget*, const QStringList&, int align = Qt::AlignCenter, QColor c = Qt::black);
    static void createTableItem(QTableWidget*, int, int, const QString&, int flags = Qt::ItemIsEnabled, int align = Qt::AlignCenter, QColor c = Qt::black);
    static void setTableHeaders(QTableWidget*, const QStringList&, int orintation = Qt::Horizontal);
    static void removeAllRowsTable(QTableWidget*);
    static void fullClearTable(QTableWidget*);
    static QList<int> selectedRows(QTableWidget*);
    static QList<int> selectedCols(QTableWidget*);
    static void createAllItems(QTableWidget*, int align = Qt::AlignCenter);
    static void clearAllItemsText(QTableWidget*);
    static void setTableRowColor(QTableWidget*, int, const QColor &c);
    static void resizeTableContents(QTableWidget*);

    //перемещает строку row_index на shift позиций, shift может быть меньше нуля
    static void shiftTableRow(QTableWidget*, int row_index, int shift);

    //перемещает указанную строку в начало таблицы
    static void shiftTableRowToBegin(QTableWidget*, int);

    //перемещает указанную строку в конец таблицы
    static void shiftTableRowToEnd(QTableWidget*, int);

    //найти минимальное/максимальное числовое значение в заданном столбце.
    //если row_first > 0 то поиск начинается с этой строки.
    //значения в столбце должны быть числами, некорректные значения игнорируются.
    //в 3-й параметр запишется индекс строки с найденным значением.
    static double minNumericColValue(QTableWidget*, int, int&, int row_first = -1);
    static double maxNumericColValue(QTableWidget*, int, int&, int row_first = -1);

};


 #endif




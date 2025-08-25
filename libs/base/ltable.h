 #ifndef LTABLE_H
 #define LTABLE_H

#include <QObject>
#include <QColor>
#include <QDateTime>


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
    static void setTableRowColor(QTableWidget*, int, const QColor &c); //установить цвет фона строки
    static void setTableTextRowColor(QTableWidget*, int, const QColor &c); //установить цвет текста строки
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

    //найти минимальное/максимальное значение QDateTime в заданном столбце.
    //если row_first > 0 то поиск начинается с этой строки.
    //значения в столбце должны быть датой и временем, некорректные значения игнорируются.
    //в 3-й параметр запишется индекс строки с найденным значением.
    //dt_mask необходимо задавать в формате из двух частей d[<date mask>] и d[<time mask>], причем слагаемых может быть только одно(любое),
    //а так же важна последовательность этих двух слагаемых маски.
    //вслучае невалидных параметров вернет пустую QDateTime()
    //в ячейках перед преведением значения даты/время всякие посмотронние символы типа '(' будут удаляться
    static QDateTime minDTColValue(QTableWidget*, int, int&, int row_first = -1, QString dt_mask = "d[dd.MM.yyyy] t[hh:mm]");
    static QDateTime maxDTColValue(QTableWidget*, int, int&, int row_first = -1, QString dt_mask = "d[dd.MM.yyyy] t[hh:mm]");

};


 #endif




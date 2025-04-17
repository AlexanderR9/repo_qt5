#ifndef LSTRING_H
#define LSTRING_H


#include <QStringList>

//набор статических методов для работы со строками
///////////LString//////////////////////////
class LString
{
public:

    //возвращает строку в виде одиночного пробела
    static QString spaceSymbol() {return QString(" ");}

    //количество вхождений подстроки в строку
    static int subCount(const QString &s, const QString sub_s);

    //ищет позицию вхождения подстроки в строку, начиная С КОНЦА
    static int strIndexOfByEnd(const QString &s, const QString sub_s);

    //выдает список всех позиций в входной строке, на которых стоит указанная подстрока
    static QList<int> subStrIndexes(const QString&, const QString sub_s);

    //ищет и возвращает подстроку в строке, которая находится между двумя заданными подстроками (например между "(" и ")")
    static QString strBetweenStr(const QString &s, QString s1, QString s2);

    //заменяет все найденные sub_s1 на sub_s2, но только в заданном интервале pos1:pos2, сами позиции не входят в кусок поиска
    static QString replaceByRange(const QString&, const QString sub_s1, const QString sub_s2, quint16 pos1, quint16 pos2);

    //отрезает в строке n символов слева и возвращает новую результирующую строку
    static QString strTrimLeft(const QString &s, int n);

    //добавляет в строке n символов QChar слева и возвращает новую результирующую строку
    static QString strAddLeft(const QString &s, int n,  QChar c = QChar(' '));

    //выравнивает строку s до заданного размера n, добавляя слева необходимое количество символов QChar и возвращает новую результирующую строку
    static QString strAlignLeft(const QString &s, int n,  QChar c = QChar(' '));

    //отрезает в строке n символов справа и возвращает новую результирующую строку
    static QString strTrimRight(const QString &s, int n);

    //добавляет в строке n символов QChar справа и возвращает новую результирующую строку
    static QString strAddRight(const QString &s, int n,  QChar c = QChar(' '));

    //выравнивает строку s до заданного размера n, добавляя справа необходимое количество символов QChar и возвращает новую результирующую строку
    static QString strAlignRight(const QString &s, int n,  QChar c = QChar(' '));

    //преобразует строку в список строк по заданному разделителю.
    //если remove_empty_line = true то все пустые строки удаляются и все крайние пробелы у всех строк отсекаются.
    //исходная строка не меняется.
    static QStringList trimSplitList(const QString&, QString split_symbol = "\n", bool remove_empty_line = true);

    //преобразовать QStringList в единую строку, все элементы будут отделены split_symbol
    static QString uniteList(const QStringList&, QString split_symbol);

    //удаляет в строке все длинные пробелы и заменяет их на одиночные, предварительно пробелы с краев обрезаются.
    //если remove_tabs = true то все табуляции заменяются одиночные пробелы.
    //исходная строка не меняется.
    static QString removeLongSpaces(const QString&, bool remove_tabs = true);
    static QString removeSpaces(const QString&); //удаляет вообще все пробелы
    static QString removeSymbol(const QString&, QChar); //удаляет указанный символ в строке (полностью на всех найденных позициях)

    //возвращает строку состоящую из N одинаковых символов
    static QString symbolString(QChar c, quint16 N = 50) {return QString(N, c);}

    //разбивает строку на символы и возвращает список символов с их кодами.
    //если строка пуста, то вернет пустой список.
    static QStringList toUnicode(const QString&);


    //удаляет в QStringList пустые строки
    //если remove_spaces = true то также удаляются строки состоящие из одних пробелов.
    static void removeEmptyStrings(QStringList&, bool remove_spaces = true);


};




#endif // LSTRING_H




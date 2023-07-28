 #ifndef LSTATIC_H
 #define LSTATIC_H

#include <QColor>
#include <QString>


///////////LStatic//////////////////////////
class LStatic
{
public:
    static int defIconSize() {return 40;}

	//for strings
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



    //преобразует русский текст в unicode через заданный кодек, после чего его можно вывести на экран или записать в файл
    static QString fromCodec(const QString&, QString codec = "utf8");
    //преобразует QColor в строку типа (r; g; b)
    static QString fromColor(const QColor&, QString split_symbol = ";");
    //преобразует строку в QColor, если строка не корректна, то вернет QColor(0, 0, 0)
    static QColor strToColor(const QString&, QString split_symbol = ";");
    //преобразует QByteArray в строку типа: AA BB 12 FD ....., line_size задает через сколько байт вставлять '\n'
    //если with_int_values то выводится каждый байт с расшифровкой в десятичном виде  : AA(170) BB(187) ...
    static QString baToStr(const QByteArray&, int line_size = 8, bool with_int_values = false);

    //преобразует строку в список строк по заданному разделителю
    //если remove_empty_line = true то все пустые строки удаляются и все крайние пробелы у всех строк отсекаются
    //исходная строка не меняется
    static QStringList trimSplitList(const QString&, QString split_symbol = "\n", bool remove_empty_line = true);

    //удаляет в строке все длинный пробелы и заменяет их на одиночные,
    //если remove_tabs = true то все табуляции заменяются одиночные пробелы
    //исходная строка не меняется
    static QString removeLongSpaces(const QString&, bool remove_tabs = true);

    //возвращает строку в виде одиночного пробела
    static QString spaceSymbol() {return QString(" ");}
    //возвращает символ пробел, unicode - 32
    static QChar spaceSymbolChar() {return QChar(32);}
    //возвращает управляющй символ перехода на другую строку (\n), unicode - 10
    static QChar nextLineSymbol() {return QChar(10);}
    //возвращает управляющй символ возврата каретки в начало строки (\r), unicode - 13
    static QChar carriageReturnSymbol() {return QChar(13);}
    //возвращает управляющй символ табуляции (\t), unicode - 9
    static QChar tabulationSymbol() {return QChar(9);}
    //количество вхождений символа в строку
    static int subCharCount(const QString&, const QChar);
    //преобразует строку в посимвольное представление с кодами каждого символа
    //show_nums - дает отображение индексов символов, show_char - дает отображение самих символов
    static QString strToCharUnicodes(const QString&, bool show_nums = true, bool show_char = false, QString separator = "|");


    //возвращает информацию о текущей ОС
    static QString systemInfo();


};



#endif




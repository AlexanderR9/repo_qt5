 #ifndef LMATH_H
 #define LMATH_H

 #include "math.h"
 #include "qlist.h"
 #include "qvector.h"

union floatUnion
{
    float f;
    quint8 buff[4];
};
union doubleUnion
{
    double f;
    quint8 buff[8];
};


//////////////////////
class LMath
{
public:
    static int rndInt(uint a, uint b); //случайное значение от a до b, при условии a <= b
    static double rnd(); //случайное значение от 0 до 1(всегда меньше 1) с разрешением 4 знака
    static float rndFloat(float a, float b); //случайное вещественное значение от a до b, при условии a <= b
    static bool factorOk(double k);
    static double min(const QVector<double> &v);
    static double max(const QVector<double> &v);
    static double pi() {return 3.14159265;}
    static int sign(double a) {return ((a < 0) ? -1 : 1);} //знак числа а
    static int sign(qint64 a) {return ((a < 0) ? -1 : 1);} //знак числа а
    static double sqrt(const double&, const double &exact = 0.001);
    static qint64 floor64(const double&); //нижнее округление
    static qint64 ceil64(const double&); //верхнее округление
    static double fractionalPart(const double&, quint8 digist = 8); //возвращает дробную часть числа, digist знаков после запятой (пример 0.01565465)


    //случайное значение знака 50/50 (т.е. либо -1, либо 1)
    static int rndSignum() {return ((rnd() < 0.5) ? -1 : 1);}

    //установка генератора случайных чисел в случайное положение
    static void rndReset();

    //случайное значение да/нет при заданной вероятности в %
    static bool probabilityOk(float);

    //бит в байте
    static quint8 byteSize() {return 8;}

    //проверка взведён ли бит на позиции bit_pos значения a, bit_pos = [0..31]
    static bool isBitOn(int a, quint8 bit_pos);

    //взводит бит на позиции bit_pos значения a, bit_pos = [0..31]
    static void setBitOn(int &a, quint8 bit_pos);

    //сбрасывает бит на позиции bit_pos значения a, bit_pos = [0..31]
    static void setBitOff(int &a, quint8 bit_pos);

    //возвращает строковую последовательность нулей и единиц значения a
    static QString toStr(int a);

    //возвращает строку в шеснадцатиричном формате значения a, если with_x == true то вернет в формате 0xXXXXXXXX
    static QString uint8ToBAStr(quint8, bool with_x = false);
    static QString uint16ToBAStr(quint16, bool with_x = false);
    static QString uint32ToBAStr(quint32, bool with_x = false);
    static QString uint64ToBAStr(quint64, bool with_x = false);
    static QString floatToBAStr(float, bool with_x = false);
    static QString doubleToBAStr(double, bool with_x = false);
    static QString charToBAStr(char, bool with_x = false);

    //взять младшие 2 байта у quint32
    static quint16 lowHalfFromUint32(quint32 a) {a = (a << 16);  return (a >> 16);}
    //взять старшие 2 байта у quint32
    static quint16 highHalfFromUint32(quint32 a) {return (a >> 16);}
    //взять младшие 4 байта у quint64
    static quint32 lowHalfFromUint64(quint64 a) {a = (a << 32);  return (a >> 32);}
    //взять старшие 4 байта у quint64
    static quint32 highHalfFromUint64(quint64 a) {return (a >> 32);}


    //развернуть порядок байт значения
    static quint16 rolloverOrderUint16(quint16);
    static quint32 rolloverOrderUint32(quint32);
    static quint64 rolloverOrderUint64(quint64);


    //взять n байт в заданном QByteArray, начиная с указанной позиции и преобразовать в числою
    //если позиция указана неверно или байт не хватает, то вернет 0.
    static quint16 uint16FromBA(const QByteArray&, quint16);
    static quint32 uint32FromBA(const QByteArray&, quint16);
    static quint64 uint64FromBA(const QByteArray&, quint16);
    static int intFromBA(const QByteArray&, quint16);
    static float floatFromBA(const QByteArray&, quint16);
    static double doubleFromBA(const QByteArray&, quint16);


private:
    static QString alignStrBA(const QString&, int, bool);


    template<typename T>
    static void getValueFromBA(T&, const QByteArray&, quint16 pos);

};

////////////////////////////////////////////////////////////////////
class LBigInt
{
public:
    LBigInt();   //невалидное БЧ
    LBigInt(QString, bool neg = false); //на вход подать строку в которой все символы это десятичные цифры, знак устанавливается отдельным параметром
    LBigInt(const LBigInt&); //клонировать другое БЧ
    LBigInt(quint16); //степень двойки преобразовать в БЧ, is_negative == false
    LBigInt(quint32 a, quint16 degree); //a^degree преобразовать в БЧ, is_negative == false

    inline bool invalid() const {return m_rawData.isEmpty();}
    inline quint16 len() const {return m_rawData.length();}    
    inline QString rawData() const {return m_rawData;}
    inline int groupsCount() const {return m_groups.count();}
    inline bool isNegative() const {return is_negative;}

    void toDebug(QString label = QString()); //diag func

    //operations funcs
    void increase(const LBigInt&); //операция - сложение с другим БЧ  (знак учитывается)
    void increaseSimple(qint32); //операция - сложение с простым числом  (знак учитывается)
    void increaseOne(); //опрерация увеличение на 1 (аналог ++)
    void decrease(const LBigInt&); //операция - вычитание из своего БЧ другим БЧ, т.е this.B_NUM - other.B_NUM  (знак учитывается)
    void multiply(const LBigInt&); //операция - умножение на другое БЧ  (знак учитывается)
    void multiplySimple(qint32); //операция - умножение на простое число 32bit  (знак учитывается)
    void multiply_10(quint8 deegree = 1); //операция - умножение на 10 в степени deegree
    void makeDual(); //операция - умножение на 2
    void invertSign(); //операция - умножение на '-1'

    //деление происходит грубо, т.е. до целого значения, идея - получить нужный порядок результирующего БЧ.
    //если делитель равен 0, то результат будет -1.
    //если делитель больше делимого как минимум на 1 группу (в 10^groupSize() раз), то результат будет 0.
    void division(const LBigInt&); //операция - деление на другое БЧ  (знак учитывается)
    void division_10(quint8 deegree = 1); //операция - деление на 10 в степени deegree, дробный остаток запишется в m_divisionResult
    inline double divisionRemainder() const {return m_divisionResult;} //дробный остаток после операции деления
    inline QString strDivisionRemainder() const {return QString::number(m_divisionResult, 'f', 2*groupSize());}

    LBigInt div(const LBigInt&) const; //результат целочисленного деления, знак не учитывается
    LBigInt mod(const LBigInt&) const; //остаток от целочисленного деления, знак не учитывается


    bool isLarger(const LBigInt&) const; //вернет true если свое БЧ больше параметра (знак учитывается)
    bool isSmaller(const LBigInt&) const; //вернет true если свое БЧ меньше параметра (знак учитывается)
    bool isEqual(const LBigInt&) const; //вернет true если свое БЧ равно параметру (знак учитывается)
    bool isLarger_abs(const LBigInt&) const; //вернет true если модуль своего БЧ больше модуля параметра (знак не учитывается)
    bool isSmaller_abs(const LBigInt&) const; //вернет true если модуль своего БЧ меньше модуля параметра (знак не учитывается)
    bool isEqual_abs(const LBigInt&) const; //вернет true если модуль своего БЧ равен модулю параметра (знак не учитывается)
    bool isEqualSign(const LBigInt&) const; //вернет true если знак своего БЧ такой же как у параметра
    bool isNull() const; //вернет true если БЧ равно 0
    QString finalValue() const; //значение с учетом знака, для интерфейса пользователя
    qint64 groupAt(int i) const; // елемент указанной группы
    qint64 groupFirst() const;
    qint64 groupLast() const;

    static quint8 groupSize() {return 8;} //максимально допустимое количество символов в одной группе многочлена

    LBigInt operator=(const LBigInt&);

    //функция преобразует БЧ в double, уменьшая его на dec_order порядков.
    //dec_order необходимо задать таким чтобы поделенной БЧ уместилось в double
    double toLessOrderDouble(quint8 dec_order) const;

protected:
    QString m_rawData; //больше число INT в строковом виде, все символы должны быть десятичными цифрами иначе объект будет невалидным
    bool is_negative; //признак того что число отрицательное (знак БЧ)

    //группы на которые разбивается m_rawData (разложение на многочлен),
    //длина группы(кол-во символов) - groupSize()
    //с идексом 0 - самые младшие разряды, т.е индекс группы умноженный на groupSize() это степень 10,
    //т.е. множитель для группы с индексом i, m_groups.at(i) * 10^(i*groupSize())
    //все элементы должны быть неотрицательные
    QList<qint64> m_groups;

    //по умолчанию равен -1. не отражает знак числа БЧ.
    //обновляется только при выполнении операции деления, имеет смысл только если это значение > 0.
    //Может принимать > 0 и < 1 и только в случае если при делении результат - БЧ всего с одной группой (младшей),
    //т.е. когда результат по сути не является БЧ, и данная переменная содержит дробную часть результата.
    //особенно актуально если целочисленная часть результирующего БЧ равна 0.
    double m_divisionResult;

    void checkValidity();
    void initGroups();

    //функция приводит указанный набор  групп к правильному состоянию,
    //т.е. все элеменеты должны быть не длинее groupSize(), и при этом неотрицательные.
    //количество элементов-групп может увеличиться(ситуация: некоторые старшие разряды одной группы переползают на уровень выше)
    static void normalizeGroups(QList<qint64>&);
    static void normalizeGroupsSign(QList<qint64>&); //только нормализация знаков

    //функция приводит указанный набор вещественных групп к правильному состоянию,
    //нужна после операции деления для переноса части дробных знаков в младшие группы.
    //значения входного списка должны быть неотрицательные.
    //дробный остаток записывает в 3-й параметр.
    static void normalizeFloatGroups(const QList<double>&, QList<qint64>&, double&);

    //удаление всех старших нулевых групп, кроме самой младшей
    static void removeOlderNull(QList<qint64>&);

    // размер единицы старшей группы для младшей, т.е. 10 в степени groupSize()
    static qint64 olderDigitVolume();

    //функция перезаписывает поля БЧ m_groups и m_rawData в соответствии с входным списком групп.
    //входные группы должны быть нормализованными.
    //поле is_negative не меняется.
    void reloadData(const QList<qint64>&);



    void reset();
    void toNull(); //установить своему БЧ нулевое значение


private:
    //операция - сложение с двух БЧ  (знак не учитывается),
    //складываются попарно группы своего БЧ и другого.
    //предполагается что со знаками разобрались до выполнения этой операции и оба БЧ валидны.
    //при выполнении операции свое БЧ не меняется, результат записывается в промежуточный список групп(входной параметр)
    //на выходе всегда получаем список нормализованных групп
    void increasePrivate(const LBigInt&, QList<qint64>&);

    //операция - вычитание из своего БЧ 2-е БЧ  (знак не учитывается),
    //предполагается что со знаками разобрались до выполнения этой операции и оба БЧ валидны
    //при выполнении операции свое БЧ не меняется, результат записывается в промежуточный список групп(входной параметр)
    //на выходе всегда получаем список нормализованных групп
    void decreasePrivate(const LBigInt&, QList<qint64>&);


    //операция - умножение своего БЧ на другое БЧ  (знак не учитывается),
    //предполагается что со знаками разобрались до выполнения этой операции и оба БЧ валидны
    //при выполнении операции свое БЧ не меняется, результат записывается в промежуточный список групп(входной параметр)
    //на выходе всегда получаем список нормализованных групп
    void multiplyPrivate(const LBigInt&, QList<qint64>&);


    //операция - деление своего БЧ на другое БЧ  (знак не учитывается),
    //предполагается что со знаками разобрались до выполнения этой операции и оба БЧ валидны
    //при выполнении операции свое БЧ не меняется, результат записывается в промежуточный список групп(входной параметр)
    //на выходе всегда получаем список нормализованных групп.
    void divisionPrivate(const LBigInt&, QList<qint64>&);
//    void divisionPrivate_large(const LBigInt&, QList<qint64>&); //порядок делителя(количество групп) БОЛЬШЕ делимого.



    //преобразует строку в вещественный остаток(<1) и записывает его в m_divisionResult
    void toFloatRemainder(QString);

};


 #endif

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
    static double rnd(); //случайное значение от 0 до 1
    static bool factorOk(double k);
    static double min(const QVector<double> &v);
    static double max(const QVector<double> &v);
    static double pi() {return 3.14159265;}
    static int sign(double a) {return ((a < 0) ? -1 : 1);} //знак числа а
    static double sqrt(const double&, const double &exact = 0.001);
    static int rndSignum() {return ((rnd() < 0.5) ? -1 : 1);} //случайное значение знака (т.е. либо -1, либо 1)

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

    //try template TO DO
    //template<typename T>
    //static QString intToBAStr(T a, bool with_x = false);// {return QString();}

private:
    static QString alignStrBA(const QString&, int, bool);

};




 #endif

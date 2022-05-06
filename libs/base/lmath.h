 #ifndef LMATH_H
 #define LMATH_H

 #include "math.h"
 #include "qlist.h"
 #include "qvector.h"


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



    static quint8 byteSize() {return 8;}

    // проверка взведён ли бит на позиции bit_pos значения a, bit_pos = [0..31]
    static bool isBitOn(int a, quint8 bit_pos);

    //взводит бит на позиции bit_pos значения a, bit_pos = [0..31]
    static void setBitOn(int &a, quint8 bit_pos);

    //сбрасывает бит на позиции bit_pos значения a, bit_pos = [0..31]
    static void setBitOff(int &a, quint8 bit_pos);

    //возвращает строковую последовательность нулей и единиц значения a
    static QString toStr(int a);

};




 #endif

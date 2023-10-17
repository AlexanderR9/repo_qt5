#include "lmath.h"
#include "lstring.h"
#include "lstatic.h"
#include "qdebug.h"



/////////////////////////////////
int LMath::rndInt(uint a, uint b)
{
    if (a == b) return a;
    if (a > b) return -1;
    return (a + qrand()%(b-a+1));
}
double LMath::rnd()
{
    return double(rndInt(0, 99))/100;
}
bool LMath::factorOk(double k)
{
    if (k < 1.01 || k > 100) return false;
    double lim = double(1)/k;
    return (rnd() < lim);
}
double LMath::min(const QVector<double> &v)
{
    if (v.isEmpty()) return 0;
    double m = v.at(0);	
    foreach(double a, v)
       if (a<m) m=a;
    return m;
}
double LMath::max(const QVector<double> &v)
{
    if (v.isEmpty()) return 0;
    double m = v.at(0);	
    foreach(double a, v)
    if (a>m) m=a;
    return m;
}
double LMath::sqrt(const double &value, const double &exact)
{
/*
    const float x2 = value * 0.5F;
    const float threehalfs = 1.5F;

    union {
	float f;
	quint32 i;
    } conv = {value}; // member 'f' set to value of 'number'.
    conv.i  = 0x5f3759df - ( conv.i >> 1 );
    conv.f  *= ( threehalfs - ( x2 * conv.f * conv.f ) );
    return conv.f;
*/



    double squareRoot = value/2;     
    int iter = 0;
    for (;;) 
    {
	iter++;
	double nx = (squareRoot + value/squareRoot)/2;
        if (fabs(squareRoot - nx) < exact)  break; //точность
	squareRoot = nx;
    }
//    qDebug()<<QString("LMath::sqrt  iters %1").arg(iter);
    return squareRoot;

    
}


/////////////BIT OPERATIONS////////////////
bool LMath::isBitOn(int a, quint8 bit_pos)
{
    if (bit_pos > (sizeof(a)*byteSize() - 1)) return false;
    quint32 aa = 1;
    if (bit_pos > 0) aa <<= bit_pos;
    return !((aa & a) == 0);
}
void LMath::setBitOn(int &a, quint8 bit_pos)
{
    if (bit_pos > (sizeof(a)*byteSize() - 1)) return;
    quint32 aa = 1;
    if (bit_pos > 0) aa <<= bit_pos;
    a |= aa;
}
void LMath::setBitOff(int &a, quint8 bit_pos)
{
    if (bit_pos > (sizeof(a)*byteSize() - 1)) return;
    if (!isBitOn(a, bit_pos)) return;

    quint32 aa = 0;
    if (bit_pos > 0) aa <<= bit_pos;
    a |= aa;
}
QString LMath::toStr(int a)
{
    QString s;
    for(uint i=0; i<sizeof(a)*byteSize(); i++)
        s = QString(" %1%2").arg(isBitOn(a,i)?"1":"0").arg(s);
    return s;
}


/////////////BYTE OPERATIONS////////////////


//to str BA
QString LMath::uint8ToBAStr(quint8 a, bool with_x)
{
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}
QString LMath::uint16ToBAStr(quint16 a, bool with_x)
{
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}
QString LMath::uint32ToBAStr(quint32 a, bool with_x)
{
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}
QString LMath::uint64ToBAStr(quint64 a, bool with_x)
{
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}
QString LMath::floatToBAStr(float a, bool with_x)
{
    floatUnion v;
    v.f = a;

    QString s;
    for (int i=0; i<4; i++)
        s += uint8ToBAStr(v.buff[i], false);

    if (with_x) return QString("0x%1").arg(s);
    return s;
}
QString LMath::doubleToBAStr(double a, bool with_x)
{
    doubleUnion v;
    v.f = a;

    QString s;
    for (int i=0; i<8; i++)
        s += uint8ToBAStr(v.buff[i], false);

    if (with_x) return QString("0x%1").arg(s);
    return s;
}
quint16 LMath::rolloverOrderUint16(quint16 a)
{
    return ((a << 8) | (a >> 8));
}
quint32 LMath::rolloverOrderUint32(quint32 a)
{
    quint32 mid = 0;
    quint32 b = 0;
    b = a >> 24;
    mid = a << 8;
    mid = mid >> 24;
    b |= (mid << 8);
    mid = a << 16;
    mid = mid >> 24;
    b |= (mid << 16);
    b |= a << 24;
    return b;
}
quint64 LMath::rolloverOrderUint64(quint64 a)
{
    quint64 mid = 0;
    quint64 b = 0;
    b = a >> 56;
    for (int i=8; i<=56; i+=8)
    {
        //qDebug("%i", i);
        mid = a << i;
        mid = mid >> 56;
        b |= (mid << i);
    }
    return b;
}


quint16 LMath::uint16FromBA(const QByteArray &ba, quint16 pos)
{
    quint16 v = 0;
    getValueFromBA(v, ba, pos);
    return v;
}
quint32 LMath::uint32FromBA(const QByteArray &ba, quint16 pos)
{
    quint32 v = 0;
    getValueFromBA(v, ba, pos);
    return v;
}
quint64 LMath::uint64FromBA(const QByteArray &ba, quint16 pos)
{
    quint64 v = 0;
    getValueFromBA(v, ba, pos);
    return v;
}
int LMath::intFromBA(const QByteArray &ba, quint16 pos)
{
    int v = 0;
    getValueFromBA(v, ba, pos);
    return v;
}
float LMath::floatFromBA(const QByteArray &ba, quint16 pos)
{
    float v = 0;
    getValueFromBA(v, ba, pos);
    return v;
}
double LMath::doubleFromBA(const QByteArray &ba, quint16 pos)
{
    double v = 0;
    getValueFromBA(v, ba, pos);
    return v;
}


//service func
QString LMath::alignStrBA(const QString &s, int n, bool with_x)
{
    QString s_res = LString::strAlignLeft(s, n, QChar('0')).toUpper();
    if (with_x) return QString("0x%1").arg(s_res);
    return s_res;
}


//static template metod
template<typename T>
void LMath::getValueFromBA(T &v, const QByteArray &ba, quint16 pos)
{
    v = 0;
    int sf = sizeof(T);
    if ((pos+sf) > ba.size()) return;

    QByteArray ba_mid(ba.mid(pos, sf));
    LStatic::reverseBA(ba_mid);
    memcpy(&v, ba_mid.data(), sf);
}













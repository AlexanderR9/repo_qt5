 #include "lmath.h"
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





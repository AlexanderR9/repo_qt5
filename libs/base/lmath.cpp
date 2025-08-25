#include "lmath.h"
#include "lstring.h"
#include "lstatic.h"
#include "qdebug.h"

#include <QTime>
#include <QDebug>
#include <QtMath>
#include <QCryptographicHash>
#include <QByteArray>



/////////////////////////////////
int LMath::rndInt(uint a, uint b)
{
    if (a == b) return a;
    if (a > b) return -1;
    return (a + qrand()%(b-a+1));
}
float LMath::rndFloat(float a, float b)
{
    if (a == b) return a;
    if (a > b) return -1;
    return (a + rnd()*(b-a));


    /*
    float df = b-a;
    float result = a + rnd()*df;
    qDebug()<<QString("a=%1,  b=%2,  df=%3,  result=%4").arg(a).arg(b).arg(df).arg(result);
    return result;
    */
}
double LMath::rnd()
{
    return double(rndInt(0, 9999))/10000;
}
void LMath::rndReset()
{
    qsrand(QTime::currentTime().msec());
}
bool LMath::probabilityOk(float p)
{
    // RAND_MAX == 2 147 483 647
    if (p > 0)
    {
        p /= float(100);
        float rnd_limit = p*float(RAND_MAX);
        return (qrand() < rnd_limit);
    }
    return false;
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
qint64 LMath::floor64(const double &a)
{
    QString s = QString::number(a, 'f', 6);
    int pos = s.indexOf(QChar('.'));
    if (pos > 0) return s.left(pos).toLong();
    pos = s.indexOf(QChar(','));
    if (pos > 0) return s.left(pos).toLong();
    return 0;
}
qint64 LMath::ceil64(const double &a)
{
    int sign = LMath::sign(a);

    QString s = QString::number(a, 'f', 6);
    int pos = s.indexOf(QChar('.'));
    if (pos > 0) return (s.left(pos).toLong()+sign);
    pos = s.indexOf(QChar(','));
    if (pos > 0) return (s.left(pos).toLong()+sign);
    return 0;
}
double LMath::fractionalPart(const double &a, quint8 digist)
{
    int sign = LMath::sign(a);
    double b = qAbs(a);
    b -= LMath::floor64(b);
    quint64 divisor = qPow(10, digist);
    qint64 r = floor64(b*double(divisor));
    b = double(r)/double(divisor);
    return (sign*b);
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
QString LMath::charToBAStr(char a, bool with_x)
{
    return uint8ToBAStr(quint8(a), with_x);
    //return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
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
QByteArray LMath::calcHMACSha256(QByteArray key, QByteArray baseString)
{
    QCryptographicHash::Algorithm metod = QCryptographicHash::Sha256;
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) // if key is longer than block size (64), reduce key length with SHA-1 compression
    {
        //qDebug("key.length() > blockSize");
        key = QCryptographicHash::hash(key, metod);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "quot;
    for (int i = 0; i < key.length(); i++)
    {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

     QByteArray total = outerPadding;
     QByteArray part = innerPadding;
     part.append(baseString);
     total.append(QCryptographicHash::hash(part, metod));
     return QCryptographicHash::hash(total, metod);
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


/////////////////////LBigInt///////////////////////////
LBigInt::LBigInt()
    :m_rawData(QString()),
    is_negative(false),
    m_divisionResult(-1)
{
    m_groups.clear();
}
LBigInt::LBigInt(QString str_big_number, bool neg)
    :m_rawData(str_big_number.trimmed()),
    is_negative(neg),
    m_divisionResult(-1)
{
    m_groups.clear();

    checkValidity();
    initGroups();
}
LBigInt::LBigInt(const LBigInt &other)
    :m_rawData(QString()),
      is_negative(false),
      m_divisionResult(-1)
{
    m_groups.clear();
    if (other.invalid()) return;

    m_rawData = other.rawData();
    is_negative = other.isNegative();
    initGroups();
}
LBigInt::LBigInt(quint16 degree)
    :is_negative(false),
      m_divisionResult(-1)
{
    QList<qint64> data;
    qint64 a = qint64(qPow(2, 16));
    if (degree > 16)
    {
        data.append(a);
        normalizeGroups(data);
        reloadData(data);

        for (int i=17; i<=degree; i++)
            makeDual();
    }
    else
    {
        a = qint64(qPow(2, degree));
        data.append(a);
        normalizeGroups(data);
        reloadData(data);
    }
}
LBigInt::LBigInt(quint32 a, quint16 degree)
    :is_negative(false),
      m_divisionResult(-1)
{
    m_rawData = QString::number(a);
    initGroups();
    if (degree < 2) return;

    LBigInt bi(*this);
    for (int i=2; i<=degree; i++)
        multiply(bi);
}
void LBigInt::increase(const LBigInt &other)
{
    if (invalid() || other.invalid()) return;

    QList<qint64> result;
    if (!isEqualSign(other)) //знаки противоположны, т.е. сложение сводится к вычитанию
    {
        if (isSmaller_abs(other)) is_negative = other.isNegative();

        decreasePrivate(other, result);
        reloadData(result);
        if (isNull()) is_negative = false;
    }
    else //знаки одинаковы, т.е. производится попарное сложение групп, а знак не меняется
    {
        increasePrivate(other, result);
        reloadData(result);
    }
}
void LBigInt::increaseSimple(qint32 a)
{
    if (invalid() || a == 0) return;

    bool neg = (a < 0);
    a = qAbs(a);
    LBigInt ba(QString::number(a), neg);
    increase(ba);
}
void LBigInt::increaseOne()
{
    qint32 a = 1;
    increaseSimple(a);
}
void LBigInt::decrease(const LBigInt &other)
{
    if (invalid() || other.invalid()) return;

    QList<qint64> result;
    if (!isEqualSign(other)) //знаки противоположны, т.е. вычитание сводится к сложению, а знак не меняется
    {
        increasePrivate(other, result);
        reloadData(result);
    }
    else //знаки одинаковы, т.е. производится попарное вычитание групп с перемещением разрядов при необходимости
    {
        if (isSmaller_abs(other)) is_negative = true;
        else is_negative = false;

        decreasePrivate(other, result);
        reloadData(result);
        if (isNull()) is_negative = false;
    }
}
void LBigInt::multiply(const LBigInt &other)
{
    if (invalid() || other.invalid()) return;
    if (isNull() || other.isNull()) {toNull(); return;} //хотя бы одно из БЧ нулевое

    if (!isEqualSign(other)) is_negative = true; //знаки противоположны
    else is_negative = false; //знаки одинаковы

    QList<qint64> result;
    multiplyPrivate(other, result);
    reloadData(result);
}
void LBigInt::multiply_10(quint8 deegree)
{
    //m_divisionResult = -1;
    if (invalid() || deegree == 0) return;
    if (isNull()) return;

    if (m_divisionResult > 0)
    {
        qint64 a = qint64(m_divisionResult*double(olderDigitVolume()));
        QString s = QString::number(a);
        for (int i=0; i<deegree; i++)
        {
            if (i < s.length()) m_rawData.append(s.at(i));
            else m_rawData.append(QChar('0'));
            m_divisionResult *= double(10);
        }
        m_divisionResult = LMath::fractionalPart(m_divisionResult, groupSize());
        if (m_divisionResult < 0.0000000001) m_divisionResult=-1;
    }
    else m_rawData.append(LString::symbolString(QChar('0'), deegree));

    initGroups();
}
void LBigInt::division_10(quint8 deegree)
{
    m_divisionResult = -1;
    if (invalid() || deegree == 0) return;
    if (isNull()) return;

    quint16 l = len();
    if (l > deegree)
    {
        QString s_remaind = rawData().right(deegree);
        m_rawData = LString::strTrimRight(m_rawData, deegree);
        initGroups();
        toFloatRemainder(s_remaind);
    }
    else
    {
        QString s_remaind(rawData());
        if (l < deegree) s_remaind.prepend(LString::symbolString(QChar('0'), deegree-l));
        toNull();
        toFloatRemainder(s_remaind);
    }
}
double LBigInt::toLessOrderDouble(quint8 dec_order) const
{
    if (dec_order == 0) return -1;
    LBigInt b(*this);
    b.division_10(dec_order);
    return b.finalValue().toDouble();
}
void LBigInt::division(const LBigInt &other)
{
    m_divisionResult = -1;
    if (invalid() || other.invalid()) return;
    if (isNull()) return;
    if (other.isNull()) //делим на 0
    {
        QList<qint64> result;
        result.append(1);
        is_negative = true;
        reloadData(result);
        return;
    }

    if (!isEqualSign(other)) is_negative = true; //знаки противоположны
    else is_negative = false; //знаки одинаковы

    QList<qint64> result;    
    /*
    if (other.groupsCount() > groupsCount()) //делитель намного больше делимого
    {
        divisionPrivate_large(other, result);
        if (result.isEmpty()) {toNull(); return;}
    }
    else divisionPrivate(other, result);
    */
    divisionPrivate(other, result);
    reloadData(result);
}
void LBigInt::makeDual()
{
    if (invalid() || isNull()) return;

    QList<qint64> result;
    int n = groupsCount();
    for (int i=0; i<n; i++) result.append(2*groupAt(i));

    normalizeGroups(result);
    reloadData(result);
}
void LBigInt::multiplySimple(qint32 a)
{
    if (invalid()) return;
    if (isNull()) return;
    if (a == 0) {toNull(); return;}

    if (a < 0) invertSign();
    a = qAbs(a);

    QList<qint64> result;
    int n = groupsCount();
    for (int i=0; i<n; i++)
        result.append(a*groupAt(i));

    normalizeGroups(result);
    reloadData(result);
}
void LBigInt::invertSign()
{
    if (isNegative())
    {
        is_negative = false;
        return;
    }
    is_negative = true;
}
void LBigInt::normalizeGroupsSign(QList<qint64> &list)
{
    if (list.isEmpty()) return;

    for (int i=0; i<list.count(); i++)
        list[i] = qAbs(list.at(i));
}
void LBigInt::normalizeGroups(QList<qint64> &list)
{
    if (list.isEmpty()) return;
    normalizeGroupsSign(list);

    //auxiliary vars
    QList<qint64> result;
    quint8 gs = groupSize();
    qint64 older_digit = olderDigitVolume(); // размер единицы старшей группы для младшей

    qint64 remainder = 0;
    while (2>1)
    {
        bool was_append = false;
        int n = list.count();
        for (int i=0; i<n; i++)
        {
            if (result.count() > i) continue;
            else was_append=true;

            qint64 a = list.at(i) + remainder;
            remainder = 0;
            if (a < older_digit) {result.append(a); continue;}

            QString s = QString::number(a);
            result.append(s.right(gs).toLong());
            remainder = LString::strTrimRight(s, gs).toLong();

            if (i == n-1) //is last element
            {
                list.append(remainder);
                remainder = 0;
            }

            break;
        }
        if (!was_append) break;
    }

    list.clear();
    list.append(result);
}
void LBigInt::normalizeFloatGroups(const QList<double> &list, QList<qint64> &result, double &remainder)
{
    result.clear();
    if (list.isEmpty()) return;

    quint8 gs = groupSize();
    qint64 older_digit = olderDigitVolume(); // размер единицы старшей группы для младшей
    int n = list.count();
    if (n == 1)
    {
        result.append(LMath::floor64(list.first()));
        remainder = LMath::fractionalPart(list.first(), gs);
        return;
    }

    // разделяем целые и дробные части
    QMap<int, qint64> mid_data;
    qint64 i_part = 0;
    double f_part = 0;
    int i = n-1;
    while (i >= 0)
    {
        i_part = LMath::floor64(list.first()) + qint64(f_part*older_digit);
        f_part = LMath::fractionalPart(list.at(i), gs);
        mid_data.insert(i, i_part);
    }

    remainder = f_part;
    for (int i=0; i<n; i++)
        result.append(mid_data.value(i));

    removeOlderNull(result);
    normalizeGroups(result);
}

void LBigInt::reloadData(const QList<qint64> &list)
{
    reset();
    if (list.isEmpty()) return;

    //reload raw data
    foreach (const qint64 &v, list)
    {
        QString s = QString::number(v);
        if (s.length() < groupSize())
            s.prepend(LString::symbolString(QChar('0'), groupSize()-s.length()));
        m_rawData.prepend(s);
    }
    while (2 > 1)
    {
        if (m_rawData.count() == 1) break;
        if (m_rawData.isEmpty()) break;
        if (m_rawData.at(0) != QChar('0')) break;
        m_rawData.remove(0, 1);
    }

    //reload groups values
    m_groups.append(list);
}
void LBigInt::reset()
{
    m_rawData.clear();
    m_groups.clear();
}
void LBigInt::toNull()
{
    is_negative = false;
    QList<qint64> list;
    list.append(0);
    reloadData(list);
}

bool LBigInt::isLarger(const LBigInt &other) const
{
    if (invalid() || other.invalid()) return false;
    if (isNegative() && !other.isNegative()) return false;
    if (!isNegative() && other.isNegative()) return true;

    int gc = groupsCount();
    if (!isNegative() && !other.isNegative()) // both +
    {
        if (gc > other.groupsCount()) return true;
        if (gc < other.groupsCount()) return false;
        return (groupAt(gc-1) > other.groupAt(gc-1));
    }
    if (isNegative() && other.isNegative()) // both -
    {
        if (gc > other.groupsCount()) return false;
        if (gc < other.groupsCount()) return true;
        return (groupAt(gc-1) < other.groupAt(gc-1));
    }

    return false;
}
bool LBigInt::isSmaller(const LBigInt &other) const
{
    if (invalid() || other.invalid()) return false;
    if (isLarger(other)) return false;
    if (isEqual(other)) return false;
    return true;
}
bool LBigInt::isEqual(const LBigInt &other) const
{
    if (invalid() || other.invalid()) return false;
    if (isNegative() != other.isNegative())  return false;
    return (rawData() == other.rawData());
}
bool LBigInt::isEqualSign(const LBigInt &other) const
{
    return (isNegative() == other.isNegative());
}
bool LBigInt::isLarger_abs(const LBigInt &other) const
{
    if (invalid() || other.invalid()) return false;

    int gc = groupsCount();
    if (gc > other.groupsCount()) return true;
    if (gc < other.groupsCount()) return false;
    return (groupAt(gc-1) > other.groupAt(gc-1));
}
bool LBigInt::isSmaller_abs(const LBigInt &other) const
{
    if (invalid() || other.invalid()) return false;
    if (isLarger_abs(other)) return false;
    if (isEqual_abs(other)) return false;
    return true;
}
bool LBigInt::isEqual_abs(const LBigInt &other) const
{
    if (invalid() || other.invalid()) return false;
    return (rawData() == other.rawData());
}
bool LBigInt::isNull() const
{
    if (invalid()) return false;
    if (groupsCount() != 1) return false;
    return (groupAt(0) == 0);
}
qint64 LBigInt::groupAt(int i) const
{
    if (invalid()) return -1;
    if (i < 0 || i >= groupsCount()) return 0;
    return m_groups.at(i);
}
qint64 LBigInt::groupFirst() const
{
    if (invalid()) return -1;
    return m_groups.first();
}
qint64 LBigInt::groupLast() const
{
    if (invalid()) return -1;
    return m_groups.last();
}
LBigInt LBigInt::operator=(const LBigInt &other)
{
    return LBigInt(other);
}
void LBigInt::checkValidity()
{
    if (m_rawData.isEmpty()) return;
    foreach (const QChar c, m_rawData)
    {
        if (!c.isDigit())
        {
            m_rawData.clear();
            break;
        }
    }
}
void LBigInt::initGroups()
{
    m_groups.clear();
    if (invalid()) return;

    quint16 l = len();
    quint8 gs = groupSize();

    QString data(m_rawData);
    while (l > gs)
    {
        QString s_group(data.right(gs));
        data = LString::strTrimRight(data, gs);
        m_groups.append(s_group.toLong());
        l -= gs;
    }
    if (!data.isEmpty())
    {
        m_groups.append(data.toLong());
    }
}
void LBigInt::toDebug(QString label)
{
    if (label.isEmpty()) qDebug()<<QString("LBigInt INFO: raw_data[%1] len[%2] validity[%3]").arg(rawData()).arg(len()).arg(invalid()?"INVALID":"OK");
    else qDebug()<<QString("LBigInt INFO[%0]: raw_data[%1] len[%2] validity[%3]").arg(label).arg(rawData()).arg(len()).arg(invalid()?"INVALID":"OK");
    if (invalid()) return;

    qDebug()<<QString("sign[%1], groups[%2]").arg(is_negative?"-":"+").arg(groupsCount());
    for(int i=0; i<groupsCount(); i++)
        qDebug()<<QString("G[%1] -> [%2]*10^%3").arg(i).arg(m_groups.at(i)).arg(i*groupSize());

    qDebug()<<QString("Division remainder: %1").arg(strDivisionRemainder());

}
QString LBigInt::finalValue() const
{
    if (invalid()) return QString("BIG_NUM_INVALID");
    QString s_remainder;
    if (m_divisionResult > 0)
    {
        s_remainder = QString::number(m_divisionResult, 'f', ((len() == 1) ? 2*groupSize() : groupSize()));
        s_remainder = LString::strTrimLeft(s_remainder, 1);
    }

    if (isNegative()) return QString("%1%2%3").arg("-").arg(m_rawData).arg(s_remainder);
    return QString("%1%2").arg(m_rawData).arg(s_remainder);
}
void LBigInt::removeOlderNull(QList<qint64> &list)
{
    if (list.isEmpty()) return;
    while (2>1)
    {
        if (list.count() <= 1) break;
        if (list.last() != 0) break;
        list.removeLast();
    }
}
qint64 LBigInt::olderDigitVolume()
{
    return qint64(qPow(10, groupSize()));
}
LBigInt LBigInt::div(const LBigInt &divisor) const
{
    LBigInt result;
    if (invalid() || divisor.invalid()) return result;
    if (isNull() || divisor.isNull()) return result;
    if (isSmaller_abs(divisor)) return LBigInt("0");

    LBigInt divisor_abs(divisor);
    if (divisor_abs.isNegative()) divisor_abs.invertSign();

    result.toNull();
    LBigInt b_remainder(*this);
    if (b_remainder.isNegative()) b_remainder.invertSign();

    quint16 d_len = len() - divisor_abs.len();
    //qDebug()<<QString("dlen=%1").arg(d_len);
    if (d_len > 0)
    {
        for (int degree=d_len; degree>0; degree--)
        {
            LBigInt bdec(10, degree);
            LBigInt dec_divisor(divisor_abs);
            dec_divisor.multiply(bdec);
           // qDebug()<<QString("dec_divisor: %1, degree=%2, bdec=%3").arg(dec_divisor.finalValue()).arg(degree).arg(bdec.finalValue());
            while(2>1)
            {
                if (dec_divisor.isLarger(b_remainder)) break;

                result.increase(bdec);
                b_remainder.decrease(dec_divisor);
            }
           // qDebug()<<QString("result: %1,  remainder: %2").arg(result.finalValue()).arg(b_remainder.finalValue());
        }
    }
    ////////////////////////////////////////////////////////////
    while (divisor_abs.isSmaller(b_remainder))
    {
        result.increaseOne();
        b_remainder.decrease(divisor_abs);
    }
    return result;
}
LBigInt LBigInt::mod(const LBigInt &divisor) const
{
   // qDebug()<<QString("--------- %1 MOD %2 --------------").arg(rawData()).arg(divisor.finalValue());
    LBigInt result;
    LBigInt bdiv = div(divisor);
    if (bdiv.invalid()) return result;

    LBigInt b_remainder(*this);
    if (b_remainder.isNegative()) b_remainder.invertSign();
    if (bdiv.isNull()) return b_remainder;

    LBigInt divisor_abs(divisor);


    if (divisor_abs.isNegative()) divisor_abs.invertSign();
    divisor_abs.multiply(bdiv);
   // qDebug()<<QString("bdiv = %1, divisor_abs.multiply: %2").arg(bdiv.finalValue()).arg(divisor_abs.finalValue());


    b_remainder.decrease(divisor_abs);
    return b_remainder;
}



//private funcs of LBigInt
void LBigInt::increasePrivate(const LBigInt &b, QList<qint64> &result)
{
    result.clear();
    int n = qMax(groupsCount(), b.groupsCount());
    for (int i=0; i<n; i++)
    {
        if (i >= b.groupsCount()) result.append(groupAt(i));
        else if (i >= groupsCount()) result.append(b.groupAt(i));
        else
        {
            qint64 gr_i = groupAt(i) + b.groupAt(i);
            result.append(gr_i);
        }
    }

    LBigInt::normalizeGroups(result);
}
void LBigInt::decreasePrivate(const LBigInt &b, QList<qint64> &result)
{
    result.clear();
    int n = qMax(groupsCount(), b.groupsCount());
    for (int i=0; i<n; i++) //попарное вычитание
    {
        if (i >= b.groupsCount()) result.append(groupAt(i));
        else if (i >= groupsCount()) result.append(-1*b.groupAt(i));
        else
        {
            qint64 gr_i = groupAt(i) - b.groupAt(i);
            result.append(gr_i);
        }
    }
    removeOlderNull(result); //удаление старших нулевых групп

    n = result.count();
    if (n == 1)
    {
        normalizeGroupsSign(result);
        return;
    }

    //приведение всех групп к одному знаку
    int parent_sign  = LMath::sign(result.last()); //знак старшей группы
    qint64 older_digit = olderDigitVolume(); // переход на 1 из старшей группы в соседнюю младшую

    //список признаков групп со знаками не равными parent_sign, соответственно там нужно перетащить older_digit из старшей группы
    //true - значит что необходимо списать 1 разряд из старшей группы.
    //размер списка на 1 меньше чем result.count(), имеет смысл при result.count() > 1
    QList<bool> wrong_signs;

    //обнаружение групп с неправильным знаком
    for (int i=0; i<(n-1); i++)
    {
        wrong_signs.append(false);
        if (result.at(i) == 0) continue;
        if (LMath::sign(result.at(i)) == parent_sign) continue;

        wrong_signs[i] = true;
        qint64 gr_i = older_digit - qAbs(result.at(i));
        result[i] = parent_sign*gr_i;
    }
    normalizeGroupsSign(result); //нормализация знака всех групп

    //списание 1 разряда для групп рядом стоящих с группами с неправильным знаком
    while (2>1)
    {
        if (result.count() <= 1) break;

        bool has_wrong = false;
        for (int i=0; i<wrong_signs.count(); i++)
        {
            if (!wrong_signs.at(i)) continue;
            else has_wrong = true;

            wrong_signs[i] = false;
            if (result.at(i+1) == 0)
            {
                result[i+1] = older_digit - 1;
                wrong_signs[i+1] = true;
            }
            else result[i+1]--;
            break;
        }
        if (!has_wrong) break;
    }
    removeOlderNull(result); //удаление старших нулевых групп
}
void LBigInt::multiplyPrivate(const LBigInt &b, QList<qint64> &result)
{
   // qDebug("////////////////LBigInt::multiplyPrivate///////////////");
    result.clear();
    QList<qint64> mid_data;
    QList<quint16> degrees;
    quint8 gs = groupSize();
    quint16 max_degree = 0;

    //перемножаем все комбинации обоих
    int n = groupsCount();
    int m = b.groupsCount();
    for (int i=0; i<n; i++)
    {
        qint64 ai = groupAt(n-i-1);
        for (int j=0; j<m; j++)
        {
            qint64 aj = b.groupAt(m-j-1);
            mid_data.append(qint64(ai*aj));
           // qDebug()<<QString("%1*%2 = %3").arg(aj).arg(ai).arg(mid_data.last());
            degrees.append(gs*((n-i-1)+(m-j-1)));
            if (degrees.last() > max_degree) max_degree = degrees.last();
        }
    }

/*  diag code
    n = mid_data.count();
    qDebug()<<QString("mid_data size: %1").arg(n);
    for (int i=0; i<n; i++)
        qDebug()<<QString("i_%1  mid_data=%2  degree=%3").arg(i).arg(mid_data.at(i)).arg(degrees.at(i));
*/

    //складываем все члены с одинаковым порядком
    n = mid_data.count();
    quint16 cur_degree = 0;
    while (2>1)
    {
        qint64 v = 0;
        for (int i=0; i<n; i++)
        {
            if (degrees.at(i) == cur_degree)
                v += mid_data.at(i);
        }
        result.append(v);

        cur_degree += gs;
        if (cur_degree > max_degree) break;
    }

    /*
    qDebug("------raw result-------");
    for (int i=0; i<result.count(); i++)
        qDebug()<<QString("i_%1  result=%2  degree=%3").arg(i).arg(result.at(i)).arg(i*gs);
        */

    LBigInt::removeOlderNull(result);
    LBigInt::normalizeGroups(result);

    /*
    qDebug("------raw result2-------");
    for (int i=0; i<result.count(); i++)
        qDebug()<<QString("i_%1  result=%2  degree=%3").arg(i).arg(result.at(i)).arg(i*gs);
        */

}
void LBigInt::divisionPrivate(const LBigInt &divisor, QList<qint64> &result)
{
   // qDebug()<<QString("%1 / %2").arg(finalValue()).arg(divisor.finalValue());

    //БЧ b - делитель
    //свое БЧ - делимое
    result.clear();
    LBigInt bdiv = div(divisor);
    if (bdiv.invalid()) return;
    //qDebug()<<QString("bdiv: %1").arg(bdiv.finalValue());

    //формируем целую часть при делении
    for (int i=0; i<bdiv.groupsCount(); i++)
        result.append(bdiv.groupAt(i));

    quint8 gs  = groupSize();
    LBigInt bmod = mod(divisor); //находим остаток отцелочисленного деления
    //qDebug()<<QString("bmod: %1").arg(bmod.finalValue());

    QString s("0.");
    for (int i=0; i<gs*2; i++)
    {
        bmod.multiply_10();
        if (bmod.isSmaller_abs(divisor)) s.append("0");
    }
    //qDebug()<<QString("bmod_next: %1").arg(bmod.finalValue());
    s.append(bmod.div(divisor).finalValue());
    //qDebug()<<s;
    m_divisionResult = s.toDouble();



    /*
    result.clear();
    qint64 older_digit = olderDigitVolume();
    quint8 gs  = groupSize();

    //V1 делитель всего с одной группой, т.е. как бы простое число
    if (b.groupsCount() == 1)
    {
        QList<double> f_result;
        for (int i=0; i<groupsCount(); i++)
            f_result.append(double(groupAt(i))/double(b.groupFirst()));
        normalizeFloatGroups(f_result, result, m_divisionResult);
        if (result.count() > 1) m_divisionResult = -1;
        return;
    }

    //V2  количество групп равно (при этом их больше 1), т.е. порядки чисел примерно одинаковы
    if (groupsCount() == b.groupsCount())
    {
        double own_f = groupLast();
        double other_f = b.groupLast();
        own_f += (double(groupAt(groupsCount()-2))/double(older_digit));
        other_f += (double(b.groupAt(groupsCount()-2))/double(older_digit));
        m_divisionResult = own_f/other_f;
        result.append(LMath::floor64(m_divisionResult));
        m_divisionResult = LMath::fractionalPart(m_divisionResult, gs);
        return;
    }

    //V3 количество групп и делимого больше чем у делителя (при этом их у обоих БЧ больше 1)
    LBigInt b_other_normal(b); //клонируем делитель
    quint16 n_degree = 0;
    while (b_other_normal.groupsCount() != groupsCount()) //доводим делитель до количества групп равное как у делимого
    {
        b_other_normal.multiply_10(1);
        n_degree++;
    }
    //qDebug()<<QString("upper_degree %1,  b_other_normal=%2").arg(n_degree).arg(b_other_normal.finalValue());

    //делим 2 старшие группы
    double own_f = groupLast();
    double other_f = b_other_normal.groupLast();
    own_f += (double(groupAt(groupsCount()-2))/double(older_digit));
    other_f += (double(b_other_normal.groupAt(groupsCount()-2))/double(older_digit));
    m_divisionResult = own_f/other_f;
    QString s_body = QString::number(LMath::floor64(m_divisionResult));
    m_divisionResult = LMath::fractionalPart(m_divisionResult, gs);
    QString s_remainder = QString::number(qint64(m_divisionResult*double(olderDigitVolume())));

    //домножаем результат на столько же на сколько наращивали делитель
    for (int i=0; i<n_degree; i++)
    {
        if (s_remainder.isEmpty()) s_body.append(QChar('0'));
        else
        {
            s_body.append(s_remainder.at(0));
            s_remainder = LString::strTrimLeft(s_remainder, 1);
        }
        m_divisionResult *= double(10);
    }
    m_divisionResult = LMath::fractionalPart(m_divisionResult, gs);
    if (m_divisionResult < 0.0000000001) m_divisionResult=-1;

    //заполняем результат
    int l = s_body.length();
    while (l > gs)
    {
        QString s_group(s_body.right(gs));
        s_body = LString::strTrimRight(s_body, gs);
        result.append(s_group.toLong());
        l -= gs;
    }
    if (!s_body.isEmpty())  result.append(s_body.toLong());
    */
}
/*
void LBigInt::divisionPrivate_large(const LBigInt &b, QList<qint64> &result)
{
    result.clear();
    int d_len = b.len() - len();
    if (d_len > 12) return;

    // в данной ситуации целая часть будет нулевая и соответственно состоять всего из 1-й группы.
    // а итоговы результат деления будет содержаться в переменной m_divisionResult
    result.append(0);

    LBigInt a(rawData()); //клонируем делимое
    LBigInt divisor(b.rawData()); //клонируем делитель
    while (2 > 1) //сокращаем делимое и делитель до тех пор пока их нельзя будет преобразовать в обычный double
    {
        a.division_10();
        divisor.division_10();
        if (a.len() < 18 && divisor.len() < 18) break;
    }

    double f_a = a.finalValue().toDouble();
    double f_divisor = divisor.finalValue().toDouble();
    qDebug()<<QString("LBigInt::divisionPrivate_large  f_a=%1,  f_divisor=%2").arg(f_a).arg(f_divisor);
    m_divisionResult = qAbs(f_a/f_divisor);
    qDebug()<<QString("m_divisionResult  %1").arg(m_divisionResult);
}
*/
void LBigInt::toFloatRemainder(QString s)
{
    int l = s.length();
    quint8 gs = groupSize();
    if (l > gs) s = s.left(gs);
    else if (l < gs) s.append(LString::symbolString(QChar('0'), gs-l));

    int n_null_left = 0;
    for (int i=0; i<gs; i++)
    {
        if (s.at(i) == QChar('0')) n_null_left++;
        else break;
    }

    m_divisionResult =  double(s.toLong())/double(olderDigitVolume());
    if (n_null_left > 0)
    {
        for (int i=0; i<n_null_left; i++)
            m_divisionResult /= double(10);
    }
}




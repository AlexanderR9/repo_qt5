#include "lmath.h"
#include "lstring.h"
#include "lstatic.h"
#include "qdebug.h"

#include <QTime>
#include <QDebug>
#include <QtMath>


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
LBigInt::LBigInt(QString str_big_number, bool neg)
    :m_rawData(str_big_number.trimmed()),
    is_negative(neg)
{
    m_groups.clear();

    checkValidity();
    initGroups();
}
LBigInt::LBigInt(const LBigInt &other)
    :m_rawData(QString()),
      is_negative(false)
{
    m_groups.clear();
    if (other.invalid()) return;

    m_rawData = other.rawData();
    is_negative = other.isNegative();
    initGroups();
}
LBigInt::LBigInt(quint16 degree)
    :is_negative(false)
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
void LBigInt::increase(const LBigInt &other)
{
    if (invalid() || other.invalid()) return;

    QList<qint64> result;
    if (!isEqualSign(other)) //знаки противоположны, т.е. сложение сводится к вычитанию
    {
        is_negative = false;
        decreasePrivate(other, result);
        reloadData(result);
        if (isNull()) return;

        if (isSmaller_abs(other))
            is_negative = other.isNegative();
    }
    else //знаки одинаковы, т.е. производится попарное сложение групп, а знак не меняется
    {
        increasePrivate(other, result);
        reloadData(result);
    }
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
        is_negative = false;
        decreasePrivate(other, result);
        reloadData(result);
        if (isNull()) return;

        if (isSmaller_abs(other))
            is_negative = true;
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
    if (isNull()) is_negative = false;
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
void LBigInt::toDebug()
{
    qDebug()<<QString("LBigInt INFO: raw_data[%1] len[%2] validity[%3]").arg(rawData()).arg(len()).arg(invalid()?"INVALID":"OK");
    if (invalid()) return;

    qDebug()<<QString("sign[%1], groups[%2]").arg(is_negative?"-":"+").arg(groupsCount());
    for(int i=0; i<groupsCount(); i++)
        qDebug()<<QString("G[%1] -> [%2]*10^%3").arg(i).arg(m_groups.at(i)).arg(i*groupSize());
}
QString LBigInt::finalValue() const
{
    if (invalid()) return QString("BIG_NUM_INVALID");
    if (isNegative()) return QString("%1%2").arg("-").arg(m_rawData);
    return m_rawData;
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
        qint32 ai = groupAt(n-i-1);
        for (int j=0; j<m; j++)
        {
            qint32 aj = b.groupAt(m-j-1);
            mid_data.append(ai*aj);
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
        qint32 v = 0;
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






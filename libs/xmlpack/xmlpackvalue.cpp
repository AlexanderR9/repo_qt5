#include "xmlpackvalue.h"
#include "xmlpacktype.h"

#include <QDebug>
#include <QDataStream>


void LXMLPackValue::setUserValue(const QString &s_value, QString &err)
{
    bool ok;
    err.clear();

    if (isDouble)
    {
        double dv = s_value.toDouble(&ok);
        if (!ok) err = QString("WARNING: invalid converting [%1] to double").arg(s_value);
        else d_value = dv;
        return;
    }
    if (isDiscrete)
    {
        quint32 dis_v = s_value.toUInt(&ok);
        if (!ok) err = QString("WARNING: invalid converting [%1] to unsigned int").arg(s_value);
        else if (dis_v > 1) err = QString("WARNING: invalid discrete value [%1], must be 0/1").arg(dis_v);
        else i_value = dis_v;
        return;
    }
    if (isUnsigned)
    {
        quint64 uiv = s_value.toUInt(&ok);
        if (!ok) err = QString("WARNING: invalid converting [%1] to unsigned int").arg(s_value);
        else i_value = uiv;
        return;
    }

    qint64 iv = s_value.toInt(&ok);
    if (!ok) err = QString("WARNING: invalid converting [%1] to int").arg(s_value);
    else i_value = iv;
}
void LXMLPackValue::setUserDeviation(const QString &s_value, QString &err)
{
    bool ok;
    err.clear();

    double dv = s_value.toDouble(&ok);
    if (!ok) err = QString("WARNING: invalid converting str_deviation(%1) to double").arg(s_value);
    else if (dv < 0 || dv > 90) err = QString("WARNING: invalid deviation %1, must be [0 .. 90]").arg(dv);
    else rand_deviation = dv;
}
void LXMLPackValue::conversionIntType(int data_type)
{
    if (isDouble) return;

    switch (data_type)
    {
        case petInt8:       {qint8 v = qint8(i_value);       i_value = v; break;}
        case petInt16:      {qint16 v = qint16(i_value);     i_value = v; break;}
        case petInt32:      {qint32 v = qint32(i_value);     i_value = v; break;}
        case petUint8:      {quint8 v = quint8(i_value);     i_value = v; break;}
        case petUint16:     {quint16 v = quint16(i_value);   i_value = v; break;}
        case petUint32:     {quint32 v = quint32(i_value);   i_value = v; break;}

        case petDiscrete:
        {
            quint8 v = quint8(i_value);
            if (v > 1) v = 0;
            i_value = v;
            break;
        }

        default: break;
    }
}
void LXMLPackValue::reset()
{
    i_value = 0;
    d_value = rand_deviation = 0;
    isDouble = isDiscrete = isUnsigned = false;
}
void LXMLPackValue::copy(const LXMLPackValue &pv)
{
    i_value = pv.i_value;
    d_value = pv.d_value;
    rand_deviation = pv.rand_deviation;
    isDouble = pv.isDouble;
    isDiscrete = pv.isDiscrete;
    isUnsigned = pv.isUnsigned;
}
QString LXMLPackValue::toStr() const
{
    QString s("XMLPackValue:");
    QString s_type("integer");
    QString s_value = QString::number(i_value);
    QString s_err = QString::number(rand_deviation, 'f', 3);
    if (isDouble) {s_type = "double"; s_value = QString::number(d_value, 'f', 4);}
    else if (isDiscrete) s_type = "discrete";
    return QString("%1  value(%2)=%3   err=%4").arg(s).arg(s_type).arg(s_value).arg(s_err);
}
QString LXMLPackValue::strValue(quint8 precision) const
{
    if (isDouble) return QString::number(d_value, 'f', precision);
    return QString::number(i_value);
}
QString LXMLPackValue::strDeviation() const
{
    return QString::number(rand_deviation, 'f', 2);
}
void LXMLPackValue::recalcNext()
{
    if (rand_deviation < 0.01) return;

    if (isDiscrete)
    {
        double lim = rand_deviation*100;
        if (lim > qrand()%9999)
            i_value = ((i_value == 0) ? 1 : 0);
        return;
    }

    //находим случайное отклонение от -rand_deviation до rand_deviation (в этот раз)
    double r_err = r_sign()*(rand_deviation/double(1+qrand()%10));

    //находим обновленной значение с учетом случайной погрешности
    if (isDouble) d_value += d_value*r_err/double(100);
    else i_value += qint64(i_value*r_err/double(100));
}


//for stream funcs
void LXMLPackValue::writeToStream(int data_type, QDataStream &stream)
{
    switch (data_type)
    {
        case petInt8:       {stream << qint8(i_value); break;}
        case petInt16:      {stream << qint16(i_value); break;}
        case petInt32:      {stream << qint32(i_value); break;}
        case petInt64:      {stream << qint64(i_value); break;}

        case petDiscrete:
        case petUint8:      {stream << quint8(i_value); break;}
        case petUint16:     {stream << quint16(i_value); break;}
        case petUint32:     {stream << quint32(i_value); break;}
        case petUint64:     {stream << quint64(i_value); break;}

        case petFloat:      {stream << float(d_value); break;}
        case petDouble:     {stream << d_value; break;}
        default: break;
    }
}
void LXMLPackValue::readFromStream(int data_type, QDataStream &stream)
{
    switch (data_type)
    {
        case petInt8:       {qint8 v=0; stream >> v; i_value = v; break;}
        case petInt16:      {qint16 v=0; stream >> v; i_value = v; break;}
        case petInt32:      {qint32 v=0; stream >> v; i_value = v; break;}
        case petInt64:      {stream >> i_value; break;}

        case petUint8:      {quint8 v=0; stream >> v; i_value = v; break;}
        case petUint16:     {quint16 v=0; stream >> v; i_value = v; break;}
        case petUint32:     {quint32 v=0; stream >> v; i_value = v; break;}
        case petUint64:     {quint64 v=0; stream >> v; i_value = v; break;}

        case petDiscrete:
        {
            quint8 v=0;
            stream >> v;
            if (v > 1) v = 0;
            i_value = v;
            break;
        }

        case petFloat:      {float v=0; stream >> v; d_value = v; break;}
        case petDouble:     {stream >> d_value; break;}
        default: break;
    }
}







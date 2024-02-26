#include "bb_apistruct.h"
#include "lhttp_types.h"

#include <QStringList>

QString BB_APIReqParams::fullUri() const
{
    QString s;
    if (!invalid())
    {
        s = uri;
        QString ps(paramsLine().trimmed());
        if (!ps.isEmpty()) s = QString("%1?%2").arg(s).arg(ps);
    }
    return s;
}
QString BB_APIReqParams::paramsLine() const
{
    if (invalid() || params.isEmpty())  return QString();

    QString s;
    QStringList keys(params.keys());
    foreach (const QString &v, keys)
        s.append(QString("%1=%2&").arg(v).arg(params.value(v)));
    return s.left(s.length()-1);
}
bool BB_APIReqParams::invalid() const
{
    if (metod != hrmGet && metod != hrmPost) return true;
    return (name.isEmpty() || uri.isEmpty());
}
QString BB_APIReqParams::toStr() const
{
    QString s("BB_APIReqParams: ");
    s = QString("%1 name[%2] metod[%3] params[%4] uri[%5]  req_type[%6]").arg(s).arg(name).arg(metod).arg(params.count()).arg(uri).arg(req_type);
    s = QString("%1 validity[%2]").arg(s).arg(invalid()?"FAULT":"OK");
    return s;
}
QString BB_APIReqParams::strReqTypeByType(int t, QString s_extra)
{
    QString s;
    switch (t)
    {
        case rtCandles:     {s = "GET_CANDLES"; break;}
        case rtPositions:   {s = "GET_POSITIONS"; break;}
        case rtOrders:      {s = "GET_ORDERS"; break;}
        case rtHistory:     {s = "GET_HISTORY"; break;}
        case rtBag:         {s = "GET_WALLET"; break;}
        default: return "???";
    }
    return s_extra.isEmpty() ? s : QString("%1(%2)").arg(s).arg(s_extra);
}


//////////BB_BagState/////////////
float BB_BagState::sumFreezed() const
{
    float sum = freezed_order + freezed_pos;
    if (sum < 0) return -1;
    return sum;
}




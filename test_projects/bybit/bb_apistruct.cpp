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
                /*
        if (!params.isEmpty())
        {
            s.append("?");
            QStringList keys(params.keys());
            foreach (const QString &v, keys)
                s = QString("s%1%2&").arg(v).arg(params.value(v));
        }
                */
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
    s = QString("%1 name[%2] metod[%3] params[%4] uri[%5]").arg(s).arg(name).arg(metod).arg(params.count()).arg(uri);
    s = QString("%1 validity[%2]").arg(s).arg(invalid()?"FAULT":"OK");
    return s;
}





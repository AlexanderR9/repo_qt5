#include "ug_apistruct.h"
#include "lhttp_types.h"
#include "lstring.h"
#include "lfile.h"
//#include "apiconfig.h"


//#include <QtMath>
#include <QDir>
#include <QStringList>
#include <QJsonValue>
#include <QJsonObject>
#include <QDebug>
#include <QDateTime>

#define APP_DATA_FOLDER     QString("data")


//UG_APIReqParams
QString UG_APIReqParams::appDataPath()
{
    return QString("%1%2%3").arg(LFile::appPath()).arg(QDir::separator()).arg(APP_DATA_FOLDER);
}



//UG_APIReqParams
QString UG_APIReqParams::strReqTypeByType(int t, QString s_extra)
{
    QString s;
    switch (t)
    {
        case rtPools:           {s = "GET_POOL_DATA"; break;}
        case rtJsonView:        {s = "FREE_QUERY"; break;}

        default: return "???";
    }
    return s_extra.isEmpty() ? s : QString("%1(%2)").arg(s).arg(s_extra);
}


//UG_PoolInfo
void UG_PoolInfo::reset()
{
    id = "?";
    tvl = -1;
    token0.clear();
    token1.clear();
    token0_id.clear();
    token1_id.clear();
    fee = volume_all = 0;
    ts = 0;
}
bool UG_PoolInfo::invalid() const
{
    if (id.length() < 10) return true;
    if (token0.trimmed().isEmpty() || token1.trimmed().isEmpty()) return true;
    if (token0_id.trimmed().isEmpty() || token1_id.trimmed().isEmpty()) return true;
    if (token0.trimmed().length()>5 || token1.trimmed().length()>5) return true;
    if (tvl <= 0 || fee <= 0 || ts == 0) return true;
    return false;
}
void UG_PoolInfo::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    id = j_obj.value("id").toString();
    fee = j_obj.value("feeTier").toString().toDouble();
    tvl = j_obj.value("totalValueLockedUSD").toString().toDouble();
    volume_all = j_obj.value("volumeUSD").toString().toDouble();
    ts = j_obj.value("createdAtTimestamp").toString().toUInt();

    token0 = j_obj.value("token0").toObject().value("symbol").toString().trimmed();
    token0_id = j_obj.value("token0").toObject().value("id").toString().trimmed();
    token1 = j_obj.value("token1").toObject().value("symbol").toString().trimmed();
    token1_id = j_obj.value("token1").toObject().value("id").toString().trimmed();
    fee = fee/float(10000);
}
QString UG_PoolInfo::toStr() const
{
    QString s("PoolInfo: ");
    s = QString("%1 id[%2] tvl[%3] fee[%4]").arg(s).arg(id).arg(QString::number(tvl, 'f', 1)).arg(QString::number(fee, 'f', 2));
    s = QString("%1 volume_all[%2]").arg(s).arg(QString::number(volume_all, 'f', 1));
    s = QString("%1 tokens[%2/%3]").arg(s).arg(token0).arg(token1);
    s = QString("%1 t_creation[%2]").arg(s).arg(ts);

    return s;
}
void UG_PoolInfo::toTableRow(QStringList &list) const
{
    QDateTime dt = QDateTime::fromSecsSinceEpoch(ts);

    list.clear();
    list << id << QString::number(tvl, 'f', 1) << QString::number(volume_all, 'f', 1);
    list << token0 << token1 << QString::number(fee, 'f', 2);
    list << dt.toString(UG_APIReqParams::userDateMask());

}
QString UG_PoolInfo::toFileLine() const
{
    QString fline = QString("%1 / %2").arg(id).arg(ts);
    fline = QString("%1 / %2 / %3 / %4 / %5").arg(fline).arg(token0).arg(token0_id).arg(token1).arg(token1_id);
    fline = QString("%1 / %2").arg(fline).arg(QString::number(tvl, 'f', 2));
    fline = QString("%1 / %2").arg(fline).arg(QString::number(volume_all, 'f', 2));
    fline = QString("%1 / %2").arg(fline).arg(QString::number(fee, 'f', 2));
    return fline;
}




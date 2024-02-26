#include "apicommonsettings.h"
#include "lfile.h"
#include "lstring.h"
#include "lstaticxml.h"

#include <QDir>
#include <QDebug>
#include <QSettings>

#define APP_DATA_FOLDER     QString("data")
#define APP_CONFIG_FILE     QString("config.xml")
#define INVALID_DATE        QString("2000-01-01T01:01:01Z")
#define UID_CLONE_CONFIG    QString("clone_uid.xml")

//global var
API_CommonSettings api_commonSettings;


//API_CommonSettings CPP desc
QString API_CommonSettings::appDataPath()
{
    return QString("%1%2%3").arg(LFile::appPath()).arg(QDir::separator()).arg(APP_DATA_FOLDER);
}
QString API_CommonSettings::beginPoint(const InstrumentHistory::HItem &h_item, quint8 hour)
{
    if (h_item.begin_date.isEmpty()) return INVALID_DATE;
    if (hour > 23) hour = 0;
    QString s_hour = QString("%1:00:00").arg(hour);
    if (hour < 10) s_hour.prepend('0');
    return QString("%1T%2Z").arg(h_item.begin_date).arg(s_hour);
}
QString API_CommonSettings::endPoint(const InstrumentHistory::HItem &h_item, quint8 hour)
{
    if (h_item.end_date.isEmpty()) return INVALID_DATE;
    if (hour > 23) hour = 0;
    QString s_hour = QString("%1:00:00").arg(hour);
    if (hour < 10) s_hour.prepend('0');
    return QString("%1T%2Z").arg(h_item.end_date).arg(s_hour);
}
void API_CommonSettings::reset()
{
    user_id = -1;
    services.clear();
    service_colors.clear();
    candle_sizes.clear();
    token.clear();
    start_reqs.reset();
}
void API_CommonSettings::loadUidClones(QString &err)
{
    uid_clones.clear();
    QString fname = QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(UID_CLONE_CONFIG);
    if (!LFile::fileExists(fname))
    {
        err = QString("API_CommonSettings::loadUidClones - config file not found [%1]").arg(fname);
        return;
    }

    QFile f(fname.trimmed());
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        err = QString("API_CommonSettings::loadUidClones - config filename [%1] can't load to DomDocument").arg(fname);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    QDomNode root_node = dom.namedItem("clones");
    if (root_node.isNull())
    {
        err = QString("API_CommonSettings::loadUidClones: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg("clones");
        return;
    }

    parseUidClones(root_node);
}
void API_CommonSettings::parseUidClones(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "asset")
        {
            UidClone rec;
            rec.parseXmlNode(child_node);
            if (!rec.invalid()) uid_clones.append(rec);
        }
        child_node = child_node.nextSibling();
    }
}
void API_CommonSettings::loadConfig(QString &err)
{
    QString fname = QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(APP_CONFIG_FILE);
    if (!LFile::fileExists(fname))
    {
        err = QString("API_CommonSettings::loadConfig - config file not found [%1]").arg(fname);
        return;
    }

    QFile f(fname.trimmed());
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        err = QString("API_CommonSettings::loadConfig - config filename [%1] can't load to DomDocument").arg(fname);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    QDomNode root_node = dom.namedItem("params");
    if (root_node.isNull())
    {
        err = QString("API_CommonSettings::loadConfig: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg("params");
        return;
    }

    QDomNode services_node = root_node.namedItem("services");
    if (!services_node.isNull()) parseServicesNode(services_node);

    QDomNode candles_node = root_node.namedItem("candles");
    if (!candles_node.isNull()) parseCandlesNode(candles_node);

    QDomNode auto_start_node = root_node.namedItem("auto_start");
    if (!auto_start_node.isNull()) parseAutoStartNode(auto_start_node);

    QDomNode history_node = root_node.namedItem("history");
    if (!history_node.isNull()) parseHistoryNode(history_node);

    QDomNode global_filter_node = root_node.namedItem("global_filter");
    if (!global_filter_node.isNull()) parseGlobalFilterNode(global_filter_node);

}
void API_CommonSettings::parseHistoryNode(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    i_history.timeout = LStaticXML::getIntAttrValue("timeout", node, 2000);
    i_history.block_size = LStaticXML::getIntAttrValue("block_size", node, 100);

    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "prices") i_history.prices.parseConfigNode(child_node);
        else if (child_node.nodeName() == "coupons") i_history.coupons.parseConfigNode(child_node);
        else if (child_node.nodeName() == "divs") i_history.divs.parseConfigNode(child_node);
        else if (child_node.nodeName() == "events") i_history.events.parseConfigNode(child_node);

        child_node = child_node.nextSibling();
    }
}
void API_CommonSettings::parseGlobalFilterNode(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "bond") g_filter.bond.parseConfigNode(child_node);
        else if (child_node.nodeName() == "stock") g_filter.stock.parseConfigNode(child_node);
        child_node = child_node.nextSibling();
    }
}
void API_CommonSettings::parseServicesNode(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        QString cur_service;
        if (child_node.nodeName() == "service")
        {
            cur_service = LStaticXML::getStringAttrValue("name", child_node).trimmed();
            if (!cur_service.isEmpty())
            {
                QString s_color = LStaticXML::getStringAttrValue("color", child_node).trimmed();
                if (!s_color.isEmpty() && s_color.left(1) == "#") service_colors.insert(cur_service, s_color);

                //load metods
                QDomNode metod_node = child_node.firstChild();
                while (!metod_node.isNull())
                {
                    if (metod_node.nodeName() == "metod")
                    {
                        QString cur_metod = LStaticXML::getStringAttrValue("name", metod_node).trimmed();
                        if (!cur_metod.isEmpty())
                        {
                            services << QString("%1/%2").arg(cur_service).arg(cur_metod);
                            QString api_metod = LStaticXML::getStringAttrValue("api_metod", metod_node).trimmed();
                            if (!api_metod.isEmpty()) cycle_metods.insert(cur_metod, api_metod);
                        }
                    }
                    metod_node = metod_node.nextSibling();
                }
            }
        }
        child_node = child_node.nextSibling();
    }
}
void API_CommonSettings::parseCandlesNode(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "size")
        {
            QString key = LStaticXML::getStringAttrValue("key", child_node).trimmed();
            QString value = LStaticXML::getStringAttrValue("value", child_node).trimmed();
            if (!key.isEmpty() && !value.isEmpty()) candle_sizes.insert(key, value);
        }
        child_node = child_node.nextSibling();
    }
}
void API_CommonSettings::parseAutoStartNode(const QDomNode &node)
{
    start_reqs.reset();
    start_reqs.timeout = LStaticXML::getIntAttrValue("timeout", node, 3000);

    QMap<quint8, QString> map;
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "metod")
        {
            QString metod = LStaticXML::getStringAttrValue("name", child_node).trimmed();
            int step = LStaticXML::getIntAttrValue("i", child_node, -1);
            if (step > 0 && !metod.isEmpty()) map.insert(step, metod);
        }
        child_node = child_node.nextSibling();
    }

    //check sequence
    QList<quint8> keys(map.keys());
    for (int i=0; i<keys.count(); i++)
    {
        if (i+1 == keys.at(i))
        {
            start_reqs.src.append(map.value(i+1));
            //qDebug()<<QString("step=%1  src=%2").arg(i+1).arg(start_reqs.src.last());
        }
        else
        {
            qWarning()<<QString("API_CommonSettings::parseAutoStartNode WARNING invalid sequence, %1 != %2").arg(i+1).arg(keys.at(i));
            start_reqs.reset();
            break;
        }
    }
}
void API_CommonSettings::parseToken(QString commonSettingsToken)
{
    token = "?";
    commonSettingsToken = commonSettingsToken.trimmed();
    if (commonSettingsToken.length() < 10) return;

    int a = 4;
    QString s1 = commonSettingsToken.left(a);
    QString s2 = commonSettingsToken.right(a);
    commonSettingsToken = LString::strTrimLeft(commonSettingsToken, a);
    commonSettingsToken = LString::strTrimRight(commonSettingsToken, a);
    token = QString("%1%2%3%4").arg("t.").arg(s2).arg(commonSettingsToken).arg(s1);
}
bool API_CommonSettings::isCloneUid(const QString &uid) const
{
    if (!uid_clones.isEmpty())
    {
        foreach (const UidClone &v, uid_clones)
            if (v.clones.contains(uid)) return true;
    }
    return false;
}
bool API_CommonSettings::hasCloneUid(const QString &orig_uid) const
{
    if (!uid_clones.isEmpty())
    {
        foreach (const UidClone &v, uid_clones)
            if (v.uid == orig_uid) return true;
    }
    return false;
}
bool API_CommonSettings::isHasCloneBond(const QString &orig_uid) const
{
    if (!uid_clones.isEmpty())
    {
        foreach (const UidClone &v, uid_clones)
            if (v.uid == orig_uid) return v.is_bond;
    }
    return false;
}
QString API_CommonSettings::getOrigUidByClone(const QString &uid) const
{
    if (!uid_clones.isEmpty())
    {
        foreach (const UidClone &v, uid_clones)
            if (v.clones.contains(uid)) return v.uid;
    }
    return QString();
}
QString API_CommonSettings::getLastCloneUidByOrig(const QString &orig_uid) const
{
    if (!uid_clones.isEmpty())
    {
        foreach (const UidClone &v, uid_clones)
        {
            if (v.uid == orig_uid)
            {
                if (!v.clones.isEmpty()) return v.clones.last();
            }
            break;
        }
    }
    return QString();
}


/////////////////////////////
void API_CommonSettings::InstrumentHistory::HItem::parseConfigNode(const QDomNode &node)
{
    reset();
    if (node.isNull()) return;

    begin_date = LStaticXML::getStringAttrValue("begin", node).trimmed();
    end_date = LStaticXML::getStringAttrValue("end", node).trimmed();
   // qDebug()<<QString("LOADED PERIOD: %1").arg(toStr());
}
void API_CommonSettings::GlobalFilter::GFItem::parseConfigNode(const QDomNode &node)
{
    country = "all";
    if (node.isNull()) return;
    country = LStaticXML::getStringAttrValue("country", node).trimmed();
}
void API_CommonSettings::TradeDialog::save(QSettings &settings)
{
    settings.setValue(QString("API_CommonSettings/TradeDialog/deviation_index"), deviation_index);
    settings.setValue(QString("API_CommonSettings/TradeDialog/lots_index"), lots_index);
}
void API_CommonSettings::TradeDialog::load(QSettings &settings)
{
    deviation_index = settings.value(QString("API_CommonSettings/TradeDialog/deviation_index"), -1).toInt();
    lots_index = settings.value(QString("API_CommonSettings/TradeDialog/lots_index"), -1).toInt();
}
void API_CommonSettings::UidClone::parseXmlNode(const QDomNode &node)
{
    uid = LStaticXML::getStringAttrValue("uid", node).trimmed();
    ticker = LStaticXML::getStringAttrValue("ticker", node).trimmed();
    is_bond = (LStaticXML::getStringAttrValue("is_bond", node).trimmed() == "true");
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "clone")
        {
            QString c_uid = ticker = LStaticXML::getStringAttrValue("value", child_node).trimmed();
            if (!c_uid.isEmpty()) clones.append(c_uid);
        }
        child_node = child_node.nextSibling();
    }
}



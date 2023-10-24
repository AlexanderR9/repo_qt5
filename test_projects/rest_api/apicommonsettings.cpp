#include "apicommonsettings.h"
#include "lfile.h"
#include "lstring.h"
#include "lstaticxml.h"

#include <QDir>
#include <QDebug>

#define APP_DATA_FOLDER     QString("data")
#define APP_CONFIG_FILE     QString("config.xml")

//global var
API_CommonSettings api_commonSettings;


//API_CommonSettings CPP desc
QString API_CommonSettings::appDataPath()
{
    return QString("%1%2%3").arg(LFile::appPath()).arg(QDir::separator()).arg(APP_DATA_FOLDER);
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

}
void API_CommonSettings::parseHistoryNode(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "period")
        {
            i_history.begin_date = LStaticXML::getStringAttrValue("begin", child_node).trimmed();
            i_history.end_date = LStaticXML::getStringAttrValue("end", child_node).trimmed();
        }
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
                        if (!cur_metod.isEmpty()) services << QString("%1/%2").arg(cur_service).arg(cur_metod);
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
            qDebug()<<QString("step=%1  src=%2").arg(i+1).arg(start_reqs.src.last());
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





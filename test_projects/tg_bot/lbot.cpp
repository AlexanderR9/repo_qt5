#include "lbot.h"
#include "tgconfigloaderbase.h"

#include <QDebug>

//LBot
LBot::LBot(QObject *parent)
    :LTGAbstractBot(parent)
{
    setObjectName("lbot");

}
void LBot::slotJsonReceived(QJsonObject reply_obj)
{
    qDebug("LBot::slotJsonReceived");
    emit signalMsg(QString("answer json received!"));

    jsonToDebug(reply_obj);
    return;
}
void LBot::slotJArrReceived(QJsonArray j_arr)
{
    qDebug("LBot::slotJArrReceived");
    emit signalMsg(QString("answer json_arr received, size=%1").arg(j_arr.count()));

    for (int i=0; i<j_arr.count(); i++)
    {
        const QJsonValue &jv = j_arr.at(i);
        emit signalMsg(QString("  %1. %2").arg(i+1).arg(LBot::jsonValueToStr(jv)));

        if (jv.isObject())
        {
            jsonToDebug(jv.toObject());
        }
    }
}


//static funcs
void LBot::jsonToDebug(const QJsonObject &j_obj, quint8 level)
{
    if (level == 0) qDebug()<<QString("----------------ANSWER JSON OBJECT-------------------");
    QString space;
    if (level > 0)
        for (quint8 i=0; i<level; i++) space.append("  ");

    qDebug()<<QString("%1 JSON OBJ: size=%2  level=%3").arg(space).arg(j_obj.count()).arg(level);

    QStringList keys(j_obj.keys());
    for (int i=0; i<keys.count(); i++)
    {
        QJsonValue jv = j_obj[keys.at(i)];
        if (jv.isObject())
        {
            qDebug()<<QString("%1 key=[%2] : VALUE_OBJ").arg(space).arg(keys.at(i));
            jsonToDebug(jv.toObject(), level+1);
        }
        else qDebug()<<QString("%1 key=[%2] : (%3)").arg(space).arg(keys.at(i)).arg(jsonValueToStr(jv));
    }
}
QString LBot::jsonValueToStr(const QJsonValue &jv)
{
    QString s("---");

    if (jv.isArray()) s = QString("type=[%1]  count=[%2]").arg("array").arg(jv.toArray().count());
    else if (jv.isBool()) s = QString("type=[%1]  value=[%2]").arg("bool").arg(jv.toBool()?"true":"false");
    else if (jv.isDouble())
    {
        double d = jv.toDouble();
        if (d > 10000) s = QString("type=[%1]  value=[%2]").arg("double").arg(qint64(d));
        else QString("type=[%1]  value=[%2]").arg("double").arg(QString::number(d, 'f', 2));
    }
    else if (jv.isNull()) s = QString("type=[%1]").arg("null");
    else if (jv.isUndefined()) s = QString("type=[%1]").arg("undefined");
    else if (jv.isObject()) s = QString("type=[%1]").arg("object");
    else if (jv.isString()) s = QString("type=[%1]  value=[%2]").arg("string").arg(jv.toString());
    else s = QString("type=[%1(%2)]  value=[%3]").arg("??").arg(jv.type()).arg(jv.toString());

    return s;
}



//load func
void LBot::loadConfig(const QString &fname)
{
    QString err;
    reset();

    LTGConfigLoaderBase loader(fname);
    loader.loadBotParams(m_params, err);
    if (!err.isEmpty())
    {
        emit  signalError(err);
        return;
    }

    reinitSender();
}




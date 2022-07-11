#include "tgjsonworker.h"

#include <QDebug>

///////////////LJsonWorker/////////////////////////////////
QString LJsonWorker::strJValueType(const QJsonValue &value)
{
    switch (value.type())
    {
        case QJsonValue::Bool:      return QString("type_bool");
        case QJsonValue::Double:    return QString("type_double");
        case QJsonValue::String:    return QString("type_string");
        case QJsonValue::Array:     return QString("type_array");
        case QJsonValue::Object:    return QString("type_object");
        case QJsonValue::Null:      return QString("type_null");
        case QJsonValue::Undefined: return QString("type_undefined");
        default: break;
    }
    return "??";
}
QString LJsonWorker::strJValueValue(const QJsonValue &value)
{
    switch (value.type())
    {
        case QJsonValue::Bool:      return value.toBool() ? "true" : "false";
        case QJsonValue::Double:    return QString::number(value.toDouble(), 'f', 4);
        case QJsonValue::String:    return value.toString();
        default: break;
    }
    return "---";
}
void LJsonWorker::jsonToDebug(const QJsonObject &j_obj, quint8 level)
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
        else qDebug()<<QString("%1 key=[%2] : (%3/%4)").arg(space).arg(keys.at(i)).arg(strJValueType(jv)).arg(strJValueValue(jv));
    }
}
void LJsonWorker::convertJArrToTGUpdates(const QJsonArray &jarr, QList<LTGUpdate> &tg_updates)
{
    tg_updates.clear();
    foreach (const QJsonValue &value, jarr)
    {
        QJsonObject jobj = value.toObject();
        QStringList keys = jobj.keys();
        if (keys.contains("update_id") && keys.contains("message"))
        {
            QJsonObject msg_obj = jobj.value("message").toObject();
            if (msg_obj.keys().contains("chat") && msg_obj.keys().contains("date") && msg_obj.keys().contains("text"))
            {
                LTGUpdate update(msg_obj.value("text").toString());
                update.update_id = jobj.value("update_id").toInt();
                update.dt.setSecsSinceEpoch(msg_obj.value("date").toInt());
                update.chat_id = msg_obj.value("chat").toObject().value("id").toInt();
                tg_updates.append(update);
            }
            else qWarning()<<QString("LJsonWorker::convertJArrToTGUpdates - WARNING: !msg_obj.keys().contains(chat) || !msg_obj.keys().contains(date) || !msg_obj.keys().contains(text)");
        }
        else qWarning()<<QString("LJsonWorker::convertJArrToTGUpdates - WARNING: !keys.contains(update_id) || !keys.contains(message)");
    }
}


//LTGUpdate
QString LTGUpdate::toStr() const
{
    QString s = QString("LTGUpdate:  %1  update_id=%2  chat_id=%3  text=[%4]  valitity(%5)").arg(dt.toString("dd.MM.yyyy hh:mm:ss")).
            arg(update_id).arg(chat_id).arg(text).arg(invalid()? "fault" : "Ok");
    return s;
}

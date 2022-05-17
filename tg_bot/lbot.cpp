#include "lbot.h"
#include "lstatic.h"
#include "tgsender.h"


#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDomDocument>
#include <QDomNode>
#include <QJsonDocument>
#include <QJsonArray>



LBot::LBot(QObject *parent)
    :LSimpleObject(parent),
    m_chatID(-1),
    m_sender(NULL)
{
    setObjectName("lbot");
    m_token = QString("???");

}
void LBot::init()
{
    if (m_token.isEmpty())
    {
        emit signalError("LBot: WARNING - token is empty");
        return;
    }


    signalMsg("Bot object initializate OK!");
    signalMsg(QString("Bot token: [%1]").arg(m_token));
    signalMsg(QString("Chat ID: [%1]").arg(m_chatID));


    initSender();
}
void LBot::initSender()
{
    if (m_sender) return;
    if (m_token.isEmpty()) return;

    m_sender = new TGSender(m_token, this);
    connect(m_sender, SIGNAL(signalJsonReceived(QJsonObject)), this, SLOT(slotJsonReceived(QJsonObject)));
    connect(m_sender, SIGNAL(signalJArrReceived(QJsonArray)), this, SLOT(slotJArrReceived(QJsonArray)));
    connect(m_sender, SIGNAL(signalFinishedFault()), this, SLOT(slotFinishedFault()));

}
/*
void LBot::initNetObjects()
{
    m_request = new QNetworkRequest();
    m_request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_netManager = new QNetworkAccessManager(this);
    connect(m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotRequestFinished(QNetworkReply*)));
    connect(this, SIGNAL(signalJsonReceived(QJsonObject)), this, SLOT(slotJsonReceived(QJsonObject)));
    connect(this, SIGNAL(signalJArrReceived(QJsonArray)), this, SLOT(slotJArrReceived(QJsonArray)));
}
*/
void LBot::getMe()
{
    QJsonObject jsonObject;
    m_sender->sendJsonRequest(jsonObject, TGSender::tgrcGetMe);

    if (m_sender->hasErr())
    {
        emit signalError(QString("LBot::getMe() - err=[%1]").arg(m_sender->err()));
    }
}
void LBot::sendMsg(const QString &text)
{
    /*
    m_reqCode = tgrcSendTextMsg;
    QJsonObject jsonObject;
    jsonObject["chat_id"] = m_chatID;
    jsonObject["text"] = text;
    jsonObject["disable_web_page_preview"] = false;
    jsonObject["disable_notification"] = false;
    sendJsonRequest(jsonObject);
    */
}
void LBot::getUpdates()
{
    /*
    m_reqCode = tgrcGetUpdates;
    QJsonObject jsonObject;
    jsonObject["limit"] = 2;
    jsonObject["timeout"] = 4;
    jsonObject["offset"] = 529822266;
    sendJsonRequest(jsonObject);
    */
}
/*
void LBot::sendJsonRequest(const QJsonObject &json_obj)
{
    QString api_method = LBot::apiMetodByReqCode(m_reqCode);
    emit signalMsg(QString("Try send json request, metod=[%1]...........").arg(api_method));
    if (api_method.trimmed().isEmpty())
    {
        signalError(QString("request code invalid: %1").arg(m_reqCode));
        return;
    }

    qDebug("");
    qDebug() << QString("SEND REQUEST (%1)").arg(api_method);
    QUrl request_url = QString("%1/%2").arg(bot_url).arg(api_method);
    m_request->setUrl(request_url);
    m_netManager->post(*m_request, QJsonDocument(json_obj).toJson());
}
void LBot::slotRequestFinished(QNetworkReply *reply)
{
    if (reply)
    {
        QJsonObject result = QJsonDocument::fromJson(reply->readAll()).object();
        if (result.count() < 2)
        {
            emit signalError(QString("answered json invalid, size=%1").arg(result.count()));
            return;
        }

        QStringList keys = result.keys();
        if (!keys.contains("ok"))
        {
            emit signalError(QString("answered json invalid, not found key [ok]"));
            return;
        }
        if (!keys.contains("result"))
        {
            emit signalError(QString("answered json invalid, not found key [result]"));
            return;
        }
        if (!result["ok"].toBool())
        {
            emit signalError(QString("answered json invalid, value[ok]=false"));
            return;
        }
        if (!result["result"].isObject() && !result["result"].isArray())
        {
            emit signalError(QString("answered json invalid, value[result] is not json_object"));
            return;
        }

        jsonToDebug(result);

        if (result["result"].isArray()) emit signalJArrReceived(result["result"].toArray());
        else emit signalJsonReceived(result["result"].toObject());
    }
    else emit signalError(QString("answered json invalid, reply is null"));
}
*/
void LBot::slotJsonReceived(QJsonObject reply_obj)
{
    qDebug("------------------------------");
    qDebug("LBot::slotJsonReceived");
    emit signalMsg(QString("answer json received!"));

    jsonToDebug(reply_obj);
    return;

    qDebug("------------------------------");
    QStringList keys = reply_obj.keys();
    qDebug()<<QString("keys %1,  size %2, count %3, len %4").arg(keys.count()).arg(reply_obj.size()).arg(reply_obj.count()).arg(reply_obj.length());

    qDebug("------------------------------");
    qDebug("KEYS:");
    for (int i=0; i<keys.count(); i++)
    {
        QJsonValue jv = reply_obj[keys.at(i)];
        QString value_type = "??";
        if (jv.isArray()) value_type = "array";
        else if (jv.isBool()) value_type = "bool";
        else if (jv.isDouble()) value_type = "double";
        else if (jv.isNull()) value_type = "null";
        else if (jv.isObject()) value_type = "object";
        else if (jv.isString()) value_type = "string";
        qDebug()<<keys.at(i) << QString("   type=[%1]").arg(value_type);
    }
}
void LBot::slotJArrReceived(QJsonArray j_arr)
{
    qDebug("------------------------------");
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
void LBot::slotFinishedFault()
{
    emit signalError(QString("req finished fault: err=[%1]").arg(m_sender->err()));
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
    if (fname.trimmed().isEmpty())
    {
        err = QString("LBot: WARNING - config filename is empty ");
        emit  signalError(err);
        return;
    }
    QFile f(fname.trimmed());
    if (!f.exists())
    {
        err = QString("LBot: WARNING - config filename [%1] not found").arg(fname);
        emit  signalError(err);
        return;
    }
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        err = QString("LBot: WARNING - config filename [%1] can't load to DomDocument").arg(fname);
        emit  signalError(err);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    QString root_node_name("config");
    QDomNode root_node = dom.namedItem(root_node_name);
    if (root_node.isNull())
    {
        err = QString("LBot: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg(root_node_name);
        emit signalError(err);
        return;
    }

    QString token_node_name("token");
    QDomNode token_node = root_node.namedItem(token_node_name);
    if (token_node.isNull())
    {
        err = QString("LBot: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg(token_node_name);
        emit signalError(err);
        return;
    }
    QString chat_id_node_name("chat_id");
    QDomNode chat_id_node = root_node.namedItem(chat_id_node_name);
    if (chat_id_node.isNull())
    {
        err = QString("LBot: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg(chat_id_node_name);
        emit signalError(err);
        return;
    }

    m_token = LStatic::getStringAttrValue("value", token_node);
    if (m_token.isEmpty())
    {
        err = QString("LBot: invalid token value.").arg(fname).arg(token_node_name);
        emit signalError(err);
        return;
    }

    m_chatID = LStatic::getIntAttrValue("value", chat_id_node);

     err = QString("MBConfigLoader: config loaded ok!");
     emit signalMsg(err);
     err = QString("token = [%1]").arg(m_token);
     emit signalMsg(err);

     init();
}




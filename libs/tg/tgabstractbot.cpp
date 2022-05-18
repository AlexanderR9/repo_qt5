#include "tgabstractbot.h"
#include "tgsender.h"

#include <QDebug>


//LTGAbstractBot
LTGAbstractBot::LTGAbstractBot(QObject *parent)
    :LSimpleObject(parent),
    m_sender(NULL)
{
    setObjectName("tg_abstract_bot");
}
bool LTGAbstractBot::invalid() const
{
    if (m_params.token.trimmed().isEmpty()) return true;
    if (m_params.chatID <= 0) return true;
    return false;
}
void LTGAbstractBot::setBotParams(const LTGParamsBot &p)
{
    m_params.setParams(p);
    reinitSender();
}
void LTGAbstractBot::reinitSender()
{
    if (m_sender) {delete m_sender; m_sender = NULL;}

    if (invalid())
    {
        emit signalError(QString("TGAbstractBot::reinitSender() - invalid bot params (%1).").arg(m_params.toStr()));
        return;
    }

    m_sender = new LTGSender(m_params.token, this);
    connect(m_sender, SIGNAL(signalJsonReceived(QJsonObject)), this, SLOT(slotJsonReceived(QJsonObject)));
    connect(m_sender, SIGNAL(signalJArrReceived(QJsonArray)), this, SLOT(slotJArrReceived(QJsonArray)));
    connect(m_sender, SIGNAL(signalFinishedFault()), this, SLOT(slotFinishedFault()));

    signalMsg("Bot object initializate OK!");
    signalMsg(m_params.toStr());
}
void LTGAbstractBot::slotFinishedFault()
{
    emit signalError(QString("req finished fault: err=[%1]").arg(m_sender->err()));
}
void LTGAbstractBot::getMe()
{
    QJsonObject jsonObject;
    m_sender->sendJsonRequest(jsonObject, LTGSender::tgrcGetMe);

    if (m_sender->hasErr())
        emit signalError(QString("TGAbstractBot::getMe() - err=[%1]").arg(m_sender->err()));
}
void LTGAbstractBot::sendMsg(const QString &text)
{
    QJsonObject jsonObject;
    jsonObject["chat_id"] = m_params.chatID;
    jsonObject["text"] = text;
    jsonObject["disable_web_page_preview"] = false;
    jsonObject["disable_notification"] = false;
    m_sender->sendJsonRequest(jsonObject, LTGSender::tgrcSendTextMsg);

    if (m_sender->hasErr())
        emit signalError(QString("TGAbstractBot::sendMsg() - err=[%1]").arg(m_sender->err()));
}
void LTGAbstractBot::getUpdates(qint64 last_update_id)
{
    QJsonObject jsonObject;
    jsonObject["limit"] = m_params.limit_msg;
    jsonObject["timeout"] = m_params.req_timeout;

    if (last_update_id > 0)
        jsonObject["offset"] = last_update_id + 1;

    m_sender->sendJsonRequest(jsonObject, LTGSender::tgrcGetUpdates);

    if (m_sender->hasErr())
        emit signalError(QString("TGAbstractBot::getUpdates() - err=[%1]").arg(m_sender->err()));
}


///////////////LTGParamsBot/////////////////////////////////
void LTGParamsBot::setParams(const LTGParamsBot &p)
{
    token = p.token;
    chatID = p.chatID;
    req_timeout = p.req_timeout;
    limit_msg = p.limit_msg;
}
QString LTGParamsBot::toStr() const
{
    return QString("TGParamsBot: tocken=[%1] chatID=[%2] req_timeout=[%3] limit_msg=[%4]").
            arg(token).arg(chatID).arg(req_timeout).arg(limit_msg);
}
void LTGParamsBot::reset()
{
    token.clear();
    chatID = -1;
    req_timeout = 3;
    limit_msg = 20;
}





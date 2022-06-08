#include "tgbot.h"


TGBot::TGBot(QObject *parent)
    :LTGAbstractBot(parent)
{

}
void TGBot::slotJsonReceived(QJsonObject jobj)
{

}
void TGBot::slotJArrReceived(QJsonArray jarr)
{

}
QString TGBot::toStrParams() const
{
    return QString("BOT PARAMS: %1").arg(this->m_params.toStr());
}
QMap<QString, QString> TGBot::getParams() const
{
    QMap<QString, QString> map;
    if (invalid())
    {
        map.insert("State", "FAULT_CONFIG");
        return map;
    }

    map.insert("Token", m_params.token);
    map.insert("Chat ID", QString::number(m_params.chatID));
    map.insert("Limit msg", QString::number(m_params.limit_msg));
    map.insert("Timeout, sec.", QString::number(m_params.req_timeout));
    map.insert("State", "OK");
    return map;
}


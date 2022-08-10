#include "tgbot.h"
#include "logpage.h"
#include "tgjsonworker.h"
#include "lstatic.h"
#include "cfdconfigobj.h"

#include <QDebug>

#define UPDATE_INTERVAL_PERIOD1     24      //hours
#define UPDATE_INTERVAL_PERIOD2     24*3    //hours
#define UPDATE_INTERVAL_PERIOD3     24*10   //hours


//TGBot
TGBot::TGBot(const CalcActionParams &act_params, QObject *parent)
    :LTGAbstractBot(parent),
    last_update_id(-1),
    m_actParams(act_params)
{
    m_msgs.clear();
}
void TGBot::slotJsonReceived(QJsonObject jobj)
{
    qDebug("TGBot::slotJsonReceived");
}
void TGBot::slotTimer()
{
    this->getUpdates(last_update_id);
}
void TGBot::receivedUpdates(const QList<LTGUpdate> &updates)
{
    //qDebug()<<QString("TGBot::receivedUpdates -  updates_size %1").arg(updates.count());
    if (updates.isEmpty())
    {
        last_update_id = -1;
        return;
    }

    last_update_id = updates.last().update_id;
    for (int i=0; i<updates.count(); i++)
    {
        parseUpdate(updates.at(i));
    }
}
void TGBot::parseUpdate(const LTGUpdate &update)
{
    //qDebug()<<update.toStr();
    if (m_params.chatID != update.chat_id)
    {
        QString err = QString("UPDATE: warning chat_id(%1)").arg(update.chat_id);
        sendLog(err, 2);
        err = QString("Received update invalid, update.chat_id(%1) != %2").arg(update.chat_id).arg(m_params.chatID);
        return;
    }

    QString req = update.text.trimmed();
    QStringList list = LStatic::trimSplitList(req, LStatic::spaceSymbol());
    if (list.count() == 2 && list.first().toLower() == "get")
    {
        replyLastPrice(list.last());
    }
}
void TGBot::replyLastPrice(const QString &ticker)
{
    double price = 0;
    int hours_ago;
    emit signalGetLastPrice(ticker, price, hours_ago);
    QString msg("Invalid ticker!!!");
    if (price >= 0) msg = QString("Last price: %1,    hours ago %2").arg(QString::number(price, 'f', 2)).arg(hours_ago);
    this->sendMsg(msg);
}
void TGBot::slotNewChangingPrices(const QString &ticker, const QList<double> &changing_data)
{
    if (changing_data.count() != 3)
    {
        QString msg = QString("(%1) Invalid changing_data size: %2 != 3").arg(ticker).arg(changing_data.count());
        sendLog(msg, 1);
        return;
    }

    /*
    qDebug()<<QString("TGBot::slotNewChangingPrices - %1:  d1=%2%  d2=%3%  d3=%4%").arg(ticker).
              arg(QString::number(changing_data.at(0), 'f', 2)).
              arg(QString::number(changing_data.at(1), 'f', 2)).
              arg(QString::number(changing_data.at(2), 'f', 2));
              */

    if (qAbs(changing_data.at(0)) > m_actParams.notice_day_size)
        trySendDeviation(ticker, changing_data.at(0), 1);

    if (qAbs(changing_data.at(1)) > m_actParams.notice_week_size)
        trySendDeviation(ticker, changing_data.at(1), 2);

    if (qAbs(changing_data.at(2)) > m_actParams.notice_month_size)
        trySendDeviation(ticker, changing_data.at(2), 3);

}
void TGBot::trySendDeviation(const QString &ticker, const double &d, int period_type)
{
    int pos = findMsg(ticker, period_type);
    if (pos < 0)
    {
        TGMsg msg(ticker, period_type);
        msg.deviation = d;
        m_msgs.append(msg);
        sendDeviation(m_msgs.last());
    }
    else
    {
        if (needUpdateInfo(m_msgs.at(pos), d))
        {
            m_msgs[pos].deviation = d;
            m_msgs[pos].dt = QDateTime::currentDateTime();
            sendDeviation(m_msgs.at(pos));
        }
    }
}
void TGBot::sendDeviation(const TGMsg &msg)
{
    QString text = QString("%1:  period [%2],  deviation=%3%").arg(msg.ticker).arg(msg.strPeriod()).arg(msg.strDeviation());
    this->sendMsg(text);

    text = QString("sended tg_message (%1 : %2%)").arg(msg.ticker).arg(msg.strDeviation());
    sendLog(text, 0);
}
int TGBot::findMsg(const QString &ticker, int period_type) const
{
    for (int i=0; i<m_msgs.count(); i++)
    {
        if (m_msgs.at(i).ticker == ticker && m_msgs.at(i).period_type == period_type)
            return i;
    }
    return -1;
}
bool TGBot::needUpdateInfo(const TGMsg &msg, const double &d) const
{
    if (qAbs(d)/qAbs(msg.deviation) > 1.3) return true;
    if (qAbs(d)/qAbs(msg.deviation) < 0.7) return true;

    QDateTime dt = QDateTime::currentDateTime();
    double d_hours = msg.dt.secsTo(dt)/3600;
    switch (msg.period_type)
    {
        case 1:
        {
            if (d_hours > UPDATE_INTERVAL_PERIOD1) return true;
            break;
        }
        case 2:
        {
            if (d_hours > UPDATE_INTERVAL_PERIOD2) return true;
            break;
        }
        case 3:
        {
            if (d_hours > UPDATE_INTERVAL_PERIOD3) return true;
            break;
        }
        default: break;
    }
    return false;
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
void TGBot::sendLog(const QString &msg, int result)
{
    LogStruct log(amtTGBot, result);
    log.msg = msg;
    emit signalSendLog(log);
}


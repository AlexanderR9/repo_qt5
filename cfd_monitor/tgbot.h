#ifndef TGBOT_H
#define TGBOT_H


#include "tgabstractbot.h"

#include <QMap>
#include <QDateTime>

struct LogStruct;
struct CalcActionParams;

//TGMsg
struct TGMsg
{
    TGMsg(const QString &t, int p) :ticker(t), period_type(p), deviation(0) {dt = QDateTime::currentDateTime();}

    QString ticker;
    QDateTime dt;
    int period_type; // 1-day, 2-week, 3-month
    double deviation; // deviation price by period, %

    QString strPeriod() const
    {
        switch (period_type)
        {
            case 1: return QString("DAY");
            case 2: return QString("WEEK");
            case 3: return QString("MONTH");
            default: break;
        }
        return "??";
    }
    QString strDeviation() const {return QString::number(deviation, 'f', 1);}
};



// TGBot
class TGBot : public LTGAbstractBot
{
    Q_OBJECT
public:
    TGBot(const CalcActionParams&, QObject*);
    virtual ~TGBot() {}

    QString name() const {return QString("MyLBot");}
    QString toStrParams() const;

    QMap<QString, QString> getParams() const;

protected:
    QList<TGMsg> m_msgs; //список сообщений отправленных пользователю
    qint64 last_update_id; //id последнего полученного сообщения
    const CalcActionParams &m_actParams;

    void sendLog(const QString&, int);
    void trySendDeviation(const QString&, const double&, int); //попытка отправить сообщение пользователю
    void sendDeviation(const TGMsg&); //отправить сообщение пользователю
    int findMsg(const QString&, int) const;
    bool needUpdateInfo(const TGMsg&, const double&) const; //проверка необходимости обновить данные об изменнении цены инструмента
    void receivedUpdates(const QList<LTGUpdate>&);
    void parseUpdate(const LTGUpdate&); //проверить пришедшее сообщение/запрос и при необходимости ответить
    void replyLastPrice(const QString&); //выдать текущую цену по заданному тикеру

protected slots:
    void slotJsonReceived(QJsonObject);
    void slotTimer();

public slots:
    void slotNewChangingPrices(const QString&, const QList<double>&); //получены новые данные об изменнении цены инструмента

signals:
    void signalSendLog(const LogStruct&);
    void signalGetLastPrice(const QString&, double&, int&);
    void signalGetInstaPtr(const QString&, bool&);



};


#endif //TGBOT_H



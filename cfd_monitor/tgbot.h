#ifndef TGBOT_H
#define TGBOT_H


#include "tgabstractbot.h"

#include <QMap>
#include <QDateTime>

struct LogStruct;


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
    TGBot(QObject*);
    virtual ~TGBot() {}

    QString name() const {return QString("MyLBot");}
    QString toStrParams() const;

    QMap<QString, QString> getParams() const;

protected:
    QList<TGMsg> m_msgs;

    void sendLog(const QString&, int);
    void trySendDeviation(const QString&, const double&, int);
    void sendDeviation(const TGMsg&);
    int findMsg(const QString&, int) const;
    bool needUpdateInfo(const TGMsg&, const double&) const;

protected slots:
    void slotJsonReceived(QJsonObject);
    void slotJArrReceived(QJsonArray);

public slots:
    void slotNewChangingPrices(const QString&, const QList<double>&);

signals:
    void signalSendLog(const LogStruct&);



};


#endif //TGBOT_H



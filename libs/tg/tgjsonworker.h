#ifndef TG_JSON_WORKER_H
#define TG_JSON_WORKER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>


//LTGUpdate (принятое ботом сообщение/картинка/и т.п.)
struct LTGUpdate
{
    LTGUpdate() :update_id(-1), chat_id(-1) {}
    LTGUpdate(const QString &s) :update_id(-1), chat_id(-1), text(s) {}

    qint64 update_id;
    qint64 chat_id;
    QDateTime dt;
    QString text;

    bool invalid() const {return (!dt.isValid() || update_id <= 0 || chat_id <= 0);}
    QString toStr() const;

};


//static LJsonWorker
class LJsonWorker
{
public:
    static QString strJValueType(const QJsonValue&);
    static QString strJValueValue(const QJsonValue&);
    static void jsonToDebug(const QJsonObject&, quint8 level = 0); //выхлоп объекта json в debug
    static void convertJArrToTGUpdates(const QJsonArray&, QList<LTGUpdate>&); //преобразует QJsonArray с список структур LTGUpdate (это полученные сообщения)

};



#endif // TG_JSON_WORKER_H


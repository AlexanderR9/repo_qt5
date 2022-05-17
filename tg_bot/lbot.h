#ifndef LBOT_H
#define LBOT_H


#include <lsimpleobj.h>


#include <QJsonObject>
#include <QJsonArray>

class TGSender;

// LBot
class LBot : public LSimpleObject
{
    Q_OBJECT
public:
    LBot(QObject*);
    virtual ~LBot() {}

    void loadConfig(const QString&);
    QString name() const {return QString("tgbot_obj");}

    //tg funcs
    void getMe();
    void getUpdates();
    void sendMsg(const QString&);

    //static service funcs
    static void jsonToDebug(const QJsonObject&, quint8 level = 0);
    static QString jsonValueToStr(const QJsonValue&);

protected:
    QString                 m_token;
    qint64                  m_chatID;
    TGSender                *m_sender;


    void init();
    void initSender();
    //void sendJsonRequest(const QJsonObject&);

protected slots:
    //void slotRequestFinished(QNetworkReply*);
    void slotJsonReceived(QJsonObject);
    void slotJArrReceived(QJsonArray);
    void slotFinishedFault();

signals:
    //void signalJsonReceived(QJsonObject);
    //void signalJArrReceived(QJsonArray);

};



#endif // LBOT_H

#ifndef LBOT_H
#define LBOT_H


#include <lsimpleobj.h>

#include <TarnaBot>
using namespace Telegram;

#include <QJsonObject>

class QNetworkRequest;
class QNetworkAccessManager;
class QNetworkReply;

enum TGRequestCode {tgrcGetMe = 601, tgrcInvalid = -1};

// LBot
class LBot : public LSimpleObject
{
    Q_OBJECT
public:
    LBot(QObject*);
    virtual ~LBot();

    void loadConfig(const QString&);
    QString name() const {return QString("tgbot_obj");}

    void getMe();
    void getUpdates();
    void sendMsg();


protected:
    QString      m_token;
    TarnaBot    *m_botObj;
    QString     bot_url;
    QNetworkRequest *m_request;
    QNetworkAccessManager *m_netManager;
    int m_reqCode;


    void init();
    void sendJsonRequest(const QJsonObject&, const QString&);

protected slots:
    void slotRequestFinished(QNetworkReply*);
    void slotJsonReceived(QJsonObject);

signals:
    void signalJsonReceived(QJsonObject);

};



#endif // LBOT_H

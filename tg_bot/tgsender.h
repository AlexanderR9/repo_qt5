#ifndef TG_SENDER_H
#define TG_SENDER_H


#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class QNetworkRequest;
class QNetworkAccessManager;
class QNetworkReply;


// TGSender
class TGSender : public QObject
{
    Q_OBJECT
public:
    enum TGRequestCode {tgrcGetMe = 601, tgrcSendTextMsg, tgrcGetUpdates, tgrcInvalid = -1};

    TGSender(const QString &token,  QObject *parent = NULL);
    virtual ~TGSender() {if (m_request) delete m_request;}

    void sendJsonRequest(const QJsonObject&, int);

    static QString apiMetodByReqCode(int);
    static QString tgUrlBase() {return QString("https://api.telegram.org");}

    inline QString err() const {return m_err;}
    inline bool hasErr() const {return !m_err.isEmpty();}

protected:
    QString                  m_err;
    const QString           &m_botToken;
    QNetworkRequest         *m_request;
    QNetworkAccessManager   *m_netManager;

    void initNetObjects();

protected slots:
    void slotRequestFinished(QNetworkReply*);

signals:
    void signalJsonReceived(QJsonObject);
    void signalJArrReceived(QJsonArray);
    void signalFinishedFault();

private:
    QString requestUrl(const QString&) const;

};


#endif //TG_SENDER_H

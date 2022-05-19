#ifndef TG_SENDER_H
#define TG_SENDER_H


#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class QNetworkRequest;
class QNetworkAccessManager;
class QNetworkReply;

//класс для отправки запросов к серверу TG, и получения ответов от него.
//обмен идет через json объекты.

// LTGSender
class LTGSender : public QObject
{
    Q_OBJECT
public:
    enum TGRequestCode {tgrcGetMe = 601, tgrcSendTextMsg, tgrcGetUpdates, tgrcInvalid = -1};

    LTGSender(const QString &token,  QObject *parent = NULL);
    virtual ~LTGSender();

    void sendJsonRequest(const QJsonObject&, int); //отправить запрос, int - код команды из множества TGRequestCode

    static QString apiMetodByReqCode(int); //преобразовать код команды из множества TGRequestCode в строковое название функции для TG API
    static QString tgUrlBase() {return QString("https://api.telegram.org");}

    inline QString err() const {return m_err;} //ошибка возникшая при выполнении запроса
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
    void signalJsonReceived(QJsonObject); //пришел нормальный ответ в виде QJsonObject
    void signalJArrReceived(QJsonArray); //пришел нормальный ответ в виде QJsonArray
    void signalFinishedFault(); //запрос завершился неудачно, описание ошибки в переменной m_err

private:
    QString requestUrl(const QString&) const; //полное значение url для текущего запроса

};


#endif //TG_SENDER_H

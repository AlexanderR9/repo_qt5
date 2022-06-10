 #ifndef LHTML_REQUESTER_H
 #define LHTML_REQUESTER_H


#include "lhtmlrequesterbase.h"

#include <QNetworkRequest>


class QNetworkAccessManager;
class QNetworkReply;


//класс для посылки http запросов на получение html кода заданной страницы
//как пользоваться: наобходимо создать объект LHTMLRequester на постоянной основе,
//подключить сигналы signalError и signalFinished
//далее задать URL методом setUrl(const QString&),
//далее послать запрос методом startRequest(),
//далее ждать прихода одного из сигналов либо signalError либо signalFinished
//если пришел сигнал signalFinished то можно получить код html методом getHtmlData(QString&)

//примечание: код html будет храниться в переменной m_replyBuffer до сл. запроса.
//            url можно задавать перед каждым запросом, а можно только один раз (зависит от задачи).


// LHTMLRequester
class LHTMLRequester : public LHTMLRequesterBase
{
    Q_OBJECT
public:
    LHTMLRequester(QObject *parent = NULL);
    virtual ~LHTMLRequester() {}

    void startRequest();
    void getHtmlData(QString&);
    void initHttpHeaders();

    inline int replySize() const {return m_replyBuffer.count();}    //размер ответа
    inline bool replyEmpty() const {return m_replyBuffer.isEmpty();}    //пустой ответ

    
protected:
    QNetworkAccessManager *m_manager; 
    QNetworkRequest m_request;
    QByteArray m_replyBuffer;
    
    void checkUrl(bool&);

protected slots:
    void slotFinished(QNetworkReply*);
    void slotAccessibleChanged();
    void slotAuthenticationRequired();
    void slotSSLErrors();



};


 #endif

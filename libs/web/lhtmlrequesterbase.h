#ifndef LHTML_REQUESTER_BASE_H
#define LHTML_REQUESTER_BASE_H

#include <QObject>


// LHTMLRequesterBase
class LHTMLRequesterBase : public QObject
{
    Q_OBJECT
public:
    LHTMLRequesterBase(QObject *parent = NULL);
    virtual ~LHTMLRequesterBase() {}

    virtual void setUrl(const QString&); //установить значение адреса в m_url
    virtual void startRequest() = 0; //выполнить запрос страницы

    inline bool isBuzy() const {return is_buzy;} //признак того что в текущий момент идет запрос
    inline QString url() const {return m_url;}

protected:
    QString m_url;
    bool is_buzy;

    virtual void checkUrl(bool&); //проверка адреса URL

signals:
    void signalError(const QString&); //эмитится когда при попытке запроса произошла ошибка
    void signalFinished(bool); //bool - обобщенный результат запроса, эмитится когда запрос выполнен

};


 #endif //LHTML_REQUESTER_BASE_H

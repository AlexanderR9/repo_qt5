#ifndef LHTML_PAGE_REQUESTER_H
#define LHTML_PAGE_REQUESTER_H

#include "lhtmlrequesterbase.h"

class QWebEnginePage;

// LHTMLPageRequester
class LHTMLPageRequester : public LHTMLRequesterBase
{
    Q_OBJECT
public:
    LHTMLPageRequester(QObject *parent = NULL);
    virtual ~LHTMLPageRequester() {}

    void startRequest();
    QString title() const;

    inline QString htmlData() const {return html_data;}
    inline QString plainData() const {return plain_data;}
    inline int htmlDataSize() const {return html_data.length();}
    inline int plainDataSize() const {return plain_data.length();}
    inline bool badRequest() const {return bad_request;}


protected:
    QWebEnginePage *m_page; //объект для получения html страницы
    QString html_data;  //код полученной html страницы
    QString plain_data; //данные html страницы в строковом виде
    bool bad_request; //признак неуспешности последнего запроса

    void clearData();

protected slots:
    void slotFinished(bool);

signals:
    void signalProgress(int); //эмитится в процессе выполнения запроса, папаметр показывает прогресс выполнения
    void signalDataReady(); //эмитится когда завершено получения данных со страницы html_data и plain_data

private:
    void functorPlainData(const QString&);
    void functorHtmlData(const QString&);

};


 #endif //LHTML_PAGE_REQUESTER_H

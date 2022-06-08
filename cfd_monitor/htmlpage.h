#ifndef HTMLPAGE_H
#define HTMLPAGE_H

#include "basepage.h"

#include "ui_htmlpage.h"
#include <QWebEnginePage>
class LHTMLPageRequester;

/*
class HtmlWorker
{

};
*/



//HtmlPage
class HtmlPage : public BasePage, Ui::HtmlPage
{
    Q_OBJECT
public:
    HtmlPage(QWidget*);
    virtual ~HtmlPage() {}

    QString iconPath() const {return QString(":/icons/images/html.png");}
    QString caption() const {return QString("HTML");}

    void updatePage() {}
    void tryRequest(const QString&);

protected:
    LHTMLPageRequester *m_requester;

    void resetPage(const QString&);

protected slots:
    void slotDataReady(); //выполняется когда запрос полностью завершен, независимо от результата выполнения
    void slotProgress(int); //выполняется в процессе выполнения запроса
    void slotBreaked(int, int);

signals:
    void signalGetUrlByTicker(const QString&, QString&);

};


#endif //HTMLPAGE_H



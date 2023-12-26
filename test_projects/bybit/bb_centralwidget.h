#ifndef BB_CENTRALWIDGET_H
#define BB_CENTRALWIDGET_H

#include "lsimplewidget.h"



class QStackedWidget;
class QSettings;
class LHttpApiRequester;
struct BB_APIReqParams;
class QJsonObject;


//BB_CentralWIdget
class BB_CentralWidget : public LSimpleWidget
{    
    Q_OBJECT
public:
    BB_CentralWidget(QWidget*);
    virtual ~BB_CentralWidget() {}

    virtual void load(QSettings&);
    virtual void save(QSettings&);
    void setExpandLevel(int);
    bool requesterBuzy() const;
    void updateDataPage();


protected:
    LListWidgetBox      *w_list;
    QStackedWidget      *w_stack;
    LHttpApiRequester   *m_reqObj;

    void init();
    void clearStack();
    void createPages();
    void initReqObject();
    int pageCount() const;
    void prepareReq(const BB_APIReqParams&);

protected slots:
    void slotReqFinished(int);
    void slotPageChanged(int);

public slots:
    void slotSendReq(const BB_APIReqParams&);
    void slotEnableControls(bool);

signals:
    void signalEnableControls(bool);
    void signalJsonReply(int, const QJsonObject&);

private:
    QString hmacSha1(QByteArray, QByteArray) const;

};



#endif // BB_CENTRALWIDGET_H

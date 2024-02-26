#ifndef BB_BASEPAGE_H
#define BB_BASEPAGE_H

#include "lsimplewidget.h"

#include <QTime>

struct BB_APIReqParams;

//BB_BasePage
class BB_BasePage : public LSimpleWidget
{
    Q_OBJECT
public:
    BB_BasePage(QWidget*, int t, int user_type = 1);
    virtual ~BB_BasePage() {reset();}

    virtual void updateDataPage(bool force = false) = 0;  //выполняется когда эта страница активируется (всплывает наверх) в stacked_widget

protected:
    BB_APIReqParams     *m_reqData; //data for send request
    QTime                m_updateTime; //last update time

    virtual void reset();
    virtual void sendRequest(int limit = -1, QString name_extra = QString());
    virtual bool updateTimeOver(quint16 secs = 60, bool force = false);

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);

signals:
    void signalSendReq(const BB_APIReqParams*);

};


#endif // BB_BASEPAGE_H

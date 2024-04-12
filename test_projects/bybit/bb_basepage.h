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

    virtual void updateDataPage(bool forcibly = false) = 0;  //выполняется когда эта страница активируется (всплывает наверх) в stacked_widget

    inline void setMinUpdatingInterval(quint16 a) {m_minUpdatingInterval = a;}

protected:
    BB_APIReqParams     *m_reqData; //data for send request
    QTime                m_updateTime; //last update time
    quint16              m_minUpdatingInterval;

    virtual void reset();
    virtual void sendRequest(int limit = -1, QString name_extra = QString());
    virtual bool updateTimeOver(bool forcibly = false);
    virtual int minUpdatingInterval() const {return m_minUpdatingInterval;}

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);

signals:
    void signalSendReq(const BB_APIReqParams*);

};


#endif // BB_BASEPAGE_H

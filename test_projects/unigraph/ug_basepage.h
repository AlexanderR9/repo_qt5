#ifndef UG_BASEPAGE_H
#define UG_BASEPAGE_H

#include "lsimplewidget.h"

#include <QTime>

struct UG_APIReqParams;
class QTimer;


//UG_BasePage
class UG_BasePage : public LSimpleWidget
{
    Q_OBJECT
public:
    UG_BasePage(QWidget*, int t, int user_type = 1);
    virtual ~UG_BasePage() {reset();}

    virtual void updateDataPage(bool forcibly = false) = 0;  //выполняется когда эта страница активируется (всплывает наверх) в stacked_widget

    inline void setMinUpdatingInterval(quint16 a) {m_minUpdatingInterval = a;}

    virtual void startUpdating(quint16);
    virtual void stopUpdating();
    virtual bool updatingRunning() const;
    virtual void saveData() = 0;
    virtual void loadData() = 0;

protected:
    UG_APIReqParams     *m_reqData; //data for send request
    QTime                m_updateTime; //last update time
    quint16              m_minUpdatingInterval;
    QTimer               *m_timer; //for next request
    quint32              m_timerCounter;
    quint16              m_reqLimit; //каким пачками запрашивать данные


    virtual void reset();
    virtual void sendRequest(QString name_extra = QString());
    virtual bool updateTimeOver(bool forcibly = false);
    virtual int minUpdatingInterval() const {return m_minUpdatingInterval;}
    virtual void clearPage() = 0;
    virtual QString dataFile() const {return QString();}

public slots:
    virtual void slotJsonReply(int, const QJsonObject&) = 0;
    virtual void slotReqBuzyNow() = 0;

protected slots:
    virtual void slotTimer();

signals:
    void signalSendReq(const UG_APIReqParams*);
    void signalStopUpdating();
    void signalGetReqLimit(quint16&);

};


#endif // UG_BASEPAGE_H

#ifndef UG_CENTRALWIDGET_H
#define UG_CENTRALWIDGET_H

#include "lsimplewidget.h"



class QStackedWidget;
class QSettings;
class LHttpApiRequester;
struct UG_APIReqParams;
class QJsonObject;
class SubGraphReq;
class QLineEdit;
struct PoolFilterParams;


//UG_CentralWidget
class UG_CentralWidget : public LSimpleWidget
{    
    Q_OBJECT
public:
    UG_CentralWidget(QWidget*);
    virtual ~UG_CentralWidget() {if (m_graphReqObj) delete m_graphReqObj;}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    void setExpandLevel(int);
    void setUpdatingInterval(quint16);
    bool requesterBuzy() const;
    void updateDataPage();
    void updateDataPage(quint16);
    void setApiServer(QString);
    void setApiKeys(QString, QString);
    void setViewPrecision(quint8 p);
    bool updatingRunning() const;
    bool isBuzy() const;
    void setDelayAfterTX(quint16);


    void actAdd();
    void actRemove();
    void actStop();
    void actLoadData();
    void actSaveData();
    void actChart();

protected:
    LListWidgetBox      *w_list;
    QStackedWidget      *w_stack;
    LHttpApiRequester   *m_reqObj;
    SubGraphReq         *m_graphReqObj;

    void init();
    void clearStack();
    void createPages();
    void initReqObject();
    int pageCount() const;
    void sendQuery();
    void freeReq();
    //void prepareReq(const BB_APIReqParams*);

protected slots:
    void slotReqFinished(int);
    void slotPageChanged(int);
    void slotPageActivated(int);
    void slotGetReqLimit(quint16&);


public slots:
    void slotSendReq(const UG_APIReqParams*);
    void slotEnableControls(bool);
    void slotStopUpdating();

signals:
    void signalEnableControls(bool);
    void signalJsonReply(int, const QJsonObject&);
    void signalPageChanged(int);
    void signalGetFilterParams(quint16&, PoolFilterParams&);
    void signalBuzy();
    void signalChangeSubGraph(QString);

};



#endif // UG_CENTRALWIDGET_H

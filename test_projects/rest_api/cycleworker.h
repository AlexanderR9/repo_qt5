#ifndef CYCLEWORKER_H
#define CYCLEWORKER_H

#include "lsimpleobj.h"


class QTimer;

//CycleWorker
class CycleWorker : public LSimpleObject
{
    Q_OBJECT
public:
    enum CycleMode {cmCoupons = 211, cmDivs, cmHistory, cmPrices, cmNone = 0};
    struct CycleItem
    {
        CycleItem() {}
        CycleItem(const QString &s1, const QString &s2, const QString &s3) :type(s1), figi(s2), uid(s3) {}
        QString type; //bond or stock
        QString figi;
        QString uid;
    };

    CycleWorker(QObject *parent = NULL);
    virtual ~CycleWorker() {}

    void checkCycleMode(const QString&);
    void prepareCycleData(QStringList&);
    void breakCycle();
    void getNextCycleData(QStringList&);

    QString name() const {return QString("cycle_worker_obj");}
    inline bool cycleModeOn() const {return (m_mode > cmNone);}
    inline QString apiMetod() const {return m_apiMetod;}

protected:
    QTimer              *m_timer;
    int                  m_mode;
    QList<CycleItem>     m_cycleData;
    QString              m_apiMetod; //current real metod

    void reset();
    void trimDataByAPIMetod();

protected slots:
    void slotTimer();

signals:
    void signalFinished();
    void signalNextReq();
//    void signalGetCycleData(QStringList&);

};

#endif // CYCLEWORKER_H




#ifndef CYCLEWORKER_H
#define CYCLEWORKER_H

#include "lsimpleobj.h"


class QTimer;
class QJsonObject;
class QJsonArray;


//CycleWorker
class CycleWorker : public LSimpleObject
{
    Q_OBJECT
public:
    enum CycleMode {cmCoupons = 211, cmDivs, cmHistory, cmBondPrices, cmStockPrices, cmNone = 0};
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
    void handleReplyData(const QJsonObject&);
    void parseLastPrices(const QJsonObject&);


    QString name() const {return QString("cycle_worker_obj");}
    inline bool cycleModeOn() const {return (m_mode > cmNone);}
    inline QString apiMetod() const {return m_apiMetod;}

    static QString couponsFile(); //full path
    static QString divsFile();
    //static QString historyFile();

protected:
    QTimer              *m_timer;
    int                  m_mode;
    QList<CycleItem>     m_cycleData;
    QString              m_apiMetod; //current real metod
    QString              m_lastFigi;

    void reset();
    void trimDataByAPIMetod();

protected slots:
    void slotTimer();

private:
    void parseCoupons(const QJsonArray&);
    void parseDivs(const QJsonArray&);
    void parseCandles(const QJsonArray&);
    void parsePrices(const QJsonArray&);


signals:
    void signalFinished();
    void signalNextReq();
    void signalCyclePrice(const QString&, float);

};

#endif // CYCLEWORKER_H




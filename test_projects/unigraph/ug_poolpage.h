#ifndef UG_POOLPAGE_H
#define UG_POOLPAGE_H

#include "ug_basepage.h"
#include "ug_apistruct.h"

class QJsonObject;

// PoolFilterParams
struct PoolFilterParams
{
    PoolFilterParams() {reset();}
    double min_tvl; // TVL * e-6
    quint16 min_age;
    quint16 min_ratio;
    void reset() {min_tvl=0.05; min_age=120; min_ratio=10;}
};


//UG_PoolPage
class UG_PoolPage : public UG_BasePage
{
    Q_OBJECT
public:
    UG_PoolPage(QWidget*);
    virtual ~UG_PoolPage() {m_pools.clear();}

    QString iconPath() const {return QString(":/icons/images/clock.svg");}
    QString caption() const {return QString("Pools");}

    virtual void updateDataPage(bool forcibly = false);  //выполняется когда эта страница активируется (всплывает наверх) в stacked_widget
    virtual void startUpdating(quint16); //выполняется когда пользователь нажимает кнопку "Update"
    virtual void saveData();
    virtual void loadData();

protected:
    QList<UG_PoolInfo>       m_pools; //контейнер для хранения записей-пулов
    LSearchTableWidgetBox   *m_tableBox;
    //double                   m_minTVL;
    quint32                  m_skip;
    PoolFilterParams        m_filterParams; //критерии отсева пулов, при котором пул будет добавлятся в m_pools и таблицу

    void initTable();
    void updateTableData();
    virtual void clearPage();
    void prepareQuery();
    virtual QString dataFile() const;
    void parseReceivedJson(const QJsonArray&);
    void lightStablePools(); //подстветить строки в таблице для пулов у которых оба токена - стейблы

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    void slotReqBuzyNow();
    void slotSetTokensFromPage(QHash<QString, QString>&); //key - id token, value - ticker

protected slots:
    virtual void slotTimer();

signals:
    void signalGetFilterParams(quint16&, PoolFilterParams&);

};




#endif // UG_POOLPAGE_H



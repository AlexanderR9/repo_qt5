#ifndef UG_POOLPAGE_H
#define UG_POOLPAGE_H

#include "ug_basepage.h"
#include "ug_apistruct.h"

class QJsonObject;


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
    virtual void startUpdating(quint16);
    virtual void saveData();
    virtual void loadData();

protected:
    QList<UG_PoolInfo>       m_pools; //контейнер для хранения записей-пулов
    LSearchTableWidgetBox   *m_tableBox;
    quint16                  m_reqLimit; //каким пачками запрашивать пулы
    double                   m_minTVL; //минимальный tvl, при котором пул будет добавлятся в m_pools и таблицу
    quint32                  m_skip;

    void initTable();
    void updateTableData();
    virtual void clearPage();
    void prepareQuery();
    virtual QString dataFile() const;

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    void slotReqBuzyNow();
    void slotSetTokensFromPage(QHash<QString, QString>&); //key - id token, value - ticker

protected slots:
    virtual void slotTimer();

signals:
    void signalGetFilterParams(quint16&, double&);

};




#endif // UG_POOLPAGE_H



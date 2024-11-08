#ifndef UG_DAYSDATAPAGE_H
#define UG_DAYSDATAPAGE_H


#include "ug_basepage.h"
#include "ug_apistruct.h"

class QJsonObject;
class LChartWidget;


//UG_DaysDataPage
class UG_DaysDataPage : public UG_BasePage
{
    Q_OBJECT
public:
    UG_DaysDataPage(QWidget*);
    virtual ~UG_DaysDataPage() {}

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("Days data");}

    virtual void saveData() {}
    virtual void loadData();

    virtual void updateDataPage(bool forcibly = false);
    virtual void startUpdating(quint16);

protected:
    QList<UG_PoolDayData>       m_data; //контейнер для хранения записей

    LListWidgetBox      *m_poolListWidget;
    LTableWidgetBox     *m_historyTable;
    LChartWidget        *m_chart;

    virtual void clearPage();
    void initPage();
    void initTable();
    void updateTableData();
    QString curPoolID() const; //selected list item
    void prepareQuery();


public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    virtual void slotReqBuzyNow() {}

protected slots:
    virtual void slotTimer();

};




#endif // UG_DAYSDATAPAGE_H

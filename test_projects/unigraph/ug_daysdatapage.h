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
    quint32                     m_skip;

    LSearchListWidgetBox        *m_poolListWidget;
    LTableWidgetBox             *m_historyTable;
    LChartWidget                *m_chart;

    virtual void clearPage();
    void initPage();
    void initTable();
    void initChart();

    void updateTableData();
    QString curPoolID() const; //selected list item
    void prepareQuery();
    void loadChainPools(QString, QString);
    void trimData();
    void calcAvgFactorTH(quint16 n_days, int col);
    void repaintChart(const QList<int>&);
    void fillPoints(QList<QPointF>&, int);



public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    virtual void slotReqBuzyNow();

protected slots:
    virtual void slotTimer();
    void slotSelectionChanged();

private:
    QColor chartColor(int) const;

};




#endif // UG_DAYSDATAPAGE_H

#ifndef BB_CHARTPAGE_H
#define BB_CHARTPAGE_H

#include "bb_basepage.h"


class LChartWidget;
struct BB_APIReqParams;
class QJsonObject;
class QPointF;


//BB_ChartPage
class BB_ChartPage : public BB_BasePage
{
    Q_OBJECT
public:
    BB_ChartPage(QWidget*);
    virtual ~BB_ChartPage() {}

    QString iconPath() const {return QString(":/icons/images/chart.svg");}
    QString caption() const {return QString("Chart");}

    void updateDataPage(bool) {}

protected:
    LListWidgetBox  *w_listAll;
    LListWidgetBox  *w_listFavor;
    LChartWidget    *w_chart;

    void init();
    void loadTickers();
    void initChart();
    void repaintChart(const QList<QPointF>&);

protected slots:
    void slotTickerChanged(int);

public slots:
    void slotJsonReply(int, const QJsonObject&);

};

#endif // BB_CHARTPAGE_H

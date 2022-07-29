#ifndef CHARTPAGE_H
#define CHARTPAGE_H

#include "basepage.h"
#include "ui_chartpage.h"

#include <QMap>
#include <QDateTime>

class LSearch;
class LChartWidget;
class QListWidgetItem;




//ConfigPage
class ChartPage : public BasePage, Ui::ChartPage
{
    Q_OBJECT
public:
    ChartPage(QWidget*);
    virtual ~ChartPage() {}

    QString iconPath() const {return QString(":/icons/images/chart.svg");}
    QString caption() const {return QString("Chart");}

    void updatePage() {}
    void initSource();

protected:
    LSearch             *m_search;
    LChartWidget        *m_chart;

    void initSearch();
    void initChart();
    void searchExec();
    void sendLog(const QString&, int);
    void prepareChartPoints(const QString&, QList<QPointF>&);

signals:
    void signalGetSource(QStringList&);
    void signalGetChartData(const QString&, QMap<QDateTime, float>&);

protected slots:
    void slotRepaintChart();



};


#endif //CHARTPAGE_H



#ifndef BB_CHARTPAGE_H
#define BB_CHARTPAGE_H

#include "bb_basepage.h"


class LChartWidget;
struct BB_APIReqParams;
class QJsonObject;
class QPointF;
class LSearch;
class QLabel;
class QLineEdit;


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
    void addFavorToken();
    void removeFavorToken();
    QString selectedTicker() const;

protected:
    LListWidgetBox  *w_listAll;
    LListWidgetBox  *w_listFavor;
    LChartWidget    *w_chart;
    LSearch         *m_searchObj; //common
    LSearch         *m_searchObj_F; //favor
    QLineEdit       *m_searchEdit;
    QLabel          *m_searchLabel;
    QLabel          *m_searchLabel_F;

    void init();
    void initSearch();
    void loadTickers();
    void initChart();
    void repaintChart(const QList<QPointF>&);

    virtual void requestByTicker(QString);
    virtual void fillPoints(QList<QPointF>&, const QJsonArray&);

protected slots:
    void slotTickerChanged();

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);

};

//BB_FundRatePage
class BB_FundRatePage : public BB_ChartPage
{
    Q_OBJECT
public:
    BB_FundRatePage(QWidget*);
    virtual ~BB_FundRatePage() {}

    QString iconPath() const {return QString(":/icons/images/coin.svg");}
    QString caption() const {return QString("Fund rate");}

protected:
    QLineEdit       *m_endDateEdit;

    void reinitSearchObj();
    QDate endDate() const;

    virtual void requestByTicker(QString);
    virtual void fillPoints(QList<QPointF>&, const QJsonArray&);

};

#endif // BB_CHARTPAGE_H

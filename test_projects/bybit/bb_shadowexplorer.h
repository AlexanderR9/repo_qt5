#ifndef BB_SHADOWEXPLORER_H
#define BB_SHADOWEXPLORER_H


#include "bb_basepage.h"
#include "bb_apistruct.h"

class QTableWidgetItem;
class QJsonArray;
class QJsonObject;
class QLineEdit;
class QTimer;



//ShadowPrivotData
struct ShadowPrivotData
{
    ShadowPrivotData() {reset();}

    QList<BB_Bar> bars;
    float volatility_factor; //максимальный кеф изменения цены max/min
    float limit_shadow_size; //пользовательская настройка, %
    float limit_stoploss; //пользовательская настройка, %
    float testing_result; //total result (%) of testing
    float max_drawdown;
    quint16 n_ok;
    QList<float> candle_points; //last result candle points

    float minPrice() const;
    float maxPrice() const;
    float currentRsi() const;
    float averageOverShadow() const;
    float averageUnderShadow() const;
    float minShadowSize(bool over) const;
    float maxShadowSize(bool over) const;

    void reset();
    void addBar(const QJsonArray&);
    QString strTimeSpan() const;
    QString strPriceSpan() const;
    QString strMinMaxPrice() const;
    QString strAverageShadows() const;
    QString strMinMaxShadows(bool over) const;
    QString strHasLimitCandles(bool over) const;
    QStringList candleResult(int);
    QString strN_ok() const;
    QString strCandlePoints() const;

};


//BB_ShadowExplorer
class BB_ShadowExplorer : public BB_BasePage
{
    Q_OBJECT
public:
    BB_ShadowExplorer(QWidget*);
    virtual ~BB_ShadowExplorer() {}

    QString iconPath() const {return QString(":/icons/images/candle.png");}
    QString caption() const {return QString("Shadow explorer");}

    void updateDataPage(bool forcibly = false);
    void stopTimer();
    void loadTestingData();
    void showChart();

protected:
    LTableWidgetBox     *m_tickerTable;
    LTableWidgetBox     *m_pivotTable;
    LTableWidgetBox     *m_resultTable;
    QLineEdit           *m_testStateEdit;
    QTimer              *m_timer;
    ShadowPrivotData    m_pivotData; // for current selected ticker
    quint16             t_tick;

    void init();
    void initTickersTable();
    void initPivotTable();
    void initResultTable();

    void loadTickers();
    void resetPrivotData();
    QString selectedTicker() const;
    int selectedRow() const;
    void receivedData(const QJsonArray&);
    void updatePivotTable();
    void updateTickerRow();
    void updateResultTable();
    void prepareReq(QString&);
    void loadTestingData(const QString&, QTableWidget*);
    QList<QPointF> getPointsByTicker(const QString&, const QStringList&) const;

private:
    QString monitFile() const;
    QString dataPointsFile() const;
    void addLastToFile();
    void addLastPointsToFile(int row);

protected slots:
    void slotTickerChanged(QTableWidgetItem*);
    void slotTimer();

public slots:
    void slotJsonReply(int, const QJsonObject&);

signals:
    void signalGetLimitSize(float&);
    void signalGetStopLoss(float&);

};



#endif // BB_SHADOWEXPLORER_H

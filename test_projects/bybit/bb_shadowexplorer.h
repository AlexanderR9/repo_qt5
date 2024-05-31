#ifndef BB_SHADOWEXPLORER_H
#define BB_SHADOWEXPLORER_H


#include "bb_basepage.h"
#include "bb_apistruct.h"

class QTableWidgetItem;
class QJsonArray;
class QJsonObject;

//ShadowPrivotData
struct ShadowPrivotData
{
    ShadowPrivotData() {reset();}

    QList<BB_Bar> bars;
    float volatility_factor; //максимальный кеф изменения цены max/min

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

protected:
    LTableWidgetBox     *m_tickerTable;
    LTableWidgetBox     *m_pivotTable;
    ShadowPrivotData    m_pivotData; // for current selected ticker

    void init();
    void loadTickers();
    void resetPrivotData();
    QString selectedTicker() const;
    int selectedRow() const;
    void receivedData(const QJsonArray&);
    void updatePivotTable();
    void updateTickerRow();

protected slots:
    void slotTickerChanged(QTableWidgetItem*);

public slots:
    void slotJsonReply(int, const QJsonObject&);


};



#endif // BB_SHADOWEXPLORER_H

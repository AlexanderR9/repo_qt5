#ifndef PARAMSWIDGET_H
#define PARAMSWIDGET_H


#include <QWidget>
#include "ui_poolparams.h"


class QSettings;
struct PoolParamsStruct;
struct PoolParamsCalculated;
class LChartWidget;


//ParamsWidget
class ParamsWidget : public QWidget, public Ui::PoolParams
{
    Q_OBJECT
public:
    ParamsWidget(QWidget *parent = 0);
    virtual ~ParamsWidget() {}

    void getParams(PoolParamsStruct&);

    void save(QSettings&);
    void load(QSettings&);

protected:
    LChartWidget    *m_chart;

    void init();
    void normalizateValues();
    void normalizateValue(QLineEdit*);
    void initChart();
    void resetChart();
    void activateChart();
    void recalcPriceChanging(float, float);
    void updatePriceLabel(float);
    void addPoint(float, float);

private:
    float getTokenSize(const QLineEdit*) const;
    void updateTextColor(QWidget*, int);
    float labelPrice() const;

public slots:
    void slotCalcResult(const PoolParamsCalculated&);
    void slotChangePriceResult(float dx, float dy);

protected slots:
    void slotPriceChanged(int);

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);
    void signalPriceChanged(float); //new price

};

#endif // PARAMSWIDGET_H

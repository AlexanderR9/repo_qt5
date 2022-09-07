#ifndef FXCHART_WIDGET_H
#define FXCHART_WIDGET_H

#include "lsimplewidget.h"

class QTableWidget;
class QSettings;
class QLabel;
class QSlider;
class LChartWidget;
class FXBarContainer;

//FXChartSliderWidget
class FXChartSliderWidget : public QWidget
{
    Q_OBJECT
public:
    FXChartSliderWidget(QWidget*);
    virtual ~FXChartSliderWidget() {}

protected:
    QLabel  *m_labelBegin;
    QLabel  *m_labelEnd;
    QSlider *m_sliderBegin;
    QSlider *m_sliderEnd;

    void init();

};

//FXChartWidget
class FXChartWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    FXChartWidget(QWidget*);
    virtual ~FXChartWidget() {}

    void load(QSettings&);
    void save(QSettings&);

    virtual QString caption() const {return QString("Chart of data");} //некая надпись соответствующая этому виджету
    virtual QString iconPath() const {return QString(":/icons/images/chart.svg");} //некая иконка соответствующая этому виджету

    void repaintChart(const QList<const FXBarContainer*>&); //перерисовать графики для заданных контейнеров

protected:
    LChartWidget            *m_chart;
    QTableWidget            *m_legendTable;
    FXChartSliderWidget     *m_sliderWidget;

    void initChartWidget();
    void initTable(); //инициализация таблицы
    void initSliderWidget();

};



#endif //FXCHART_WIDGET_H


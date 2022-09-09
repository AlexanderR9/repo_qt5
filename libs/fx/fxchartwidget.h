#ifndef FXCHART_WIDGET_H
#define FXCHART_WIDGET_H

#include "lsimplewidget.h"

#include <QDateTime>


class QTableWidget;
class QSettings;
class QLabel;
class QSlider;
class LChartWidget;
class FXBarContainer;

//FXChartSettings
struct FXChartSettings
{
    FXChartSettings() {reset();}

    quint8 line_width;
    quint8 axis_presision;

    void reset() {line_width = axis_presision = 1;}
    void setSettings(const FXChartSettings &other) {line_width = other.line_width; axis_presision = other.axis_presision;}
    QString toStr() const {return QString("FXChartSettings: line_width=%1  presision=%2").arg(line_width).arg(axis_presision);}

};

//FXChartSliderWidget
class FXChartSliderWidget : public QWidget
{
    Q_OBJECT
public:
    FXChartSliderWidget(QWidget*);
    virtual ~FXChartSliderWidget() {}

    void resetState();
    void setLabelsText(const QDateTime&, const QDateTime&);

protected:
    QLabel  *m_labelBegin;
    QLabel  *m_labelEnd;
    QSlider *m_sliderBegin;
    QSlider *m_sliderEnd;

    void init();

protected slots:
    void slotRangeChanged(); //выполняется когда двигается один из слидеров

signals:
    void signalRangeChanged(quint8, quint8); //выдается диапазон текущего временного интервала в % (выполняется когда двигается один из слидеров)

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
    void resetState(); //сброс всех парамтров страницы и очистка графика
    void setChartSettings(const FXChartSettings&);

protected:
    LChartWidget            *m_chart;
    QTableWidget            *m_legendTable;
    FXChartSliderWidget     *m_sliderWidget;
    QDateTime                m_minTime; //минимальная координата Х среди всех графиков
    QDateTime                m_maxTime; //максимальная координата Х среди всех графиков
    FXChartSettings          m_chartSettings; //общие настройки графика

    void initChartWidget(); //инициализация виджета - графика
    void initTable(); //инициализация таблицы
    void initSliderWidget(); //инициализация виджета со слидерами для регулировки отрисовки временного интервала ползунками
    void updateTimeRange(const FXBarContainer*); //обновить временные границы(m_minTime, m_maxTime) графиков
    void addLegendRow(const FXBarContainer*); //добавить строку в таблицу легенды
    void updateChartSettings();

private:
    QColor nextColor() const; //вернет цвет линии для следующего графика

protected slots:
    void slotRangeChanged(quint8, quint8); //выполняется когда двигается один из слидеров
    void slotLegendRowChanged(); //выполняется при смене выделенной строки в таблице легенды


};



#endif //FXCHART_WIDGET_H


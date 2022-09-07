#include "fxchartwidget.h"
#include "ltable.h"
#include "fxenums.h"
#include "lchart.h"
#include "fxbarcontainer.h"

#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QSettings>
#include <QSplitter>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>


//FXChartWidget
FXChartWidget::FXChartWidget(QWidget *parent)
    :LSimpleWidget(parent, 32),
      m_chart(NULL),
      m_legendTable(NULL),
      m_sliderWidget(NULL)
{
    setObjectName("fx_chart_widget");

    initChartWidget();
    initTable();
    initSliderWidget();

}
void FXChartWidget::repaintChart(const QList<const FXBarContainer*> &list)
{
    qDebug()<<QString("FXChartWidget::repaintChart  data size %1").arg(list.count());

    m_chart->fullClear();
    if (list.isEmpty()) return;

    foreach (const FXBarContainer *v, list)
    {
        LChartParams p;
        p.lineColor = QColor(150, 100, 220);
        v->getChartPoints(p.points);
        m_chart->addChart(p);
    }

    m_chart->updateAxis();
}
void FXChartWidget::initSliderWidget()
{
    m_sliderWidget = new FXChartSliderWidget(this);
    h_splitter->addWidget(m_sliderWidget);
}
void FXChartWidget::initChartWidget()
{
    m_chart = new LChartWidget(this);
    v_splitter->addWidget(m_chart);
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    m_chart->fullClear();
}
void FXChartWidget::initTable()
{
    m_legendTable = new QTableWidget(this);
    v_splitter->addWidget(m_legendTable);
    LTable::fullClearTable(m_legendTable);

    QStringList headers;
    headers << "Couple" << "TF" <<  "Color" << "Points" << "Date interval";
    LTable::setTableHeaders(m_legendTable, headers);

    m_legendTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_legendTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    LTable::resizeTableContents(m_legendTable);
}
void FXChartWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
}
void FXChartWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
}


//FXChartSliderWidget
FXChartSliderWidget::FXChartSliderWidget(QWidget *parent)
    :QWidget(parent),
      m_labelBegin(NULL),
      m_labelEnd(NULL),
      m_sliderBegin(NULL),
      m_sliderEnd(NULL)
{
    setObjectName("fx_slider_widget");

    init();

}
void FXChartSliderWidget::init()
{
    m_labelBegin = new QLabel("Begin", this);
    m_labelEnd = new QLabel("End", this);
    m_sliderBegin = new QSlider(Qt::Vertical, this);
    m_sliderEnd = new QSlider(Qt::Vertical, this);

    if (layout()) delete layout();
    QGridLayout *lay = new QGridLayout(0);
    lay->setSpacing(2);
    setLayout(lay);

    lay->addWidget(m_labelBegin, 0, 0);
    lay->addWidget(m_labelEnd, 0, 1);
    lay->addWidget(m_sliderBegin, 1, 0);
    lay->addWidget(m_sliderEnd, 1, 1);

    /////////////////////////////////////
    m_sliderBegin->setMinimum(1);
    m_sliderBegin->setMaximum(100);
    m_sliderBegin->setSingleStep(1);
    m_sliderBegin->setValue(1);
    m_sliderBegin->setTickPosition(QSlider::TicksBothSides);

    m_sliderEnd->setMinimum(1);
    m_sliderEnd->setMaximum(100);
    m_sliderEnd->setSingleStep(1);
    m_sliderEnd->setValue(1);
    m_sliderEnd->setTickPosition(QSlider::TicksBothSides);
    m_sliderEnd->setInvertedAppearance(true);

}

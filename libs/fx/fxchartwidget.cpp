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

#define SLIDER_DATE_MASK        QString("MM.yy")
#define LEGEND_COLOR_COL        2


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
    resetState();
    updateChartSettings();
    if (list.isEmpty()) return;

    foreach (const FXBarContainer *v, list)
    {
        LChartParams p;
        p.lineColor = nextColor();
        p.pointsColor = p.lineColor;
        v->getChartPoints(p.points);
        m_chart->addChart(p);

        updateTimeRange(v);
        addLegendRow(v);
    }

    m_chart->updateAxis();
    m_sliderWidget->setLabelsText(m_minTime, m_maxTime);
    LTable::resizeTableContents(m_legendTable);

}
void FXChartWidget::setChartSettings(const FXChartSettings &chs)
{
    m_chartSettings.setSettings(chs);
    qDebug()<<QString("FXChartWidget::setChartSettings   %1").arg(m_chartSettings.toStr());
    updateChartSettings();
}
void FXChartWidget::addLegendRow(const FXBarContainer *data)
{
    QColor cur_color = nextColor();
    QStringList row_data;
    row_data << data->couple() << FXEnumStaticObj::strTimeFrame(data->timeframe()) << "color" << QString::number(data->barCount());
    row_data << QString("%1 / %2").arg(data->firstTime().daysTo(data->lastTime())/30).arg(data->firstTime().daysTo(data->lastTime()));
    LTable::addTableRow(m_legendTable, row_data);
    m_legendTable->item(m_legendTable->rowCount()-1, LEGEND_COLOR_COL)->setBackground(cur_color);
}
QColor FXChartWidget::nextColor() const
{
    switch (m_legendTable->rowCount())
    {
        case 0: return QColor(200, 10, 10);
        case 1: return QColor(150, 100, 200);
        case 2: return QColor(50, 200, 30);
        case 3: return QColor(0, 0, 220);
        case 4: return QColor(160, 40, 40);
        default: break;
    }
    return Qt::gray;
}
void FXChartWidget::resetState()
{
    m_chart->fullClear();
    m_sliderWidget->resetState();
    LTable::removeAllRowsTable(m_legendTable);
    m_minTime = QDateTime();
    m_maxTime = QDateTime();


}
void FXChartWidget::updateTimeRange(const FXBarContainer *data)
{
    if (!m_minTime.isValid()) m_minTime = data->firstTime();
    else if (data->firstTime() < m_minTime) m_minTime = data->firstTime();

    if (!m_maxTime.isValid()) m_maxTime = data->lastTime();
    else if (data->lastTime() > m_maxTime) m_maxTime = data->lastTime();
}
void FXChartWidget::initSliderWidget()
{
    m_sliderWidget = new FXChartSliderWidget(this);
    h_splitter->addWidget(m_sliderWidget);

    connect(m_sliderWidget, SIGNAL(signalRangeChanged(quint8, quint8)), this, SLOT(slotRangeChanged(quint8, quint8)));
}
void FXChartWidget::updateChartSettings()
{
    m_chart->setLineWidth(m_chartSettings.line_width);
    m_chart->setAxisPrecision(0, m_chartSettings.axis_presision);

}
void FXChartWidget::initChartWidget()
{
    m_chart = new LChartWidget(this);
    v_splitter->addWidget(m_chart);
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    m_chart->setLineWidth(2);
    m_chart->setAxisPrecision(0, 1);
    m_chart->setAxisOffsets(80, 40, -1);
    m_chart->setCrossXAxisTextViewMode(2);
    m_chart->setAxisTextOffsets(-1, 60);

    m_chart->fullClear();
}
void FXChartWidget::initTable()
{
    m_legendTable = new QTableWidget(this);
    v_splitter->addWidget(m_legendTable);
    LTable::fullClearTable(m_legendTable);

    QStringList headers;
    headers << "Couple" << "TF" <<  "Color" << "Points" << "Date interval, M/d";
    LTable::setTableHeaders(m_legendTable, headers);

    m_legendTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_legendTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    LTable::resizeTableContents(m_legendTable);

    connect(m_legendTable, SIGNAL(itemSelectionChanged()), this, SLOT(slotLegendRowChanged()));
}
void FXChartWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
}
void FXChartWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
}
void FXChartWidget::slotRangeChanged(quint8 v1, quint8 v2)
{
    if (m_chart->chartsCount() == 0) return;

    int days = m_minTime.daysTo(m_maxTime);
    QDateTime dt1 = m_minTime.addDays(int(double(days)*double(v1)/double(100)));
    QDateTime dt2 = m_minTime.addDays(int(double(days)*double(100 - v2 + 1)/double(100)));
    m_sliderWidget->setLabelsText(dt1, dt2);

    //rescale chart
    m_chart->rescaleBySliders(v1, (100 - v2 + 1));
}
void FXChartWidget::slotLegendRowChanged()
{
    qDebug("FXChartWidget::slotLegendRowChanged()");


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

    connect(m_sliderBegin, SIGNAL(valueChanged(int)), this, SLOT(slotRangeChanged()));
    connect(m_sliderEnd, SIGNAL(valueChanged(int)), this, SLOT(slotRangeChanged()));
}
void FXChartSliderWidget::resetState()
{
    m_labelBegin->setText("Begin");
    m_labelEnd->setText("End");
    m_sliderBegin->setValue(1);
    m_sliderEnd->setValue(1);

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
void FXChartSliderWidget::setLabelsText(const QDateTime &first, const QDateTime &last)
{
    if (first.isValid())
        m_labelBegin->setText(first.toString(SLIDER_DATE_MASK));

    if (last.isValid())
        m_labelEnd->setText(last.toString(SLIDER_DATE_MASK));

    QPalette palette;
    if (first.isValid() && last.isValid() && first >= last) palette.setBrush(QPalette::WindowText, Qt::red);
    else palette.setBrush(QPalette::WindowText, Qt::black);
    m_labelBegin->setPalette(palette);
    m_labelEnd->setPalette(palette);

}
void FXChartSliderWidget::slotRangeChanged()
{
    int v1 = m_sliderBegin->value();
    int v2 = m_sliderEnd->value();
    emit signalRangeChanged(v1, v2);
}








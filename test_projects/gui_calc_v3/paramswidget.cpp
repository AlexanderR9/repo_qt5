#include "paramswidget.h"
#include "poolstructs.h"
#include "calc_v3.h"
#include "lstring.h"
#include "lchart.h"

#include <QSettings>
#include <QDebug>

#define RESULT_PRECISION    3


//ParamsWidget
ParamsWidget::ParamsWidget(QWidget *parent)
    :QWidget(parent),
    m_chart(NULL)
{
    setupUi(this);
    setObjectName(QString("pool_params_widget"));

    init();
    initChart();

    connect(horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slotPriceChanged(int)));
}
void ParamsWidget::init()
{
    feeComboBox->clear();
    feeComboBox->addItem(LPoolCalcObj::captionByFeeType(pfs001));
    feeComboBox->addItem(LPoolCalcObj::captionByFeeType(pfs005));
    feeComboBox->addItem(LPoolCalcObj::captionByFeeType(pfs025));
    feeComboBox->addItem(LPoolCalcObj::captionByFeeType(pfs03));
    feeComboBox->addItem(LPoolCalcObj::captionByFeeType(pfs1));
}
void ParamsWidget::save(QSettings &settings)
{
    settings.setValue(QString("%1/minPrice").arg(objectName()), minPriceLineEdit->text());
    settings.setValue(QString("%1/maxPrice").arg(objectName()), maxPriceLineEdit->text());
    settings.setValue(QString("%1/curPrice").arg(objectName()), curPriceLineEdit->text());
    settings.setValue(QString("%1/size").arg(objectName()), sizeLineEdit->text());
    settings.setValue(QString("%1/input_token").arg(objectName()), inputTokenComboBox->currentIndex());
    settings.setValue(QString("%1/fee_size").arg(objectName()), feeComboBox->currentIndex());
}
void ParamsWidget::load(QSettings &settings)
{
    QString key = QString("%1/minPrice").arg(objectName());
    minPriceLineEdit->setText(settings.value(key, "0.1").toString());
    key = QString("%1/maxPrice").arg(objectName());
    maxPriceLineEdit->setText(settings.value(key, "1.1").toString());
    key = QString("%1/curPrice").arg(objectName());
    curPriceLineEdit->setText(settings.value(key, "0.5").toString());
    key = QString("%1/size").arg(objectName());
    sizeLineEdit->setText(settings.value(key, "500").toString());

    key = QString("%1/input_token").arg(objectName());
    inputTokenComboBox->setCurrentIndex(settings.value(key, 0).toInt());
    key = QString("%1/fee_size").arg(objectName());
    feeComboBox->setCurrentIndex(settings.value(key, 0).toInt());

}
void ParamsWidget::normalizateValues()
{
    normalizateValue(minPriceLineEdit);
    normalizateValue(maxPriceLineEdit);
    normalizateValue(curPriceLineEdit);
    //normalizateValue(dPriceLineEdit);
    normalizateValue(sizeLineEdit);
}
void ParamsWidget::normalizateValue(QLineEdit *le)
{
    if (le)
    {
        QString s = le->text().trimmed();
        s.replace(' ', QString());
        if (s.contains(',')) s.replace(',', '.');
        le->setText(s);
    }
}
void ParamsWidget::resetCalcParams()
{
    token0SizeLineEdit->setText("?");
    token1SizeLineEdit->setText("?");
    minTickLineEdit->setText("?");
    maxTickLineEdit->setText("?");
    liqLineEdit->setText("?");
    sumLineEdit->setText("?");
}
void ParamsWidget::updateParams(LPoolCalcObj *calc_obj)
{
    if (!calc_obj) return;

    GuiPoolResults p;
    calc_obj->getGuiParams(p);
    token0SizeLineEdit->setText(QString::number(p.token_sizes.first, 'f', RESULT_PRECISION));
    token1SizeLineEdit->setText(QString::number(p.token_sizes.second, 'f', RESULT_PRECISION));
    liqLineEdit->setText(QString::number(p.L, 'f', RESULT_PRECISION));
    minTickLineEdit->setText(QString::number(p.tick_range.first));
    maxTickLineEdit->setText(QString::number(p.tick_range.second));

    int fee_type = feeComboBox->currentIndex()+pfs001;
    this->binCountLineEdit->setText(QString::number(p.binCount(fee_type)));

    double sum = calc_obj->assetsSum(p.cur_price);
    sumLineEdit->setText(QString::number(sum, 'f', 1));

    minPriceLineEdit->setText(QString::number(p.price_range.first, 'f', calc_obj->pricePrecision()));
    maxPriceLineEdit->setText(QString::number(p.price_range.second, 'f', calc_obj->pricePrecision()));
    curPriceLineEdit->setText(QString::number(p.cur_price, 'f', calc_obj->pricePrecision()));

    horizontalSlider->setEnabled(true);
    activateChart();
}
void ParamsWidget::getParams(InputPoolParams &p)
{
    p.reset();
    normalizateValues();
    resetChart();

    bool ok;
    p.fee_type = feeComboBox->currentIndex()+pfs001;
    p.input_token = inputTokenComboBox->currentIndex();

    p.range.first = minPriceLineEdit->text().toDouble(&ok);
    if (!ok || p.range.first<=0)
    {
        emit signalError(QString("invalid min price value [%1]").arg(minPriceLineEdit->text()));
        return;
    }
    p.range.second = maxPriceLineEdit->text().toDouble(&ok);
    if (!ok || p.range.second<=0)
    {
        emit signalError(QString("invalid max price value [%1]").arg(maxPriceLineEdit->text()));
        return;
    }
    if (p.range.second <= p.range.first)
    {
        emit signalError(QString("invalid range prices [%1/%2]").arg(p.range.first).arg(p.range.second));
        return;
    }
    p.cur_price = curPriceLineEdit->text().toDouble(&ok);
    if (!ok || p.cur_price<=0)
    {
        emit signalError(QString("invalid cur price value [%1]").arg(curPriceLineEdit->text()));
        return;
    }

    p.input_size = sizeLineEdit->text().toDouble(&ok);
    if (!ok || p.input_size<0.1)
    {
        emit signalError(QString("invalid input size of token [%1]").arg(sizeLineEdit->text()));
        return;
    }

    if (p.cur_price < p.range.first && p.input_token == 1)
    {
        emit signalError(QString("invalid inputt token [%1], cur_price is under the range").arg(p.input_token));
        return;
    }
    if (p.cur_price > p.range.second && p.input_token == 0)
    {
        emit signalError(QString("invalid inputt token [%1], cur_price is over the range").arg(p.input_token));
        return;
    }
    p.validity = true;
}
void ParamsWidget::slotChangePriceResult(float dx, float dy)
{
    emit signalMsg(QString("AFTER CHANGE PRICE: dx=%1  dy=%2 ").arg(QString::number(dx, 'f', 3)).arg(QString::number(dy, 'f', 3)));

    recalcPriceChanging(dx, dy);
    updateTextColor(token0SizeLineEdit, ((dx < 0) ? -1 : 1));
    updateTextColor(token1SizeLineEdit, ((dy < 0) ? -1 : 1));
}
void ParamsWidget::recalcPriceChanging(float dx, float dy)
{
    float t0_size = getTokenSize(token0SizeLineEdit);
    float t1_size = getTokenSize(token1SizeLineEdit);
    float t0_now = t0_size+dx;
    float t1_now = t1_size+dy;

    QString s = QString("%1 => %2").arg(QString::number(t0_size, 'f', RESULT_PRECISION)).arg(QString::number(t0_now, 'f', RESULT_PRECISION));
    if (t0_size > 0) s = QString("%1 (%2%3%)").arg(s).arg((dx < 0) ? "-" : "+").arg(QString::number(qAbs(dx*100)/t0_size, 'f', 1));
    token0SizeLineEdit->setText(s);

    s = QString("%1 => %2").arg(QString::number(t1_size, 'f', RESULT_PRECISION)).arg(QString::number(t1_now, 'f', RESULT_PRECISION));
    if (t1_size > 0) s = QString("%1 (%2%3%)").arg(s).arg((dy < 0) ? "-" : "+").arg(QString::number(qAbs(dy*100)/t1_size, 'f', 1));
    token1SizeLineEdit->setText(s);

    //calc average swap price
    s = (dx < 0) ? "SELL_TOKEN0: " : "BUY_TOKEN0: ";
    s = QString("%1 %2").arg(s).arg(QString::number(qAbs(dy/dx), 'f', RESULT_PRECISION));
    swapLineEdit->setText(s);
    updateTextColor(swapLineEdit, ((dx < 0) ? -1 : 1));


    //calc assets sum
    float sum_size_start = getTokenSize(sumLineEdit);
    float sum = labelPrice()*t0_now + t1_now;
    dx = sum - sum_size_start;
    s = QString("%1 => %2").arg(QString::number(sum_size_start, 'f', RESULT_PRECISION)).arg(QString::number(sum, 'f', RESULT_PRECISION));
    s = QString("%1 (%2%3%)").arg(s).arg((dx < 0) ? "-" : "+").arg(QString::number(qAbs(dx*100)/sum_size_start, 'f', 1));
    sumLineEdit->setText(s);
    updateTextColor(sumLineEdit, ((dx < 0) ? -1 : 1));

    addPoint(t0_now, t1_now);
}
void ParamsWidget::addPoint(float x, float y)
{
    if (horizontalSlider->value() % 10 > 0) return;

    QList<QPointF> list;
    list << QPointF(x, y);
    m_chart->addChartPoints(list, 0);
    m_chart->updateAxis();
}
void ParamsWidget::updateTextColor(QWidget *w, int sign)
{
    QPalette plt;
    if (sign < 0)
    {
        plt.setColor(QPalette::Text, "#DC143C");
        plt.setColor(QPalette::Foreground, "#DC143C");
    }
    else if (sign > 0)
    {
        plt.setColor(QPalette::Foreground, "#0000CD");
        plt.setColor(QPalette::Text, "#0000CD");
    }
    else
    {
        plt.setColor(QPalette::Foreground, Qt::gray);
        plt.setColor(QPalette::Text, Qt::gray);
    }
    w->setPalette(plt);
}
float ParamsWidget::getTokenSize(const QLineEdit *le) const
{
    if (!le) return -1;
    QString s = le->text().trimmed();
    if (s.isEmpty()) return -1;
    int pos = s.indexOf("=>");
    if (pos > 0) s = s.left(pos).trimmed();
    return s.toFloat();
}
float ParamsWidget::labelPrice() const
{
    QString s = priceLabel->text().trimmed();
    if (s.isEmpty()) return -1;

    int pos = s.indexOf("(");
    if (pos < 0) return -1;
    return s.left(pos).trimmed().toFloat();
}
void ParamsWidget::initChart()
{
    priceLabel->setText(QString());
    m_chart = new LChartWidget(this);
    simulatorBox->layout()->addWidget(m_chart);

    m_chart->setCrossColor(QColor(150, 190, 150));
    m_chart->setAxisXType(LChartAxisParams::xvtSimple);
    m_chart->setCrossXAxisTextViewMode(LChartAxisParams::xvtSimple);
    m_chart->setAxisPrecision(2, 2);
    m_chart->setPointSize(4);
    m_chart->setOnlyPoints(true);

    resetChart();
}
void ParamsWidget::resetChart()
{
    m_chart->removeChart();
    m_chart->updateAxis();
    horizontalSlider->setEnabled(false);
}
void ParamsWidget::activateChart()
{
    disconnect(horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slotPriceChanged(int)));
    horizontalSlider->setEnabled(true);
    float min = minPriceLineEdit->text().toDouble();
    float max = maxPriceLineEdit->text().toDouble();
    float cur = curPriceLineEdit->text().toDouble();
    if (cur <= min) horizontalSlider->setValue(0);
    else if (cur >= max) horizontalSlider->setValue(1000);
    else horizontalSlider->setValue(1000*(cur-min)/(max-min));
    connect(horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slotPriceChanged(int)));

    float fp = float(horizontalSlider->value())/float(1000);
    updatePriceLabel(fp);

    //create line
    LChartParams p;
    p.lineColor = QColor(170, 50, 40);
    p.pointsColor = "#F4A460";
    p.points.clear();
    p.points.append(QPointF(getTokenSize(token0SizeLineEdit), getTokenSize(token1SizeLineEdit)));
    m_chart->addChart(p);
    m_chart->updateAxis();
}
void ParamsWidget::slotPriceChanged(int t)
{
    //qDebug()<<QString("slotPriceChanged t=%1").arg(t);
    float fp = float(t)/float(1000);
    updatePriceLabel(fp);

    float min = minPriceLineEdit->text().toDouble();
    float max = maxPriceLineEdit->text().toDouble();
    float a = min + (max-min)*fp;
    if (t == 0) a = min;
    else if (t == 1000) a = max;

    float p_start = curPriceLineEdit->text().toDouble();
    if (p_start < min) p_start = min;
    if (p_start > max) p_start = max;
    QString s = QString::number(a, 'f', 4);
    s = QString("%1 (%2%3%)").arg(s).arg((a < p_start) ? "-" : "+").arg(QString::number(qAbs((a-p_start)*100)/p_start, 'f', 1));;
    curPriceNowLineEdit->setText(s);

    qDebug()<<QString::number(a);
    emit signalPriceChanged(a);
}
void ParamsWidget::updatePriceLabel(float fp)
{
    float min = minPriceLineEdit->text().toDouble();
    float max = maxPriceLineEdit->text().toDouble();
    float cur = curPriceLineEdit->text().toDouble();
    float a = min + (max-min)*fp;

    priceLabel->setText(QString("%1 (%2%)").arg(QString::number(a, 'f', 4)).arg(QString::number(fp*100, 'f', 1)));
    updateTextColor(priceLabel, ((a < cur) ? -1 : 1));
}



#include "lpputpage.h"
#include "ltable.h"

#include <QSettings>
#include <QDebug>
#include <QTableWidget>
#include <QTableWidgetItem>

//#define RESULT_PRECISION    3

#define LEFT_FACTOR             1.07
#define NESTED_PRECISION        1
#define PRICE_PRECISION         1



//LpPutPage
LpPutPage::LpPutPage(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName(QString("lpput_page"));

    /*
    init();
    initChart();

    connect(horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slotPriceChanged(int)));
    */


    initTable();
}
void LpPutPage::initTable()
{
    LTable::fullClearTable(resultTable);
    resultTable->verticalHeader()->hide();

    QStringList headers;
    headers << "Step" << "Nested" << "Nested, %" << "Open P" << "Range" << "Deposited" <<
               "Range size" << "Left point" << "Loss" << "Loss, %" << "Range yield" << "Integrated yield";
    LTable::setTableHeaders(resultTable, headers);
    LTable::resizeTableContents(resultTable);

    /////////////////////////////////////////

    LTable::fullClearTable(putTable);
    putTable->verticalHeader()->hide();

    headers.clear();
    headers << "Expiration price" << "Put cost" << "Put result" << "LP reward" << "Total reward" << "LP loss" << "Total result";
    LTable::setTableHeaders(putTable, headers);
    LTable::resizeTableContents(putTable);
}
void LpPutPage::calc()
{
    average_yield = -1;

    LTable::removeAllRowsTable(resultTable);
    LTable::resizeTableContents(resultTable);
    LTable::removeAllRowsTable(putTable);
    LTable::resizeTableContents(putTable);

    QString err;
    checkParams(err);
    if (!err.isEmpty()) {qWarning()<<QString("WARNING: %1").arg(err); return;}

    qDebug("params ok");
    qDebug()<<m_params.toStr();

    step_data.clear();
    put_data.clear();

    calcLP();
    calcPut();
    updateTable();
    updatePutTable();

}
void LpPutPage::calcLP()
{
    for (int i=0; i<m_params.steps; i++)
    {
        StepState state;
        state.number = i+1;

        if (step_data.isEmpty())
        {
            state.avrP = m_params.P0;
            state.range_yield = m_params.yield1;
            state.range.first = state.avrP - m_params.r1; // left point
            state.range.second = state.avrP + m_params.r1; // right point
            state.deposited.first = m_params.e0;
            state.deposited.second = state.deposited.first*state.avrP*LEFT_FACTOR;
            state.nested_stables = state.deposited.first*state.avrP + state.deposited.second;
            //state.left_point_asset = state.deposited.first + state.deposited.second/state.buyLeftP();
            //state.loss_stable = state.nested_stables - state.left_point_asset*state.range.first;

        }
        else
        {
            state.avrP = step_data.last().range.first;
            state.range_yield = step_data.last().range_yield/1.5;
            float r_last = step_data.last().rangeSize()/2;
            float r = r_last*1.5;
            state.range.first = state.avrP - r; // left point
            state.range.second = state.avrP + r; // right point
            state.deposited.first = step_data.last().left_point_asset;
            state.deposited.second = state.deposited.first*state.avrP*LEFT_FACTOR;
            state.nested_stables += state.deposited.second;
        }

        state.left_point_asset = state.deposited.first + state.deposited.second/state.buyLeftP();
        state.loss_stable = state.left_point_asset*state.range.first - state.nested_stables;

        step_data.append(state);
    }

    // calc percent
    start_liq = step_data.last().nested_stables;
    average_yield = 0;
    for (int i=0; i<m_params.steps; i++)
    {
        step_data[i].nested_persent = float(100)*step_data.at(i).nested_stables/start_liq;
        step_data[i].loss_persent = float(100)*step_data.at(i).loss_stable/start_liq;

        float nf = step_data.at(i).nested_persent/float(100);
        step_data[i].integrated_yield = step_data.at(i).range_yield*nf + (1-nf)*m_params.yield_stable;

        average_yield += step_data.at(i).integrated_yield;
    }
    average_yield /= float(m_params.steps);
    qDebug()<<QString("INTEGRATED YIELD: %1").arg(floatToStr(average_yield));
}
void LpPutPage::calcPut()
{
    for (int i=0; i<m_params.expiration_points; i++)
    {
        ExpirationPointResult e_point;
        e_point.point_price = m_params.P0 - (i+1)*m_params.point_step;
        e_point.put_cost = m_params.put_price*m_params.put_lots;

        float a = 0;
        if (m_params.put_strike > e_point.point_price) a = m_params.put_strike - e_point.point_price;
        e_point.put_result = a*m_params.put_lots - e_point.put_cost;

        e_point.lp_reward = ((start_liq*average_yield/float(100))*m_params.expiration_month)/float(12);
        e_point.lp_loss = lpLossByPoint(e_point.point_price);
        put_data.append(e_point);
    }
}
void LpPutPage::updatePutTable()
{
    if (put_data.isEmpty()) return;

    QStringList row_data;
    for (int i=0; i<put_data.count(); i++)
    {
        //headers << "Expiration price" << "Put cost" << "Put result" << "LP reward" << "Total reward" << "LP loss" << "Total result";

        const ExpirationPointResult& point = put_data.at(i);
        row_data.clear();
        row_data << floatToStr(point.point_price);
        row_data << floatToStr(point.put_cost);
        row_data << floatToStr(point.put_result);
        row_data << floatToStr(point.lp_reward);
        row_data << floatToStr(point.totalReward());
        row_data << floatToStr(point.lp_loss);
        row_data << floatToStr(point.totalResult());

        LTable::addTableRow(putTable, row_data);
    }

    LTable::resizeTableContents(putTable);

}
float LpPutPage::lpLossByPoint(float point_price) const
{
    if (step_data.isEmpty()) return 0;

    float loss = 0;
    int i_step = 0;
    while (2>1)
    {
        if (i_step >= step_data.count()) {i_step = -1; break;}

        if (point_price < step_data.at(i_step).range.first)
        {
            loss += step_data.at(i_step).loss_stable;
        }
        else
        {
            break;
        }
        i_step++;
    }

    if (i_step < 0) return loss;

    const StepState& state = step_data.at(i_step);
    if (point_price < state.avrP)
    {
        float d_loss = state.loss_stable - loss;
        float dp = state.avrP - point_price;
        float pf = dp/(state.rangeSize()/2);
        loss += pf*d_loss;
    }
    else
    {
        float dp = point_price - state.avrP;
        float pf = dp/(state.rangeSize()/2);
        loss *= (1-pf);
    }

    return loss;
}
void LpPutPage::updateTable()
{
    if (step_data.isEmpty()) return;

    QStringList row_data;
    for (int i=0; i<m_params.steps; i++)
    {
        const StepState& state = step_data.at(i);
        row_data.clear();

        row_data << QString::number(state.number);
        row_data << QString::number(state.nested_stables, 'f', NESTED_PRECISION) << floatToStr(state.nested_persent);
        row_data << floatToStr(state.avrP);

        row_data << QString("[%1 : %2]").arg(floatToStr(state.range.first)).arg(floatToStr(state.range.second));
        row_data << QString("%1 / %2").arg(floatToStr(state.deposited.first)).arg(floatToStr(state.deposited.second));
        row_data << floatToStr(state.rangeSize());
        row_data << floatToStr(state.left_point_asset);
        row_data << floatToStr(state.loss_stable) << floatToStr(state.loss_persent);
        row_data << QString("%1 %").arg(QString::number(state.range_yield, 'f', 1));
        row_data << QString("%1 %").arg(QString::number(state.integrated_yield, 'f', 1));

        LTable::addTableRow(resultTable, row_data);
    }

    LTable::resizeTableContents(resultTable);
}
void LpPutPage::checkParams(QString &err)
{
    err.clear();
    m_params.expiration_month = expComboBox->currentText().toUInt();
    m_params.steps = stepsComboBox->currentText().toUInt();
    m_params.expiration_points = pointsComboBox->currentText().toUInt();


    bool ok;
    m_params.P0 = curPriceLineEdit->text().toFloat(&ok);
    if (!ok || m_params.P0 <= 0) {err = "P0 is invalid"; m_params.reset(); return;}
    m_params.r1 = r1LineEdit->text().toFloat(&ok);
    if (!ok || m_params.r1 <= 0) {err = "r1 is invalid"; m_params.reset(); return;}
    m_params.e0 = e0LineEdit->text().toFloat(&ok);
    if (!ok || m_params.e0 <= 0) {err = "e0 is invalid"; m_params.reset(); return;}
    m_params.yield1 = yield1lineEdit->text().toFloat(&ok);
    if (!ok || m_params.yield1 <= 0) {err = "yield1 is invalid"; m_params.reset(); return;}
    m_params.yield_stable = yieldStablelineEdit->text().toFloat(&ok);
    if (!ok || m_params.yield_stable <= 0) {err = "yield_stable is invalid"; m_params.reset(); return;}

    m_params.put_price = putPriceLineEdit->text().toFloat(&ok);
    if (!ok || m_params.put_price <= 0) {err = "put_price is invalid"; m_params.reset(); return;}
    m_params.put_strike = strikeLineEdit->text().toFloat(&ok);
    if (!ok || m_params.put_strike <= 0) {err = "put_strike is invalid"; m_params.reset(); return;}
    m_params.put_lots = lotLineEdit->text().toFloat(&ok);
    if (!ok || m_params.put_lots <= 0) {err = "put_lots is invalid"; m_params.reset(); return;}
    m_params.point_step = pointStepLineEdit->text().toFloat(&ok);
    if (!ok) {err = "point_step is invalid"; m_params.reset(); return;}


}
void LpPutPage::save(QSettings &settings)
{
    settings.setValue(QString("%1/stepsComboBox").arg(objectName()), stepsComboBox->currentIndex());
    settings.setValue(QString("%1/curPriceLineEdit").arg(objectName()), curPriceLineEdit->text());
    settings.setValue(QString("%1/r1LineEdit").arg(objectName()), r1LineEdit->text());
    settings.setValue(QString("%1/e0LineEdit").arg(objectName()), e0LineEdit->text());
    settings.setValue(QString("%1/yield1lineEdit").arg(objectName()), yield1lineEdit->text());
    settings.setValue(QString("%1/yieldStablelineEdit").arg(objectName()), yieldStablelineEdit->text());

    settings.setValue(QString("%1/putPriceLineEdit").arg(objectName()), putPriceLineEdit->text());
    settings.setValue(QString("%1/strikeLineEdit").arg(objectName()), strikeLineEdit->text());
    settings.setValue(QString("%1/lotLineEdit").arg(objectName()), lotLineEdit->text());
    settings.setValue(QString("%1/expComboBox").arg(objectName()), expComboBox->currentIndex());
    settings.setValue(QString("%1/pointsComboBox").arg(objectName()), pointsComboBox->currentIndex());
    settings.setValue(QString("%1/pointStepLineEdit").arg(objectName()), pointStepLineEdit->text());


}
void LpPutPage::load(QSettings &settings)
{
    QString key = QString("%1/stepsComboBox").arg(objectName());
    stepsComboBox->setCurrentIndex(settings.value(key, 2).toInt());
    key = QString("%1/curPriceLineEdit").arg(objectName());
    curPriceLineEdit->setText(settings.value(key, "1950").toString());
    key = QString("%1/r1LineEdit").arg(objectName());
    r1LineEdit->setText(settings.value(key, "200").toString());
    key = QString("%1/e0LineEdit").arg(objectName());
    e0LineEdit->setText(settings.value(key, "0.1").toString());
    key = QString("%1/yield1lineEdit").arg(objectName());
    yield1lineEdit->setText(settings.value(key, "125").toString());
    key = QString("%1/yieldStablelineEdit").arg(objectName());
    yieldStablelineEdit->setText(settings.value(key, "18.5").toString());

    key = QString("%1/putPriceLineEdit").arg(objectName());
    putPriceLineEdit->setText(settings.value(key, "25.3").toString());
    key = QString("%1/strikeLineEdit").arg(objectName());
    strikeLineEdit->setText(settings.value(key, "1000").toString());
    key = QString("%1/lotLineEdit").arg(objectName());
    lotLineEdit->setText(settings.value(key, "1.5").toString());
    key = QString("%1/expComboBox").arg(objectName());
    expComboBox->setCurrentIndex(settings.value(key, 2).toInt());
    key = QString("%1/pointsComboBox").arg(objectName());
    pointsComboBox->setCurrentIndex(settings.value(key, 2).toInt());
    key = QString("%1/pointStepLineEdit").arg(objectName());
    pointStepLineEdit->setText(settings.value(key, "50").toString());

}
QString LpPutPage::floatToStr(float x) const
{
    quint8 p = 1;
    if (qAbs(x) < 5) p = 4;
    else if (qAbs(x) < 10) p = 2;
    else if (qAbs(x) > 500) p = 0;
    return QString::number(x, 'f', p);
}


/////////////LpPutPage::CalcParams//////////////
void LpPutPage::CalcParams::reset()
{
    steps = expiration_month = 2;
    P0 = r1 = e0 = yield1 = yield_stable = -1;
    put_price = put_strike = put_lots = -1;

    expiration_points = 1;
    point_step = 0;

}
bool LpPutPage::CalcParams::invalid() const
{
    if (P0 <= 0 || r1 <= 0 || e0 <= 0 || yield1 <= 0 || yield_stable <= 0) return true;
    if (put_price <= 0 || put_strike <= 0 || put_lots <= 0) return true;
    return false;
}
QString LpPutPage::CalcParams::toStr() const
{
    QString s("CalcParams: ");
    s = QString("%1 P0=%2 r1=%3 e0=%4").arg(s).arg(P0).arg(r1).arg(e0);
    s = QString("%1 yield1=%2 yield_stable=%3 steps=%4").arg(s).arg(yield1).arg(yield_stable).arg(steps);
    s = QString("%1 put_price=%2 put_strike=%3 put_lots=%4").arg(s).arg(put_price).arg(put_strike).arg(put_lots);
    s = QString("%1 expiration_month=%2").arg(s).arg(expiration_month);
    s = QString("%1 expiration_points=%2 point_step=%3").arg(s).arg(expiration_points).arg(point_step);
    return s;
}



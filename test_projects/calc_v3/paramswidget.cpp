#include "paramswidget.h"
#include "tickobj.h"
#include "lstring.h"

#include <QSettings>


ParamsWidget::ParamsWidget(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName(QString("pool_params_widget"));

    init();
}
void ParamsWidget::init()
{
    feeComboBox->clear();
    feeComboBox->addItem(TickObj::captionByFeeType(tst001));
    feeComboBox->addItem(TickObj::captionByFeeType(tst005));
    feeComboBox->addItem(TickObj::captionByFeeType(tst03));
    feeComboBox->addItem(TickObj::captionByFeeType(tst1));

    //QDoubleValidator *dv = new QDoubleValidator(this);
    //minPriceLineEdit->setValidator(dv);
    //maxPriceLineEdit->setValidator(dv);
    //curPriceLineEdit->setValidator(dv);
    //sizeLineEdit->setValidator(dv);

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
void ParamsWidget::getParams(PoolParamsStruct &p)
{
    p.reset();
    normalizateValues();

    bool ok;
    p.fee_type = feeComboBox->currentIndex();
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
    if (!ok || p.input_size<10)
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
void ParamsWidget::slotCalcResult(const PoolParamsCalculated &p)
{
    quint8 prec = 3;
    token0SizeLineEdit->setText(QString::number(p.token0_size, 'f', prec));
    token1SizeLineEdit->setText(QString::number(p.token1_size, 'f', prec));
    minTickLineEdit->setText(QString::number(p.tick_range.first));
    maxTickLineEdit->setText(QString::number(p.tick_range.second));
    binCountLineEdit->setText(QString("%1,  cur_bin=%2").arg(QString::number(p.bin_count)).arg(p.cur_bin));


}




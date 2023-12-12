#include "apitradedialog.h"
#include "apicommonsettings.h"


#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>


//APITradeDialog
APITradeDialog::APITradeDialog(TradeOperationData &data, QWidget *parent)
    :LSimpleDialog(parent),
    m_data(data)
{
    setObjectName(QString("api_trade_dialog_%1").arg(data.order_type));
    init();
    addVerticalSpacer();
    resize(700, 400);
    updateTitle();

    if (m_data.invalid())
    {
        this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        updateWidgetsSizePolicy();
        slotRecalcResult();
    }
}
void APITradeDialog::updateWidgetsSizePolicy()
{
    int min_w = 200;
    foreach (const SimpleWidget &sw, m_widgets)
    {
        sw.label->setMinimumWidth(min_w);
        sw.label->setMaximumWidth(min_w);
        if (sw.edit) sw.edit->setReadOnly(true);
        if (sw.comboBox)
        {
            connect(sw.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRecalcResult()));
        }
    }
}
void APITradeDialog::init()
{
    QString key = "company";
    this->addSimpleWidget("Company", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.company);

    key = "ptype";
    this->addSimpleWidget("Paper type", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.paper_type);

    key = "price";
    this->addSimpleWidget("Current price", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, m_data.price);

    if (m_data.isBond())
    {
        key = "coupon";
        this->addSimpleWidget("Accumulated coupon", LSimpleDialog::sdtDoubleLine, key, 2);
        setWidgetValue(key, m_data.coupon);

        key = "fdate";
        this->addSimpleWidget("Finish date", LSimpleDialog::sdtString, key);
        setWidgetValue(key, m_data.finish_date);
    }

    key = "lots";
    this->addSimpleWidget(QString("Lot size (x %1ps)").arg(m_data.in_lot), LSimpleDialog::sdtIntCombo, key);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (sw)
    {
        for (int i=1; i<=5; i++) sw->comboBox->addItem(QString::number(i));
        for (int i=1; i<=5; i++) sw->comboBox->addItem(QString::number(i*10));        
        if (api_commonSettings.t_dialog.lots_index < 0) sw->comboBox->setCurrentIndex(1);
        else sw->comboBox->setCurrentIndex(api_commonSettings.t_dialog.lots_index);
    }

    key = "pstep";
    QString dev_caption("Price diviation");
    if (!m_data.isBond()) dev_caption = QString("%1, %").arg(dev_caption);
    this->addSimpleWidget(dev_caption, LSimpleDialog::sdtDoubleCombo, key);
    sw = this->widgetByKey(key);
    if (sw)
    {
        qint8 sign = 1;
        if (m_data.isBuyType()) sign = -1;
        for (int i=1; i<=20; i++) sw->comboBox->addItem(QString::number(float(sign*0.2*i), 'f', 1));
        for (int i=1; i<=15; i++) sw->comboBox->addItem(QString::number(float(sign*(4+2*i)), 'f', 1));
        if (api_commonSettings.t_dialog.deviation_index < 0) sw->comboBox->setCurrentIndex(10);
        else sw->comboBox->setCurrentIndex(api_commonSettings.t_dialog.deviation_index);
    }

    key = "result";
    this->addSimpleWidget("Operation price (1ps)", LSimpleDialog::sdtDoubleLine, key, precision());
    key = "total";
    this->addSimpleWidget("Total sum (need)", LSimpleDialog::sdtDoubleLine, key, precision());
}
void APITradeDialog::updateTitle()
{
    QString s("Trade operation: ");
    switch (m_data.order_type)
    {
        case totBuyLimit: {s.append("BUY_LIMIT"); break;}
        case totSellLimit: {s.append("SELL_LIMIT"); break;}
        case totSellStop: {s.append("SELL_STOP"); break;}
        default: {s.append("?"); break;}
    }
    setWindowTitle(s);
    setBoxTitle("Parameters");
    if (m_data.invalid()) return;

    //update price lineedit color
    if (m_data.isBond())
    {
        QPalette p_palette;
        if (m_data.price < 950)
        {
            p_palette.setColor(QPalette::Base, "#FFB6C1");
            widgetByKey("price")->edit->setPalette(p_palette);
        }
        else if (m_data.price > 1050)
        {
            p_palette.setColor(QPalette::Base, "#A52A2A");
            widgetByKey("price")->edit->setPalette(p_palette);
        }
    }

    //update paper type text
    QPalette t_palette;
    t_palette.setColor(QPalette::Text, m_data.isBond() ? Qt::darkGreen : Qt::blue);
    widgetByKey("ptype")->edit->setPalette(t_palette);
}
void APITradeDialog::slotRecalcResult()
{
    this->setWidgetValue("result", "-1");
    this->setWidgetValue("total", "-1");

    bool ok;
    bool ok2 = true;
    float dev = this->widgetValue("pstep").toFloat(&ok);
    if (!ok) ok2 = false;
    float cur_price = this->widgetValue("price").toFloat(&ok);
    if (!ok) ok2 = false;
    m_data.lots = this->widgetValue("lots").toUInt(&ok);
    if (!ok) ok2 = false;
    float coup = 0;
    if (m_data.isBond())
        coup = this->widgetValue("coupon").toFloat(&ok);
    if (!ok) ok2 = false;

    if (ok2)
    {
        if (m_data.isBond())
        {
            m_data.price = cur_price + dev;
        }
        else
        {
            float f_dev = 1 + dev/100;
            m_data.price = cur_price*f_dev;
        }
        trimPrice();

        float total = m_data.lots*m_data.in_lot*(m_data.price + coup);
        setWidgetValue("result", m_data.price);
        setWidgetValue("total", total);
        api_commonSettings.t_dialog.deviation_index = widgetByKey("pstep")->comboBox->currentIndex();
        api_commonSettings.t_dialog.lots_index = widgetByKey("lots")->comboBox->currentIndex();
    }
}
quint8 APITradeDialog::precision() const
{
    if (m_data.price > 5000) return 0;
    if (m_data.price > 10) return 1;
    if (m_data.price < 1) return 3;
    return 2;
}
void APITradeDialog::trimPrice()
{
    if (m_data.price > 10000) m_data.price = 10*qRound(m_data.price/10);
    if (m_data.price > 1500) m_data.price = qRound(m_data.price);
}


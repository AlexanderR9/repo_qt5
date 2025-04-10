#include "jstxdialog.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>


//TxApproveDialog
TxApproveDialog::TxApproveDialog(TxDialogData &data, QWidget *parent)
    :LSimpleDialog(parent),
    m_data(data)
{
    setObjectName(QString("api_approve_dialog"));
    resize(700, 300);

    init();
    updateTitle();
    addVerticalSpacer();

    /*
    updateKindWidget();

    if (m_data.invalid())
    {
        this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        updateWidgetsSizePolicy();
        slotRecalcResult();
    }
    */
}
void TxApproveDialog::updateTitle()
{
    QString s("TX operation: APPROVE");
    //setWindowTitle(QString("%1 %2").arg(s).arg(captionByOrderType(m_data.order_type)));
    setWindowTitle(s);
    //setWindowIcon(QIcon(iconByOrderType(m_data.order_type)));
    setBoxTitle("Parameters");
    //if (m_data.invalid()) return;
}
void TxApproveDialog::slotApply()
{
    m_data.dialog_params.clear();
    bool ok = true;
    QString key = "amount";
    float sum = this->widgetValue(key).toFloat(&ok);
    if (!ok || sum < 0.01) m_data.dialog_params.insert(key, QString("invalid"));
    else m_data.dialog_params.insert(key, QString::number(sum, 'f', 4));

    key = "whom";
    QString to_contract = this->widgetValue(key).toString().trimmed().toLower();
    m_data.dialog_params.insert(key, to_contract);

    LSimpleDialog::slotApply();
}
void TxApproveDialog::init()
{
    QString key = "whom";
    this->addSimpleWidget("Whom approve, contract", LSimpleDialog::sdtStringCombo, key);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;
    sw->comboBox->clear();
    sw->comboBox->addItem("SWAP_ROUTER");
    sw->comboBox->addItem("POS_MANAGER");


    key = "token";
    this->addSimpleWidget("Token address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);

    key = "amount";
    this->addSimpleWidget("Current price", LSimpleDialog::sdtDoubleLine, key, 2);
    this->setWidgetValue(key, "1.0");

}

/*
void APITradeDialog::fillLots()
{
    const SimpleWidget *sw = this->widgetByKey("lots");
    if (!sw) return;

    if (m_data.maxLot_ps > 0)
    {
        int max_steps = m_data.maxLot_ps/m_data.in_lot;
        int k = ((max_steps<20) ? 1 : 5);
        int l = 0;
        while (l < max_steps)
        {
            l += k;
            if (l >= max_steps)
            {
                l = max_steps;
                sw->comboBox->addItem(QString::number(l));
                break;
            }
            sw->comboBox->addItem(QString::number(l));
        }
    }
    else
    {
        for (int i=1; i<=10; i++) sw->comboBox->addItem(QString::number(i));
        for (int i=3; i<=6; i++) sw->comboBox->addItem(QString::number(i*5));
        for (int i=4; i<=10; i++) sw->comboBox->addItem(QString::number(i*10));
        if (api_commonSettings.t_dialog.lots_index < 0 || api_commonSettings.t_dialog.lots_index >= sw->comboBox->count())
            sw->comboBox->setCurrentIndex(1);
        else sw->comboBox->setCurrentIndex(api_commonSettings.t_dialog.lots_index);
    }
}
void APITradeDialog::fillDeviations()
{
    const SimpleWidget *sw = this->widgetByKey("pstep");
    if (!sw) return;

    qint8 sign = 1;
    if (m_data.downKind()) sign = -1;
    for (int i=1; i<=20; i++) sw->comboBox->addItem(QString::number(float(sign*0.2*i), 'f', 1));
    for (int i=1; i<=20; i++) sw->comboBox->addItem(QString::number(float(sign*(4+0.5*i)), 'f', 1));
    for (int i=0; i<=2; i++) sw->comboBox->addItem(QString::number(float(sign*(50+20*i)), 'f', 1));
    if (api_commonSettings.t_dialog.deviation_index < 0) sw->comboBox->setCurrentIndex(10);
    else sw->comboBox->setCurrentIndex(api_commonSettings.t_dialog.deviation_index);
}
QString APITradeDialog::captionByOrderType(int t)
{
    switch (t)
    {
        case totBuyLimit: return QString("BUY_LIMIT");
        case totSellLimit: return QString("SELL_LIMIT");
        case totTakeProfit: return QString("TAKE_PROFIT");
        case totStopLoss: return QString("STOP_LOSS");
        default: break;
    }
    return "UNKNOWN_ORDER_TYPE";
}
QString APITradeDialog::iconByOrderType(int t)
{
    switch (t)
    {
        case totBuyLimit: return QString(":/icons/images/ball_green.svg");
        case totSellLimit: return QString(":/icons/images/ball_red.svg");
        case totTakeProfit: return QString(":/icons/images/up.svg");
        case totStopLoss: return QString(":/icons/images/down.svg");
        default: break;
    }
    return "???";
}
void APITradeDialog::updateKindWidget()
{
    const SimpleWidget *sw = this->widgetByKey("kind");
    if (!sw) return;

    QString color = "#006400";
    if (m_data.redKind()) color = "#800000";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));
    sw->edit->addAction(QIcon(iconByOrderType(m_data.order_type)), QLineEdit::LeadingPosition);

}
void APITradeDialog::updateTitle()
{
    QString s("Trade operation: ");
    setWindowTitle(QString("%1 %2").arg(s).arg(captionByOrderType(m_data.order_type)));
    setWindowIcon(QIcon(iconByOrderType(m_data.order_type)));
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
    else if (m_data.price > 1500) m_data.price = qRound(m_data.price);
    else if (m_data.price > 10)  m_data.price = float(qRound(m_data.price*10))/float(10);
}
*/



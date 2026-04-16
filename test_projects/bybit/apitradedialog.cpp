#include "apitradedialog.h"
//#include "apicommonsettings.h"


#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateTime>


//APITradeDialog
APITradeDialog::APITradeDialog(TradeOperationData &data, QWidget *parent)
    :LSimpleDialog(parent),
    m_data(data)
{
    setObjectName(QString("api_trade_dialog_%1").arg(data.order_type));
    init();
    addVerticalSpacer();
    resize(600, 500);
    updateTitle();
    updateKindWidget();
    updateToStrikeWidget();

    /*
    if (m_data.invalid())
    {
        this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        //
        //slotRecalcResult();
    }
    */

    updateWidgetsSizePolicy();


}
void APITradeDialog::updateWidgetsSizePolicy()
{
    foreach (const SimpleWidget &sw, m_widgets)
    {
        //sw.label->setMinimumWidth(min_w);
        //sw.label->setMaximumWidth(min_w);
        if (sw.edit) sw.edit->setReadOnly(true);
        if (sw.comboBox)
        {
            sw.comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }
    }

    this->setCaptionsWidth(230);
}

void APITradeDialog::init()
{
    QString key = "kind";
    this->addSimpleWidget("Order kind", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, captionByOrderType(m_data.order_type));

    key = "ticker";
    this->addSimpleWidget("Ticker", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.ticker);

    key = "type";
    this->addSimpleWidget("Direction type", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.type);

    key = "price";
    this->addSimpleWidget("Current asset price", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, m_data.asset_price);
    this->addLineSeparator();

    key = "strike";
    this->addSimpleWidget("Strike price", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, m_data.strike);

    key = "to_strike";
    this->addSimpleWidget("To strike left", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, "-");

    key = "award";
    this->addSimpleWidget("Current award", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, m_data.award);

    key = "expirate";
    this->addSimpleWidget("Expiration", LSimpleDialog::sdtString, key);
    QDateTime dt = QDateTime::fromSecsSinceEpoch(m_data.expirate);
    float d_to = QDateTime::currentDateTime().secsTo(dt);
    d_to /= (3600*24);
    QString s_exp = QString("%1 (%2d)").arg(dt.date().toString("dd.MM.yyyy")).arg(QString::number(d_to, 'f', 2));
    this->setWidgetValue(key, s_exp);

    this->addLineSeparator();



    key = "lots";
    this->addSimpleWidget(QString("Lot size (x 1_ETH)"), LSimpleDialog::sdtDoubleCombo, key);

    key = "deviation";
    this->addSimpleWidget(QString("Award deviation, %"), LSimpleDialog::sdtIntCombo, key);

    key = "require_award";
    this->addSimpleWidget("Require award of oprtion", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, 0.05);


    fillLots();
}
void APITradeDialog::fillLots()
{
    const SimpleWidget *sw = this->widgetByKey("lots");
    float start_value = 0.1;
    float step = 0.05;

    for (int i=0; i<=20; i++)
        sw->comboBox->addItem(QString::number(start_value + i*step));

    // fill deviation
    int sign = 1;
    if (m_data.isBuy()) sign = -1;

    sw = this->widgetByKey("deviation");
    for (int i=1; i<=10; i++)
        sw->comboBox->addItem(QString::number(sign*i));

    for (int i=2; i<=9; i++)
        sw->comboBox->addItem(QString::number(sign*i*10));


    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRecalcAwardByDeviation()));

    sw->comboBox->setCurrentIndex(4);

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
void APITradeDialog::updateToStrikeWidget()
{
    QString key("to_strike");
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;

    float d = m_data.strike - m_data.asset_price;
    if (!m_data.isCall()) d *= (-1);

    this->setWidgetValue(key, d);
    if (d < 0)
    {
        sw->edit->setStyleSheet(QString("QLineEdit {background-color: #F0E68C;}"));
    }
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

    //update paper type text
    QPalette t_palette;
    t_palette.setColor(QPalette::Text, m_data.isCall() ? Qt::darkGreen : Qt::darkRed);
    widgetByKey("type")->edit->setPalette(t_palette);

    //set require_award text color
    widgetByKey("require_award")->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg("#0000CD"));
}
void APITradeDialog::slotRecalcAwardByDeviation()
{
    qDebug("APITradeDialog::slotRecalcAwardByDeviation()");

    bool ok;
    float dev = this->widgetValue("deviation").toFloat(&ok);
    if (!ok) {qWarning("APITradeDialog::slotRecalcAwardByDeviation() WARNING - deviation invalid value"); return;}

    float market_award = this->widgetValue("award").toFloat(&ok);
    if (!ok) {qWarning("APITradeDialog::slotRecalcAwardByDeviation() WARNING - market_award invalid value"); return;}

    float require_award = (dev/float(100) + 1)*market_award;
    setWidgetValue("require_award", QString::number(require_award, 'f', precision()));

}
quint8 APITradeDialog::precision() const
{
    /*
    if (m_data.price > 5000) return 0;
    if (m_data.price > 10) return 1;
    if (m_data.price < 1) return 3;
    */
    return 2;
}

/*
void APITradeDialog::trimPrice()
{
    if (m_data.asset_price > 10000) m_data.asset_price = 10*qRound(m_data.asset_price/10);
    else if (m_data.asset_price > 1500) m_data.asset_price = qRound(m_data.asset_price);
    else if (m_data.asset_price > 10)  m_data.asset_price = float(qRound(m_data.asset_price*10))/float(10);
}
*/


void APITradeDialog::slotApply()
{
    m_data.lot_size = widgetValue("lots").toFloat();
    m_data.award = widgetValue("require_award").toFloat();

    LSimpleDialog::slotApply();
}






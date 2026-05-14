#include "apitradedialog.h"
//#include "apicommonsettings.h"


#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateTime>
#include <QtMath>


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
    this->addSimpleWidget(QString("Award deviation, %"), LSimpleDialog::sdtDoubleCombo, key);

    key = "require_award";
    this->addSimpleWidget("Require award of oprtion", LSimpleDialog::sdtDoubleLine, key, precision());
    this->setWidgetValue(key, 0.05);

    key = "custom_id";
    this->addSimpleWidget("Custom order_ID", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, genCustomId());
    m_data.custom_id.clear();


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

    for (int i=1; i<=5; i++)
        sw->comboBox->addItem(QString::number(-1*sign*i));

    for (int i=0; i<=10; i++)
        sw->comboBox->addItem(QString::number(sign*i));

    for (int i=2; i<=9; i++)
        sw->comboBox->addItem(QString::number(sign*i*10));


    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRecalcAwardByDeviation()));

    sw->comboBox->setCurrentIndex(6);
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
    int r = int(qFloor(require_award*10));
    qDebug()<<QString("require_award=%1  r=%2").arg(require_award).arg(r);

    require_award = float(r)/float(10);
    if (require_award < 0.1) require_award = 0.1;

    setWidgetValue("require_award", QString::number(require_award, 'f', 4));

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


void APITradeDialog::slotApply()
{
    m_data.lot_size = widgetValue("lots").toFloat();
    m_data.award = widgetValue("require_award").toFloat();
    m_data.custom_id = widgetValue("custom_id").toString().trimmed();

    LSimpleDialog::slotApply();
}
QString APITradeDialog::genCustomId() const
{
    QString s_time = QTime::currentTime().toString("hhmmss");
    QString kind = m_data.isBuy() ? "buy" :  "sell";
    return QString("%1_%2_%3").arg(kind).arg(m_data.type.trimmed().toLower()).arg(s_time);
}





//APILinearTradeDialog
APILinearTradeDialog::APILinearTradeDialog(TradeOperationData &data, QWidget *parent)
    :APITradeDialog(data, parent)
{
    setObjectName(QString("api_linear_trade_dialog_%1").arg(data.order_type));

    /*
    init();
    addVerticalSpacer();
    resize(600, 500);
    updateTitle();
    updateKindWidget();
    updateToStrikeWidget();
    updateWidgetsSizePolicy();
    */


    reinitWidgeys();
    slotRecalcAwardByDeviation();
}
void APILinearTradeDialog::reinitWidgeys()
{
    removeAllLineSeparators();
    removeSimpleWidget("type");
    removeSimpleWidget("strike");
    removeSimpleWidget("to_strike");
    removeSimpleWidget("award");
    removeSimpleWidget("expirate");
    removeSimpleWidget("custom_id");


    const SimpleWidget *sw = this->widgetByKey("require_award");
    sw->label->setText("Limit price");
    sw = this->widgetByKey("deviation");
    sw->label->setText("Price deviation, %");
    sw = this->widgetByKey("require_award");
    sw->edit->setReadOnly(false);

    fillLots();
}
void APILinearTradeDialog::fillLots()
{
    const SimpleWidget *sw = this->widgetByKey("lots");
    sw->comboBox->clear();

    float step = 0.01;
    for (int i=1; i<=30; i++)
        sw->comboBox->addItem(QString::number(i*step));

    sw = this->widgetByKey("deviation");
    for (int i=0; i<=sw->comboBox->count(); i++)
    {
        float v = sw->comboBox->itemText(i).toFloat();
        v /= 10;
        sw->comboBox->setItemText(i, QString::number(v, 'f', 1));
    }

}
void APILinearTradeDialog::slotApply()
{
    m_data.custom_id.clear();
    m_data.lot_size = widgetValue("lots").toFloat();
    m_data.award = widgetValue("require_award").toFloat();
    //m_data.custom_id = widgetValue("custom_id").toString().trimmed();

    LSimpleDialog::slotApply();
}
void APILinearTradeDialog::slotRecalcAwardByDeviation()
{
    qDebug("APILinearTradeDialog::slotRecalcLimitPriceByDeviation()");

    bool ok;
    float dev = this->widgetValue("deviation").toFloat(&ok);
    if (!ok) {qWarning("APILinearTradeDialog::slotRecalcAwardByDeviation() WARNING - deviation invalid value"); return;}

    float cur_price = this->widgetValue("price").toFloat(&ok);
    if (!ok) {qWarning("APILinearTradeDialog::slotRecalcAwardByDeviation() WARNING - cur_price invalid value"); return;}

    float limit_price = (dev/float(100) + 1)*cur_price;
    //int r = int(qFloor(limit_price*10));
    qDebug()<<QString("deviation=%1  cur_price=%2  limit_price=%3").arg(dev).arg(cur_price).arg(limit_price);

    //limit_price = float(r)/float(10);
    //if (limit_price < 0.1) limit_price = 0.1;

    setWidgetValue("require_award", QString::number(limit_price, 'f', 2));

}


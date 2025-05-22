#include "jstxdialog.h"
#include "subcommonsettings.h"
#include "lstring.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QCheckBox>
#include <QIcon>
#include <QComboBox>
#include <QGridLayout>
#include <QMessageBox>


//TxDialogBase
TxDialogBase::TxDialogBase(TxDialogData &data, QWidget *parent)
    :LSimpleDialog(parent),
    m_data(data)
{
    setObjectName(QString("tx_base_dialog"));
    resize(700, 300);

    QString key = "tx_kind";
    this->addSimpleWidget("TX kind", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;
    sw->edit->setReadOnly(true);
    sw->edit->addAction(QIcon(iconByTXType(m_data.tx_kind)), QLineEdit::LeadingPosition);
    //addAction(QIcon(iconByOrderType(m_data.order_type)), QLineEdit::LeadingPosition);

    QString color = "#006400";
    //if (m_data.redKind()) color = "#800000";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));
    sw->edit->setText(captionByTXType(m_data.tx_kind));

    updateTitle();
}
QString TxDialogBase::iconByTXType(int tt)
{
    QString path = SubGraph_CommonSettings::commonIconsPath();
    QString icon_file;
    switch (tt)
    {
        case txWrap: {icon_file = "edit-redo.svg"; break;}
        case txUnwrap: {icon_file = "edit-undo.svg"; break;}
        case txApprove: {icon_file = "left.svg"; break;}
        case txTransfer: {icon_file = "send_msg.png"; break;}
        case txSwap: {icon_file = "swap.png"; break;}
        case txMint: {icon_file = "exchange.png"; break;}
        default: return QString();
    }
    return QString("%1%2%3").arg(path).arg(QDir::separator()).arg(icon_file);
}
QString TxDialogBase::captionByTXType(int tt)
{
    switch (tt)
    {
        case txWrap: return QString("WRAP");
        case txUnwrap: return QString("UNWRAP");
        case txApprove: return QString("APPROVE");
        case txTransfer: return QString("TRANSFER");
        case txSwap: return QString("SWAP");
        case txMint: return QString("MINT_POSITION");
        default: break;
    }
    return QString("???");
}
void TxDialogBase::updateTitle()
{
    setWindowTitle(QString("TX operation: %1").arg(captionByTXType(m_data.tx_kind)));
    setWindowIcon(QIcon(TxDialogBase::iconByTXType(m_data.tx_kind)));
    setBoxTitle("Parameters");
}



///////////////////TxApproveDialog////////////////////////////
TxApproveDialog::TxApproveDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_approve_dialog"));
    resize(700, 400);

    if (m_data.invalid()) return;

    init();
    addVerticalSpacer();

    this->setCaptionsWidth(220);
    setExpandWidgets();
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
    sw = this->widgetByKey(key);
    if (!sw) return;
    sw->edit->setReadOnly(true);
    QString color = "#800000";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "amount";
    this->addSimpleWidget("Supply amount", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.0");
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



///////////////////TxWrapDialog////////////////////////////
TxWrapDialog::TxWrapDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_wrap_dialog"));
    resize(500, 300);

    if (m_data.invalid()) return;

    init();
    addVerticalSpacer();

    this->setCaptionsWidth(220);
    setExpandWidgets();
}
void TxWrapDialog::init()
{
    QString key = "token";
    this->addSimpleWidget("Token name", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_name);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "amount";
    QString s = "Amount of wraping token";
    if (m_data.tx_kind == txUnwrap)  s = "Amount of unwraping token";
    this->addSimpleWidget(s, LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.0");
}
void TxWrapDialog::slotApply()
{
    m_data.dialog_params.clear();
    bool ok = true;
    QString key = "amount";
    float sum = this->widgetValue(key).toFloat(&ok);
    if (!ok || sum < 0.01) m_data.dialog_params.insert(key, QString("invalid"));
    else m_data.dialog_params.insert(key, QString::number(sum, 'f', 4));

    LSimpleDialog::slotApply();
}



///////////////////TxTransferDialog////////////////////////////
TxTransferDialog::TxTransferDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_transfer_dialog"));
    resize(700, 300);

    if (m_data.invalid()) return;

    init();
    addVerticalSpacer();

    this->setCaptionsWidth(220);
    setExpandWidgets();
}
void TxTransferDialog::init()
{
    QString key = "token";
    this->addSimpleWidget("Transfering token", LSimpleDialog::sdtString, key);
    QString t_value = (m_data.token_addr.isEmpty() ? m_data.token_name : m_data.token_addr);
    this->setWidgetValue(key, t_value);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "amount";
    this->addSimpleWidget("Amount of token", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.0");

    key = "to_wallet";
    this->addSimpleWidget("TO wallet address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, "0x");
}
void TxTransferDialog::slotApply()
{
    m_data.dialog_params.clear();
    bool ok = true;
    QString key = "amount";
    float sum = this->widgetValue(key).toFloat(&ok);
    if (!ok || sum < 0.01) m_data.dialog_params.insert(key, QString("invalid"));
    else m_data.dialog_params.insert(key, QString::number(sum, 'f', 4));

    key = "to_wallet";
    QString w_addr(this->widgetValue(key).toString().trimmed());
    if (w_addr.length() > 20) m_data.dialog_params.insert(key, w_addr);

    key = "token";
    m_data.dialog_params.insert(key, widgetValue(key).toString().trimmed());

    LSimpleDialog::slotApply();
}




///////////////////TxSwapDialog////////////////////////////
TxSwapDialog::TxSwapDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_swap_dialog"));
    resize(700, 300);

    if (m_data.invalid()) return;

    init();
    addVerticalSpacer();

    this->setCaptionsWidth(220);
    setExpandWidgets();
}
void TxSwapDialog::init()
{

    QString key = "desc";
    this->addSimpleWidget("Pool parameters", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_name);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;
    sw->edit->setReadOnly(true);

    key = "addr";
    this->addSimpleWidget("Pool address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    sw = this->widgetByKey(key);
    if (!sw) return;
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));


    key = "input_token";
    this->addSimpleWidget("Input token of pair", LSimpleDialog::sdtStringCombo, key);
    sw = this->widgetByKey(key);
    if (!sw) return;
    sw->comboBox->addItem("TOKEN_0");
    sw->comboBox->addItem("TOKEN_1");

    key = "amount";
    QString s = "Amount of change token";
    this->addSimpleWidget(s, LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.0");

    key = "is_simulate";
    this->addSimpleWidget("Simulate mode", LSimpleDialog::sdtBool, key);
    sw = this->widgetByKey(key);
    if (!sw) return;
    sw->checkBox->setChecked(true);

    key = "dead_line";
    this->addSimpleWidget("Dead line, secs", LSimpleDialog::sdtIntCombo, key);
    sw = this->widgetByKey(key);
    if (!sw) return;
    for (int i=1; i<15; i++)
        sw->comboBox->addItem(QString::number(i*20));
    sw->comboBox->setCurrentIndex(3);

}
void TxSwapDialog::slotApply()
{

    m_data.dialog_params.clear();
    bool ok = true;
    QString key = "amount";
    float sum = this->widgetValue(key).toFloat(&ok);
    if (!ok || sum < 0.01) m_data.dialog_params.insert(key, QString("invalid"));
    else m_data.dialog_params.insert(key, QString::number(sum, 'f', 4));

    key = "dead_line";
    int dead_line = this->widgetValue(key).toInt(&ok);
    if (!ok || dead_line < 10) m_data.dialog_params.insert(key, QString("invalid"));
    else m_data.dialog_params.insert(key, QString::number(dead_line));


    bool simulate_mode = true;
    key = "is_simulate";
    const SimpleWidget *sw = this->widgetByKey(key);
    if (sw) simulate_mode = sw->checkBox->isChecked();
    m_data.dialog_params.insert(key, (simulate_mode ? "yes" : "no"));

    int t_input = 0;
    key = "input_token";
    sw = this->widgetByKey(key);
    if (sw) t_input = sw->comboBox->currentIndex();
    m_data.dialog_params.insert(key, QString::number(t_input));

    LSimpleDialog::slotApply();
}




///////////////////TxMintPositionDialog////////////////////////////
TxMintPositionDialog::TxMintPositionDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent),
      m_mintParamsBox(NULL)
{
    setObjectName(QString("tx_mint_dialog"));
    resize(950, 400);

    if (m_data.invalid()) return;

    init();

    addVerticalSpacer();
    this->setCaptionsWidth(160);
    setExpandWidgets();
}
void TxMintPositionDialog::replaceToGridLayout()
{
    qDebug("TxMintPositionDialog::replaceToGridLayout()");
    this->gridLayout->removeWidget(groupBox);

    m_mintParamsBox = new QGroupBox("TX settings", this);
    if (m_mintParamsBox->layout()) delete m_mintParamsBox->layout();
    QVBoxLayout *mint_lay = new QVBoxLayout(0);
    mint_lay->setSpacing(4);
    mint_lay->setContentsMargins(10, 10, 10, 10);
    m_mintParamsBox->setLayout(mint_lay);

    this->gridLayout->addWidget(groupBox, 0, 0, 1, 1);
    this->gridLayout->addWidget(m_mintParamsBox, 0, 1, 1, 1);
    m_mintParamsBox->setMaximumWidth(320);

    //replace sub_widget to mintParamsBox
    for (int i=0; i<10; i++)
    {
        int lay_items = groupBox->layout()->count();
        QWidget *w_lay = groupBox->layout()->itemAt(lay_items-1)->widget();
        groupBox->layout()->removeWidget(w_lay);
        mint_lay->addWidget(w_lay);
    }

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mint_lay->addItem(verticalSpacer);

}
void TxMintPositionDialog::initStaticFields()
{
    QString key = "desc";
    this->addSimpleWidget("Pool parameters", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_name);
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "addr";
    this->addSimpleWidget("Pool address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "tvl";
    this->addSimpleWidget("Pool TVL", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("tvl"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "state";
    this->addSimpleWidget("Pool state", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("state"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "approved";
    this->addSimpleWidget("Approved assets", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("approved"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
}
void TxMintPositionDialog::initMintFields()
{
    QString key = "dead_line";
    this->addSimpleWidget("Dead line, secs", LSimpleDialog::sdtIntCombo, key);
    const SimpleWidget *sw = this->widgetByKey(key);
    for (int i=1; i<15; i++)
        sw->comboBox->addItem(QString::number(i*20));
    sw->comboBox->setCurrentIndex(3);

    key = "is_simulate";
    this->addSimpleWidget("Simulate mode", LSimpleDialog::sdtBool, key);
    sw = this->widgetByKey(key);
    sw->checkBox->setChecked(true);


    QString color = "#DD4500";
    key = "amount1";
    this->addSimpleWidget("Token_1 amount", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "-1");
    sw = this->widgetByKey(key);
    //sw->edit->setStyleSheet(QString("QLineEdit {color: %1; font-size: 14px; font-weight: bold;}").arg(color));
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1; font-weight: bold;}").arg(color));
    connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
    //--------------------------------
    key = "amount0";
    this->addSimpleWidget("Token_0 amount", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.00");
    sw = this->widgetByKey(key);
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1; font-weight: bold;}").arg(color));
    connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));


    key = "tick2";
    this->addSimpleWidget("Upper tick", LSimpleDialog::sdtIntLine, key);
    this->setWidgetValue(key, "0");
    key = "tick1";
    this->addSimpleWidget("Lower tick", LSimpleDialog::sdtIntLine, key);
    this->setWidgetValue(key, "0");


    key = "token_index";
    this->addSimpleWidget("Price token index", LSimpleDialog::sdtIntCombo, key);
    sw = this->widgetByKey(key);
    sw->comboBox->addItem(QString::number(0));
    sw->comboBox->addItem(QString::number(1));

    key = "price2";
    this->addSimpleWidget("Upper price", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.00");
    key = "price1";
    this->addSimpleWidget("Lower price", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.00");

    key = "range";
    this->addSimpleWidget("Units of range", LSimpleDialog::sdtStringCombo, key);
    sw = this->widgetByKey(key);
    sw->comboBox->addItem("price");
    sw->comboBox->addItem("tick");
    connect(sw->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRangeTypeChanged(int)));

}
void TxMintPositionDialog::init()
{
    initStaticFields();
    initMintFields();

    replaceToGridLayout();
    slotRangeTypeChanged(0);
    parseNoteText();
}
//bool TxMintPositionDialog::mintParamsValidity() const
void TxMintPositionDialog::checkMintParamsValidity(QString &err)
{
    err.clear();
    bool ok;
    QString range_type = this->widgetValue("range").toString().trimmed();
    m_data.dialog_params.insert("range_view", range_type);
    if (range_type == "tick")
    {
        int t1 = this->widgetValue("tick1").toInt(&ok);
        if (!ok) {err = "tick1 value is invalid"; return;}
        int t2 = this->widgetValue("tick2").toInt(&ok);
        if (!ok) {err = "tick2 value is invalid"; return;}
        if (t2 <= t1) {err = "ticks range is invalid, must (tick2 > tick1)"; return;}

        m_data.dialog_params.insert("l_tick", QString::number(t1));
        m_data.dialog_params.insert("h_tick", QString::number(t2));
    }
    else
    {
        int t_index = this->widgetValue("token_index").toInt(&ok);
        if (!ok || t_index<0 || t_index>1) {err = "token_index value is invalid"; return;}
        float p1 = this->widgetValue("price1").toFloat(&ok);
        if (!ok || p1 <= 0) {err = "price1 value is invalid"; return;}
        float p2 = this->widgetValue("price2").toFloat(&ok);
        if (!ok || p2 <= 0) {err = "price2 value is invalid"; return;}
        if (p2 <= p1) {err = "price range is invalid, must (price2 > price1)"; return;}

        m_data.dialog_params.insert("token_index", QString::number(t_index));
        m_data.dialog_params.insert("l_price", QString::number(p1));
        m_data.dialog_params.insert("h_price", QString::number(p2));
    }

    //check amounts
    float amount0 = this->widgetValue("amount0").toFloat(&ok);
    if (!ok) {err = "amount0 value is invalid"; return;}
    float amount1 = this->widgetValue("amount1").toFloat(&ok);
    if (!ok) {err = "amount1 value is invalid"; return;}
    if (amount1 > 0 && amount0 > 0) {err = "one of amounts must -1 value"; return;}
    if (amount1 <= 0 && amount0 <= 0) {err = "one of amounts must > 0"; return;}
    if (amount0 > 0)
    {
        m_data.dialog_params.insert("token0_amount", QString::number(amount0));
        m_data.dialog_params.insert("token1_amount", QString::number(-1));
    }
    else
    {
        m_data.dialog_params.insert("token1_amount", QString::number(amount1));
        m_data.dialog_params.insert("token0_amount", QString::number(-1));
    }
}
void TxMintPositionDialog::slotApply()
{
    m_data.dialog_params.clear();

    QString err;
    checkMintParamsValidity(err);
    if (!err.isEmpty())
    {
        QMessageBox::warning(this, "Error", err);
        m_data.dialog_params.clear();
        return;
    }

    QString simulate_mode = (widgetValue("is_simulate").toBool() ? "yes" : "no");
    m_data.dialog_params.insert("simulate_mode", simulate_mode);
    m_data.dialog_params.insert("dead_line", widgetValue("dead_line").toString().trimmed());
    m_data.dialog_params.insert("pool_address", m_data.token_addr);

    LSimpleDialog::slotApply();
}
void TxMintPositionDialog::slotRangeTypeChanged(int index)
{
    qDebug()<<QString("TxMintPositionDialog::slotRangeTypeChanged  index %1").arg(index);
    if (index == 0)
    {
        for (int i=4; i<=5; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->hide();
        for (int i=1; i<=3; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->show();
    }
    else
    {
        for (int i=4; i<=5; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->show();
        for (int i=1; i<=3; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->hide();
    }
}
void TxMintPositionDialog::slotAmountsChanged()
{
    QString sender_name = sender()->objectName();
    qDebug()<<QString("TxMintPositionDialog  sender[%1]").arg(sender()->objectName());
    const SimpleWidget *sw = NULL;
    if (sender_name.contains("amount0"))
    {
        sw = this->widgetByKey("amount1");
    }
    else if (sender_name.contains("amount1"))
    {
        sw = this->widgetByKey("amount0");
    }
    else
    {
        qWarning()<<QString("TxMintPositionDialog::slotAmountsChanged() WARNING invalid sender name [%1]").arg(sender_name);
        return;
    }

    if (sw)
    {
        disconnect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
        sw->edit->setText(QString::number(-1));
        connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
    }
    else qWarning()<<QString("TxMintPositionDialog::slotAmountsChanged() WARNING SW is NULL");
}
void TxMintPositionDialog::parseNoteText()
{
    qDebug("TxMintPositionDialog::parseNoteText()");
    QString s_note = m_data.dialog_params.value("note", QString()).trimmed().toLower();
    qDebug()<<s_note;
    s_note = LString::removeSpaces(s_note);
  //  qDebug()<<s_note;

    int pos = s_note.indexOf("amount");
    if (pos >= 0)
    {
       // qDebug()<<QString("find amount attr, pos=%1").arg(pos);
        QString s_amounts = LString::strTrimLeft(s_note, pos);
        s_amounts = LString::strBetweenStr(s_amounts, "(", ")");
        QStringList amount_list = LString::trimSplitList(s_amounts, "/");
        if (amount_list.count() == 2)
        {
            //const SimpleWidget *sw0 = this->widgetByKey("amount0");
            //const SimpleWidget *sw1 = this->widgetByKey("amount1");
            //disconnect(sw0->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
            //disconnect(sw1->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));

            if (amount_list.at(0).trimmed().toFloat() > 0)
            {
                setWidgetValue("amount0", amount_list.at(0).trimmed());
            }
            else
            {
                setWidgetValue("amount1", amount_list.at(1).trimmed());
            }


            //connect(sw0->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
            //connect(sw1->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
        }
    }

    pos = s_note.indexOf("range");
    if (pos >= 0)
    {
       // qDebug()<<QString("find range attr, pos=%1").arg(pos);
        QString s_range = LString::strTrimLeft(s_note, pos);
        s_range = LString::strBetweenStr(s_range, "[", "]");
        QStringList tick_list = LString::trimSplitList(s_range, ";");
        if (tick_list.count() == 2)
        {
            setWidgetValue("tick1", tick_list.at(0).trimmed());
            setWidgetValue("tick2", tick_list.at(1).trimmed());

            const SimpleWidget *sw = this->widgetByKey("range");
            sw->comboBox->setCurrentIndex(1);
        }
    }
}





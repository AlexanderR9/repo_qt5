#include "jstxdialog.h"
#include "subcommonsettings.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QCheckBox>
#include <QIcon>
#include <QComboBox>


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








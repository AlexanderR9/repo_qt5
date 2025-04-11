#include "jstxdialog.h"
#include "subcommonsettings.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDir>
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









#include "txdialog.h"
#include "appcommonsettings.h"
#include "lstring.h"
#include "deficonfig.h"
#include "nodejsbridge.h"

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

#define ERROR_KEY       QString("error")


//TxDialogData
bool TxDialogData::invalid() const
{
    return (tx_kind < txWrap || tx_kind > txBurn);
}


//////////////////////////DIALOGS/////////////////////////////////////

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
    QString path = AppCommonSettings::commonIconsPath();
    QString icon_file;
    switch (tt)
    {
        case txWrap: {icon_file = "edit-redo.svg"; break;}
        case txUnwrap: {icon_file = "edit-undo.svg"; break;}
        case txApprove: {icon_file = "left.svg"; break;}
        case txTransfer: {icon_file = "send_msg.png"; break;}
        case txSwap: {icon_file = "swap.png"; break;}
        case txMint: {icon_file = "exchange.png"; break;}
        case txIncrease: {icon_file = "list-add.svg"; break;}
        case txDecrease: {icon_file = "list-remove.svg"; break;}
        case txCollect: {icon_file = "crypto/usdc.png"; break;}
        case txBurn: {icon_file = "process-stop.svg"; break;}
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
        case txIncrease: return QString("INCREASE_LIQ");
        case txDecrease: return QString("DECREASE_LIQ");
        case txCollect: return QString("COLLECT_TOKENS");
        case txBurn: return QString("BURN_POSITION");
        default: break;
    }
    return QString("???");
}
void TxDialogBase::updateTitle()
{
    setWindowTitle(QString("TX operation: %1  (%2)").arg(captionByTXType(m_data.tx_kind)).arg(m_data.chain_name.toUpper()));
    setWindowIcon(QIcon(TxDialogBase::iconByTXType(m_data.tx_kind)));
    setBoxTitle("Parameters");
}
QString TxDialogBase::findTokenName() const
{
    int cid = defi_config.getChainID(m_data.chain_name);
    if (cid < 0) return "?";

    if (m_data.token_addr == AppCommonSettings::nativeTokenAddress())
        return defi_config.nativeTokenName(m_data.chain_name);

    foreach (const DefiToken &v, defi_config.tokens)
        if (v.chain_id == cid && v.address == m_data.token_addr) return v.name;
    return "?";
}
void TxDialogBase::addErrorField(QString err_text)
{
    QString key = "err";
    this->addSimpleWidget("Error", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, err_text);
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QString color = "#FF0000";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));
    sw->label->setStyleSheet(QString("QLabel {color: %1;}").arg(color));

    this->buttonBox->setEnabled(false);
}
void TxDialogBase::addSimulateField()
{
    QString key = "is_simulate";
    this->addSimpleWidget("Simulate mode", LSimpleDialog::sdtBool, key);
    const SimpleWidget *sw = this->widgetByKey(key);
    if (!sw) return;
    sw->checkBox->setChecked(true);
}
void TxDialogBase::slotApply()
{
    m_data.dialog_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(m_data.tx_kind));
    bool sim = widgetValue("is_simulate").toBool();
    m_data.dialog_params.insert(AppCommonSettings::nodejsTxSimulateFieldName(), (sim ? "yes" : "no"));
    LSimpleDialog::slotApply();
}


///////////////////TxWrapDialog////////////////////////////
TxWrapDialog::TxWrapDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_wrap_dialog"));
    resize(700, 300);

    if (m_data.invalid()) return;

    init();
    addVerticalSpacer();

    this->setCaptionsWidth(220);
    setExpandWidgets();
}
void TxWrapDialog::checkFields(QString &err)
{
    err.clear();
    switch (m_data.tx_kind)
    {
        case txWrap:
        {
            if (m_data.token_addr != AppCommonSettings::nativeTokenAddress())
                err = QString("TX only native token (%1)").arg(defi_config.nativeTokenName(m_data.chain_name));
            break;
        }
        case txUnwrap:
        {
            QString native_token = defi_config.nativeTokenName(m_data.chain_name);
            QString cur_token = widgetValue("token_name").toString().trimmed();
            if (QString("W%1").arg(native_token) != cur_token)
                err = QString("TX only wraped native token (W%1)").arg(native_token);
            break;
        }
        default:
        {
            err = QString("invalid operation %1 for this dialog").arg(m_data.tx_kind);
            break;
        }
    }
}
void TxWrapDialog::init()
{
    QString key = "token_addr";
    this->addSimpleWidget("Token address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "token_name";
    this->addSimpleWidget("Token name", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, findTokenName());
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    sw->edit->setStyleSheet(QString("QLineEdit {color: #5F9EA0;}"));

    key = "balance";
    this->addSimpleWidget("Current balance", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("balance", "-1"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    QString err;
    checkFields(err);
    if (!err.isEmpty()) {addErrorField(err); return;}

    addLineSeparator(2, "#2F4F4F");


    key = "amount";
    QString s = "Amount of wraping token";
    if (m_data.tx_kind == txUnwrap)  s = "Amount of unwraping token";
    this->addSimpleWidget(s, LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.0");

    addSimulateField();
}
void TxWrapDialog::slotApply()
{
    m_data.dialog_params.clear();
    bool ok = true;
    QString key = "amount";
    float sum = this->widgetValue(key).toFloat(&ok);
    if (!ok)
        m_data.dialog_params.insert(ERROR_KEY, QString("invalid amount value (%1)").arg(widgetValue(key).toString()));
    if (sum < 0.01 || sum > 10000)
        m_data.dialog_params.insert(ERROR_KEY, QString("amount value out of range (%1) [sum < 0.01 || sum > 10000]").arg(widgetValue(key).toString()));
    if (sum >= widgetValue("balance").toFloat())
        m_data.dialog_params.insert(ERROR_KEY, QString("amount value over balance (%1)").arg(widgetValue(key).toString()));

    m_data.dialog_params.insert(key, QString::number(sum, 'f', 4));

    TxDialogBase::slotApply();
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
    QString key = "token_name";
    this->addSimpleWidget("Transfering token name", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, findTokenName());
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    sw->edit->setStyleSheet(QString("QLineEdit {color: #5F9EA0;}"));

    key = "token_addr";
    this->addSimpleWidget("Token address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "balance";
    this->addSimpleWidget("Current balance", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("balance", "-1"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    addLineSeparator(2, "#2F4F4F");

    key = "amount";
    QString s = "Amount of wraping token";
    if (m_data.tx_kind == txUnwrap)  s = "Amount of unwraping token";
    this->addSimpleWidget(s, LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.0");

    key = "to_wallet";
    this->addSimpleWidget("TO wallet address", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, "0x");

    addSimulateField();

}
void TxTransferDialog::slotApply()
{
    m_data.dialog_params.clear();
    bool ok = true;

    //check amount
    QString key = "amount";
    float sum = this->widgetValue(key).toFloat(&ok);
    if (!ok)
        m_data.dialog_params.insert(ERROR_KEY, QString("invalid amount value (%1)").arg(widgetValue(key).toString()));
    if (sum < 0.01 || sum > 10000)
        m_data.dialog_params.insert(ERROR_KEY, QString("amount value out of range (%1) [sum < 0.01 || sum > 10000]").arg(widgetValue(key).toString()));
    if (sum >= widgetValue("balance").toFloat())
        m_data.dialog_params.insert(ERROR_KEY, QString("amount value over balance (%1)").arg(widgetValue(key).toString()));
    m_data.dialog_params.insert(key, QString::number(sum, 'f', 4));

    //check walet
    key = "to_wallet";
    QString w_addr(this->widgetValue(key).toString().trimmed());
    if (w_addr.length() > 20) m_data.dialog_params.insert(key, w_addr);
    else m_data.dialog_params.insert(ERROR_KEY, QString("target wallet address incorrect (%1)").arg(w_addr));

    m_data.dialog_params.insert("token_address", widgetValue("token_addr").toString().trimmed());
    TxDialogBase::slotApply();
}




/*

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


///////////////////TxRemoveLiqDialog////////////////////////////
TxRemoveLiqDialog::TxRemoveLiqDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_remove_liq_dialog"));
    resize(600, 400);

    if (m_data.invalid()) return;

    init();
    transformByTxKind();
    addVerticalSpacer();


    this->setCaptionsWidth(180);
    setExpandWidgets();
}
void TxRemoveLiqDialog::transformByTxKind()
{
    if (m_data.tx_kind == txCollect)
    {
        groupBox->layout()->itemAt(4)->widget()->hide();
        groupBox->layout()->itemAt(5)->widget()->hide();

    }
    else if (m_data.tx_kind == txDestroy)
    {
        groupBox->layout()->itemAt(3)->widget()->hide();
        groupBox->layout()->itemAt(5)->widget()->hide();
        groupBox->layout()->itemAt(6)->widget()->hide();
    }
}
void TxRemoveLiqDialog::init()
{
    QString key = "pid";
    this->addSimpleWidget("PID position", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "pool";
    this->addSimpleWidget("Pool parameters", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_name);
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "assets";
    this->addSimpleWidget("Current assets", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("assets"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "range";
    this->addSimpleWidget("Price range", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("range"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);


    key = "state";
    this->addSimpleWidget("Position state", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("state"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    if (sw->edit->text().toLower().contains("out")) color = "#8B0000";
    else color = "#4682B4";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "reward";
    this->addSimpleWidget("Unclimed fees", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("reward"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "is_simulate";
    this->addSimpleWidget("Simulate mode", LSimpleDialog::sdtBool, key);
    sw = this->widgetByKey(key);
    sw->checkBox->setChecked(true);
}
void TxRemoveLiqDialog::slotApply()
{
    m_data.dialog_params.clear();

    QString simulate_mode = (widgetValue("is_simulate").toBool() ? "yes" : "no");
    m_data.dialog_params.insert("simulate_mode", simulate_mode);

    LSimpleDialog::slotApply();
}



///////////////////TxMintPositionDialog////////////////////////////
TxMintPositionDialog::TxMintPositionDialog(TxDialogData &data, const TxDialogData &saved_params, QWidget *parent)
    :TxDialogBase(data, parent),
      m_mintParamsBox(NULL)
      //m_savedParams(saved_params)
{
    setObjectName(QString("tx_mint_dialog"));
    resize(950, 400);

    if (m_data.invalid()) return;
    init();
    applySavedParams(saved_params);

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

    //init price widgets
    key = "avr_price";
    this->addSimpleWidget("Average price", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "0.0");
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QPalette plt = sw->edit->palette();
    plt.setColor(QPalette::Base, "#F0E68C");
    sw->edit->setPalette(plt);

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

    key = "price1";
    sw = this->widgetByKey(key);
    connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCalcAverageRangePrice()));
    key = "price2";
    sw = this->widgetByKey(key);
    connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCalcAverageRangePrice()));

}
void TxMintPositionDialog::init()
{
    initStaticFields();
    initMintFields();

    replaceToGridLayout();
    slotRangeTypeChanged(0);
    //parseNoteText();
}
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
        for (int i=5; i<=6; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->hide();
        for (int i=1; i<=4; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->show();
    }
    else
    {
        for (int i=5; i<=6; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->show();
        for (int i=1; i<=4; i++)
            m_mintParamsBox->layout()->itemAt(i)->widget()->hide();
    }
}
void TxMintPositionDialog::slotCalcAverageRangePrice()
{
    QString s_p1 = this->widgetValue("price1").toString();
    QString s_p2 = this->widgetValue("price2").toString();

    bool ok;
    float p1 = s_p1.toFloat(&ok);
    if (ok && p1 > 0)
    {
        float p2 = s_p2.toFloat(&ok);
        if (ok && p2 > 0)
        {
            float a = (p1+p2)/2;
            this->setWidgetValue("avr_price", QString::number(a, 'f', SubGraph_CommonSettings::interfacePrecision(a)));
            return;
        }
    }
    this->setWidgetValue("avr_price", QString("?"));
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
void TxMintPositionDialog::applySavedParams(const TxDialogData &saved_params)
{
    if (m_data.token_addr != saved_params.token_addr)
    {qWarning("TxMintPositionDialog::applySavedParams  m_data.token_addr != saved_params.token_addr"); return;}

    qDebug("--------------saved_params---------------");
    QMap<QString, QString>::const_iterator it = saved_params.dialog_params.constBegin();
    while (it != saved_params.dialog_params.constEnd())
    {
        qDebug()<<QString("key[%1]  value[%2]").arg(it.key()).arg(it.value());
        it++;
    }
    qDebug("-------------------------------");

    if (saved_params.dialog_params.contains("l_tick") && saved_params.dialog_params.contains("h_tick"))
    {
        QString key = "range";
        const SimpleWidget *sw = this->widgetByKey(key);
        sw->comboBox->setCurrentIndex(1);
        this->setWidgetValue("tick1", saved_params.dialog_params.value("l_tick"));
        this->setWidgetValue("tick2", saved_params.dialog_params.value("h_tick"));
    }
    else if (saved_params.dialog_params.contains("l_price") && saved_params.dialog_params.contains("h_price"))
    {
        this->setWidgetValue("price1", saved_params.dialog_params.value("l_price"));
        this->setWidgetValue("price2", saved_params.dialog_params.value("h_price"));
        int t_index = saved_params.dialog_params.value("token_index", "0").toInt();
        if (t_index == 1)
        {
            QString key = "token_index";
            const SimpleWidget *sw = this->widgetByKey(key);
            sw->comboBox->setCurrentIndex(t_index);
        }
    }

    bool ok;
    float amount = -1;
    if (saved_params.dialog_params.contains("token0_amount"))
    {
        amount = saved_params.dialog_params.value("token0_amount").toFloat(&ok);
        if (ok && amount > 0)
        {
            this->setWidgetValue("amount0", saved_params.dialog_params.value("token0_amount"));
            return;
        }
    }
    if (saved_params.dialog_params.contains("token1_amount"))
    {
        amount = saved_params.dialog_params.value("token1_amount").toFloat(&ok);
        if (ok && amount > 0)
        {
            this->setWidgetValue("amount1", saved_params.dialog_params.value("token1_amount"));
            return;
        }
    }
}


///////////////////TxIncreaseLiqDialog////////////////////////////
TxIncreaseLiqDialog::TxIncreaseLiqDialog(TxDialogData &data, QWidget *parent)
    :TxDialogBase(data, parent)
{
    setObjectName(QString("tx_increase_liq_dialog"));
    resize(600, 500);

    if (m_data.invalid()) return;

    init();
    //initAmountFields();
    transformEditValues();
    addVerticalSpacer();

    this->setCaptionsWidth(180);
    setExpandWidgets();
}
void TxIncreaseLiqDialog::transformEditValues()
{
    QString key = "trange";
    //const SimpleWidget *sw = this->widgetByKey(key);
    QString text = widgetValue(key).toString();
    text.replace("(", "[");
    text.replace(")", "]");
    setWidgetValue(key, text);


    key = "prange";
    text = widgetValue(key).toString().trimmed();
    text.remove("(");
    text.remove(")");
    QStringList plist = LString::trimSplitList(text, ";");
    if (plist.count() == 2)
    {
        text = QString("P_low = %1    P_high = %2").arg(plist.at(0).trimmed()).arg(plist.at(1).trimmed());
        setWidgetValue(key, text);
    }
}
void TxIncreaseLiqDialog::init()
{
    QString key = "pid";
    this->addSimpleWidget("PID position", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_addr);
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    QString color = "#000080";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));

    key = "pool";
    this->addSimpleWidget("Pool parameters", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.token_name);
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "approved";
    this->addSimpleWidget("Approved assets", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("approved"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "trange";
    this->addSimpleWidget("Tick range", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("tick_range"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "prange";
    this->addSimpleWidget("Price range", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("price_range"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);

    key = "p_current";
    this->addSimpleWidget("Current price", LSimpleDialog::sdtString, key);
    this->setWidgetValue(key, m_data.dialog_params.value("current_price"));
    sw = this->widgetByKey(key);
    sw->edit->setReadOnly(true);
    color = "#0000CD";
    if (sw->edit->text().toLower().contains("out")) color = "#B22222";
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1;}").arg(color));


    //---------------------------------------------
    initAmountFields();

    key = "dead_line";
    this->addSimpleWidget("Dead line, secs", LSimpleDialog::sdtIntCombo, key);
    sw = this->widgetByKey(key);
    for (int i=1; i<15; i++)
        sw->comboBox->addItem(QString::number(i*20));
    sw->comboBox->setCurrentIndex(3);

    key = "is_simulate";
    this->addSimpleWidget("Simulate mode", LSimpleDialog::sdtBool, key);
    sw = this->widgetByKey(key);
    sw->checkBox->setChecked(true);
}
void TxIncreaseLiqDialog::initAmountFields()
{
    QString color = "#6B8E23";
    //--------------------------------
    QString key = "amount0";
    this->addSimpleWidget("Token_0 amount", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "1.00");
    const SimpleWidget *sw = this->widgetByKey(key);
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1; font-weight: bold;}").arg(color));
    connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
    //--------------------------------
    key = "amount1";
    this->addSimpleWidget("Token_1 amount", LSimpleDialog::sdtString, key, 2);
    this->setWidgetValue(key, "-1");
    sw = this->widgetByKey(key);
    sw->edit->setStyleSheet(QString("QLineEdit {color: %1; font-weight: bold;}").arg(color));
    connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
}
void TxIncreaseLiqDialog::slotApply()
{
    m_data.dialog_params.clear();

    QString err;
    checkIncreaseParamsValidity(err);
    if (!err.isEmpty())
    {
        QMessageBox::warning(this, "Error", err);
        m_data.dialog_params.clear();
        return;
    }

    QString simulate_mode = (widgetValue("is_simulate").toBool() ? "yes" : "no");
    m_data.dialog_params.insert("simulate_mode", simulate_mode);
    m_data.dialog_params.insert("dead_line", widgetValue("dead_line").toString().trimmed());

    LSimpleDialog::slotApply();
}
void TxIncreaseLiqDialog::slotAmountsChanged()
{
    QString sender_name = sender()->objectName();
    qDebug()<<QString("TxIncreaseLiqDialog  sender[%1]").arg(sender()->objectName());
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
        qWarning()<<QString("TxIncreaseLiqDialog::slotAmountsChanged() WARNING invalid sender name [%1]").arg(sender_name);
        return;
    }

    if (sw)
    {
        disconnect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
        sw->edit->setText(QString::number(-1));
        connect(sw->edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotAmountsChanged()));
    }
    else qWarning()<<QString("TxIncreaseLiqDialog::slotAmountsChanged() WARNING SW is NULL");
}
void TxIncreaseLiqDialog::checkIncreaseParamsValidity(QString &err)
{
    err.clear();
    bool ok;
    QString trange = widgetValue("trange").toString().trimmed();
    trange.remove("[");
    trange.remove("]");
    QStringList tlist = LString::trimSplitList(trange, ";");
    if (tlist.count() != 2) {err = "ticks range is incorrect value"; return;}

    int t1 = tlist.at(0).trimmed().toInt(&ok);
    if (!ok) {err = "tick1 value is invalid"; return;}
    int t2 = tlist.at(1).trimmed().toInt(&ok);
    if (!ok) {err = "tick2 value is invalid"; return;}
    if (t2 <= t1) {err = "ticks range is invalid, must (tick2 > tick1)"; return;}
    m_data.dialog_params.insert("l_tick", QString::number(t1));
    m_data.dialog_params.insert("h_tick", QString::number(t2));


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
*/


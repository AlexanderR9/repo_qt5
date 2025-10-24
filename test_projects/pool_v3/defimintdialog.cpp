#include "defimintdialog.h"
#include "txdialog.h"
#include "deficonfig.h"
#include "ltable.h"
#include "appcommonsettings.h"
#include "nodejsbridge.h"

#include <QDebug>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonValue>


//DefiMintDialog
DefiMintDialog::DefiMintDialog(TxDialogData &data, QWidget *parent)
    :QDialog(parent),
      m_data(data),
      m_apply(false)
{
    setupUi(this);
    setObjectName(QString("mint_pos_dialog"));
    setWindowTitle(QString("Mint position (%1)").arg(m_data.chain_name.trimmed().toUpper()));
    resize(1200, 500);
    setModal(true);


    init();
    initBaseTokens();

    connect(tokenComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotBaseTokenChanged()));
    connect(poolComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPoolChanged()));
    //connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotApply()));
    //connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));
    connect(updatePoolStateButton, SIGNAL(clicked()), this, SLOT(slotUpdatePoolState()));
    connect(amount0LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotAmountsEdited()));
    connect(amount1LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotAmountsEdited()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotEmulateMint()));
}
void DefiMintDialog::init()
{
    poolComboBox->clear();
    tokenComboBox->clear();

    ////////////////////poolStateTable////////////////////
    LTable::fullClearTable(poolStateTable);

    QStringList headers;
    headers << "State value";
    LTable::setTableHeaders(poolStateTable, headers, Qt::Horizontal);
    headers.clear();
    headers << "TVL, kUsdt" << "Tick" << "Price" << "Token0 balance" << "Token1 balance" << "Pool is stable";
    LTable::setTableHeaders(poolStateTable, headers, Qt::Vertical);
    LTable::createAllItems(poolStateTable);
    poolStateTable->setSelectionMode(QAbstractItemView::NoSelection);
    LTable::resizeTableContents(poolStateTable);

    updatePoolStateButton->setIcon(QIcon(QString("%1/view-refresh.svg").arg(AppCommonSettings::commonIconsPath())));
    updatePoolStateButton->setIconSize(QSize(22, 22));

    ////////////////////previewMintTable////////////////////
    LTable::fullClearTable(previewMintTable);
    headers.clear();
    headers << "First value" << "Second value";
    LTable::setTableHeaders(previewMintTable, headers, Qt::Horizontal);
    headers.clear();
    headers << "Price range" << "Tick range" << "Token amounts" << "Gas fee" << "Pool tick/price";
    LTable::setTableHeaders(previewMintTable, headers, Qt::Vertical);
    LTable::createAllItems(previewMintTable);
    previewMintTable->setSelectionMode(QAbstractItemView::NoSelection);

    resetPreviewTable();

}
void DefiMintDialog::initBaseTokens()
{
    int cid = defi_config.getChainID(m_data.chain_name);
    int chain_index = defi_config.chainIndexOf(cid);
    QString chain_icon = defi_config.chains.at(chain_index).fullIconPath();

    m_tokens.clear();
    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id == cid)
        {
            tokenComboBox->addItem(QIcon(chain_icon), v.name);
            m_tokens.insert(v.name, v.address);
        }
    }
    //tokenComboBox->setIconSize(QSize(12, 12));

    slotBaseTokenChanged();
    slotPoolChanged();

}
void DefiMintDialog::slotPoolChanged()
{
    resetPoolStateTable();
    resetPreviewTable();


    int cid = defi_config.getChainID(m_data.chain_name);
    int i = poolComboBox->currentIndex();
    QString pool_addr = poolComboBox->itemData(i).toString();
    int pool_index = defi_config.getPoolIndex(pool_addr);
    if (pool_index < 0)
    {
        QMessageBox::warning(this, "Warning", QString("Not found pool: %1").arg(pool_addr));
        return;
    }

    QString t0_name = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token0_addr, cid);
    QString t1_name = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token1_addr, cid);

    //fill table
    QString p_stable = (defi_config.isStablePool(pool_addr) ? "yes" : "no");
    QString p_color = ((p_stable == "yes") ? "#0000FF" : "#778899");
    poolStateTable->item(5, 0)->setText(p_stable);
    poolStateTable->item(5, 0)->setTextColor(p_color);

    float balance0 = -1;
    float balance1 = -1;
    emit signalGetTokenBalance(t0_name, balance0);
    emit signalGetTokenBalance(t1_name, balance1);
    poolStateTable->item(3, 0)->setText(QString::number(balance0));
    poolStateTable->item(4, 0)->setText(QString::number(balance1));

}
void DefiMintDialog::slotAmountsEdited()
{
    qDebug("DefiMintDialog::slotAmountsEdited()");
    if (!sender()) {qWarning("DefiMintDialog::slotAmountsEdited() WARNING sender is NULL"); return;}
    const QLineEdit *edit = qobject_cast<const QLineEdit*>(sender());
    if (!edit) {qWarning("DefiMintDialog::slotAmountsEdited() WARNING sender can not qobject_cast<const QLineEdit*>"); return;}


    disconnect(amount0LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotAmountsEdited()));
    disconnect(amount1LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotAmountsEdited()));
    if (edit->objectName() == "amount0LineEdit")
    {
        amount1LineEdit->setText("-1");
    }
    else
    {
        amount0LineEdit->setText("-1");
    }
    connect(amount0LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotAmountsEdited()));
    connect(amount1LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotAmountsEdited()));

    qDebug("DefiMintDialog::slotAmountsEdited() end");
}
void DefiMintDialog::slotBaseTokenChanged()
{
    disconnect(poolComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPoolChanged()));

    int cid = defi_config.getChainID(m_data.chain_name);
    int chain_index = defi_config.chainIndexOf(cid);
    QString chain_icon = defi_config.chains.at(chain_index).fullIconPath();

    QString pool_info;
    poolComboBox->clear();
    QString t_addr = baseTokenAddrSelected();
    qDebug()<<QString("DefiMintDialog::slotBaseTokenChainged() token[%1]  address[%2]").arg(tokenComboBox->currentText()).arg(t_addr);
    foreach (const DefiPoolV3 &v, defi_config.pools)
    {
        if (v.chain_id != cid) continue;
        if (v.token0_addr == t_addr || v.token1_addr == t_addr)
        {
            QString t0 = defi_config.tokenNameByAddress(v.token0_addr, cid);
            QString t1 = defi_config.tokenNameByAddress(v.token1_addr, cid);
            pool_info = QString("%1/%2 (%3)").arg(t0).arg(t1).arg(v.strFloatFee());
            poolComboBox->addItem(QIcon(chain_icon), pool_info);
            poolComboBox->setItemData(poolComboBox->count()-1, v.address);
        }
    }

    connect(poolComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPoolChanged()));
}
void DefiMintDialog::slotUpdatePoolState()
{
    qDebug("DefiMintDialog::slotUpdatePoolState()");
    updatePoolStateButton->setEnabled(false);
    this->buttonBox->setEnabled(false);

    QString pool_addr = poolComboBox->itemData(poolComboBox->currentIndex()).toString();
    emit signalTryUpdatePoolState(pool_addr);
}
QString DefiMintDialog::baseTokenAddrSelected() const
{
    QString t_name = tokenComboBox->currentText().trimmed();
    if (t_name.isEmpty()) return QString();
    return m_tokens.value(t_name, QString());
}
void DefiMintDialog::slotApply()
{
    qDebug("DefiMintDialog::slotApply()");
    m_apply = true;
    close();
}
void DefiMintDialog::resetPoolStateTable()
{
    int rows = poolStateTable->rowCount();
    for(int i=0; i<rows; i++)
        poolStateTable->item(i, 0)->setText("-");
}
void DefiMintDialog::resetPreviewTable()
{
    LTable::clearAllItemsText(previewMintTable);
    buttonBox->button(QDialogButtonBox::Apply)->setText("Emulate");
    LTable::resizeTableContents(previewMintTable);
}
void DefiMintDialog::poolStateReceived(const QStringList &p_state) const
{
    qDebug("DefiMintDialog::poolStateReceived()");
    updatePoolStateButton->setEnabled(true);
    this->buttonBox->setEnabled(true);

    if (p_state.count() == 3)
    {
        for (int i=0; i<p_state.count(); i++)
            poolStateTable->item(i, 0)->setText(p_state.at(i));
    }

    initMintSettings();
}
void DefiMintDialog::initMintSettings() const
{
    qDebug("DefiMintDialog::initMintSettings() 1");
    bool ok = false;
    qDebug("DefiMintDialog::initMintSettings() 11");
    float p = poolStateTable->item(2, 0)->text().toFloat(&ok);
    qDebug("DefiMintDialog::initMintSettings() 111");
    if (p > 0 && ok)
    {
        qDebug("DefiMintDialog::initMintSettings() ok");
        p1LineEdit->setText(QString::number(p));
        p2LineEdit->setText(QString::number(p*1.2));
    }
    else
    {
        qDebug("DefiMintDialog::initMintSettings() fault");
        p1LineEdit->setText("?");
        p2LineEdit->setText("?");
    }

    qDebug("DefiMintDialog::initMintSettings() 2");
    float amount = poolStateTable->item(3, 0)->text().toFloat(&ok);
    if (amount > 0 && ok)
    {
        amount0LineEdit->setText(QString::number(amount));
    }
    else
    {
        amount1LineEdit->setText("?");
    }

    qDebug("DefiMintDialog::initMintSettings() ok");
}
void DefiMintDialog::slotEmulateMint()
{
    bool ok;
    qDebug("DefiMintDialog::slotEmulateMint()");
    int cid = defi_config.getChainID(m_data.chain_name);
    m_data.dialog_params.clear();
    m_data.dialog_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(txMint));

    m_data.pool_addr = poolComboBox->itemData(poolComboBox->currentIndex()).toString();
    int pool_index = defi_config.getPoolIndex(m_data.pool_addr);
    if (pool_index < 0)
    {
        QMessageBox::warning(this, "Warning", QString("Not found pool: %1").arg(m_data.pool_addr));
        return;
    }

    // pool info
    m_data.dialog_params.insert("pool_address", m_data.pool_addr);
    m_data.dialog_params.insert("token0_address", defi_config.pools.at(pool_index).token0_addr);
    m_data.dialog_params.insert("token1_address", defi_config.pools.at(pool_index).token1_addr);
    m_data.dialog_params.insert("fee", QString::number(defi_config.pools.at(pool_index).fee));

    //amount info
    float amount0 = amount0LineEdit->text().toFloat(&ok);
    if (!ok) {m_data.dialog_params.insert("error", "invalid amount0 value"); return;}
    float amount1 = amount1LineEdit->text().toFloat(&ok);
    if (!ok) {m_data.dialog_params.insert("error", "invalid amount1 value"); return;}
    m_data.dialog_params.insert("token0_amount", amount0LineEdit->text().trimmed());
    m_data.dialog_params.insert("token1_amount", amount1LineEdit->text().trimmed());

    //range
    float p1 = p1LineEdit->text().toFloat(&ok);
    if (!ok) {m_data.dialog_params.insert("error", "invalid P1 value"); return;}
    float p2 = p2LineEdit->text().toFloat(&ok);
    if (!ok) {m_data.dialog_params.insert("error", "invalid P2 value"); return;}
    if (p1 >= p2) {m_data.dialog_params.insert("error", "invalid range, P2 <= P1"); return;}

    QString t0_name = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token0_addr, cid);
    QString t1_name = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token1_addr, cid);
    int price_prior_index = defi_config.getPoolTokenPriceIndex(QString("%1/%2").arg(t0_name).arg(t1_name));
    if (price_prior_index == 1) // необходимо конвертировать ценовой диапазон в значения для токена_0
    {
        float p1_0 = 1/p2;
        float p2_0 = 1/p1;
        p1 = p1_0;
        p2 = p2_0;
    }
    m_data.dialog_params.insert("p1", QString::number(p1, 'f', 8));
    m_data.dialog_params.insert("p2", QString::number(p2, 'f', 8));

    if (buttonBox->button(QDialogButtonBox::Apply)->text().toLower().contains("mint"))
    {
        qDebug("----------- REAL MINT CMD----------------");
        m_data.dialog_params.insert(AppCommonSettings::nodejsTxSimulateFieldName(), "no");
        slotApply();
    }
    else
    {
        m_data.dialog_params.insert(AppCommonSettings::nodejsTxSimulateFieldName(), "yes");
        emit signalEmulateMint(m_data);
    }


/*
p1: // low price of range for token0
p2: // high price of range for token0
pool_address: "0x..."    //адрес пула
token0_address
token1_address
fee: <int_value> //example 100/500/3000
token0_amount: // вносимая сумма токена_0 в нормальных пользовательских единицах (или -1)
token1_amount: // вносимая сумма токена_1 в нормальных пользовательских единицах (или -1)
*/


}
void DefiMintDialog::emulMintReply(const QJsonObject &js_reply) const
{
    int row = 0;
    previewMintTable->item(row, 0)->setText(js_reply.value("p1").toString().trimmed());
    previewMintTable->item(row, 1)->setText(js_reply.value("p2").toString().trimmed());
    row++;
    previewMintTable->item(row, 0)->setText(js_reply.value("tick1").toString().trimmed());
    previewMintTable->item(row, 1)->setText(js_reply.value("tick2").toString().trimmed());
    row++;
    previewMintTable->item(row, 0)->setText(js_reply.value("amount0").toString().trimmed());
    previewMintTable->item(row, 1)->setText(js_reply.value("amount1").toString().trimmed());
    LTable::setTableTextRowColor(previewMintTable, row, "#4682B4");

    row++;
    previewMintTable->item(row, 0)->setText(js_reply.value("estimated_gas").toString().trimmed());
    previewMintTable->item(row, 1)->setText("gas_limit");
    LTable::setTableTextRowColor(previewMintTable, row, Qt::gray);
    row++;
    previewMintTable->item(row, 0)->setText(js_reply.value("pool_tick").toString().trimmed());
    previewMintTable->item(row, 1)->setText(js_reply.value("pool_price").toString().trimmed());
    LTable::setTableTextRowColor(previewMintTable, row, Qt::gray);

    LTable::resizeTableContents(previewMintTable);
    buttonBox->button(QDialogButtonBox::Apply)->setText("Mint TX");

}

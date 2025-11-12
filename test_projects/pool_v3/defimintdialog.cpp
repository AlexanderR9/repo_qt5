#include "defimintdialog.h"
#include "txdialog.h"
#include "deficonfig.h"
#include "ltable.h"
#include "lstring.h"
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
    connect(p1LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotPrevewTableReset()));
    connect(p2LineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotPrevewTableReset()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotMint()));
}
void DefiMintDialog::slotPrevewTableReset()
{
    resetPreviewTable();

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
}
void DefiMintDialog::slotPoolChanged()
{
    qDebug("DefiMintDialog::slotPoolChanged()");

    resetPoolStateTable();
    resetPreviewTable();

    int i = poolComboBox->currentIndex();
    QString pool_addr = poolComboBox->itemData(i).toString();
    int pool_index = defi_config.getPoolIndex(pool_addr);
    if (pool_index < 0)
    {
        QMessageBox::warning(this, "Warning", QString("Not found pool: %1").arg(pool_addr));
        return;
    }


    //fill table
    QString p_stable = (defi_config.isStablePool(pool_addr) ? "yes" : "no");
    QString p_color = ((p_stable == "yes") ? "#0000FF" : "#778899");
    poolStateTable->item(5, 0)->setText(p_stable);
    poolStateTable->item(5, 0)->setTextColor(p_color);

    // get wallet balances
    updateTokensBalances();
}
void DefiMintDialog::updateTokensBalances()
{
    QPair<QString, QString> token_pair;
    getTokensPairOfPool(token_pair);
    if (token_pair.first.length() < 2 || token_pair.second.length() < 2)
    {
        poolStateTable->item(3, 0)->setText("err");
        poolStateTable->item(4, 0)->setText("err");
        return;
    }

    float balance0 = -1;
    float balance1 = -1;
    emit signalGetTokenBalance(token_pair.first, balance0);
    emit signalGetTokenBalance(token_pair.second, balance1);
    poolStateTable->item(3, 0)->setText(QString::number(balance0));
    poolStateTable->item(4, 0)->setText(QString::number(balance1));

    LTable::resizeTableContents(poolStateTable);
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


    slotPrevewTableReset();
    qDebug("DefiMintDialog::slotAmountsEdited() end");
}
void DefiMintDialog::slotBaseTokenChanged()
{
    qDebug("------------");
    qDebug("DefiMintDialog::slotBaseTokenChanged()");
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

    slotPoolChanged();
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
    LTable::resizeTableContents(poolStateTable);

    initMintSettings();    
}
void DefiMintDialog::initMintSettings() const
{
    qDebug("DefiMintDialog::initMintSettings() 1");
    bool ok = false;
    bool stb = (poolStateTable->item(5, 0)->text() == "yes");
    float p = poolStateTable->item(2, 0)->text().toFloat(&ok);
    int tick = poolStateTable->item(1, 0)->text().toInt(&ok);
    if (p > 0 && ok)
    {
        if (stb)
        {
            p1LineEdit->setText(QString("t%1").arg(tick));
            p2LineEdit->setText(QString("t%1").arg(tick+1));
        }
        else
        {
            p1LineEdit->setText(QString::number(p));
            p2LineEdit->setText(QString::number(p*1.2));
        }
    }
    else
    {
        p1LineEdit->setText("?");
        p2LineEdit->setText("?");
    }

    float amount = poolStateTable->item(3, 0)->text().toFloat(&ok);
    if (amount > 0 && ok)
    {
        amount0LineEdit->setText(QString::number(amount));
    }
    else
    {
        amount1LineEdit->setText("?");
    }
}
void DefiMintDialog::getTokensPairOfPool(QPair<QString, QString> &pair) const
{
    pair.first = pair.second = "?";
    int cid = defi_config.getChainID(m_data.chain_name);
    m_data.pool_addr = poolComboBox->itemData(poolComboBox->currentIndex()).toString();
    int pool_index = defi_config.getPoolIndex(m_data.pool_addr);
    if (pool_index < 0) return;

    pair.first = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token0_addr, cid); // token0
    pair.second = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token1_addr, cid); // token1
}
void DefiMintDialog::checkValidityMintParams(QString &err)
{
    err.clear();
    bool ok = false;
    float amount0 = amount0LineEdit->text().toFloat(&ok);
    if (!ok) {err = "invalid amount0 value"; return;}
    float amount1 = amount1LineEdit->text().toFloat(&ok);
    if (!ok) {err = "invalid amount1 value"; return;}
    if (amount0 <= 0 && amount1 <= 0) {err = "invalid combination values of amount0/amount1"; return;}


    QString sp1 = p1LineEdit->text().trimmed();
    QString sp2 = p2LineEdit->text().trimmed();
    if (sp1.isEmpty()) {err = "range price1 is empty"; return;}
    if (sp2.isEmpty()) {err = "range price2 is empty"; return;}
    if (sp1.at(0) == QChar('t') && sp2.at(0) != QChar('t')) {err = "price range must be all is ticks"; return;}
    if (sp1.at(0) != QChar('t') && sp2.at(0) == QChar('t')) {err = "price range must be all is ticks"; return;}

    bool is_tick_range = (sp1.at(0) == QChar('t')); // диапазон указан в виде тиков
    if (is_tick_range)
    {
        sp1 = LString::strTrimLeft(sp1, 1);
        sp2 = LString::strTrimLeft(sp2, 1);
        int t1 = sp1.toInt(&ok);
        if (!ok) {err = "invalid TICK_1 value"; return;}
        int t2 = sp2.toInt(&ok);
        if (!ok) {err = "invalid TICK_2 value"; return;}
        if (t1 >= t2) {err = "invalid range, TICK_1 >= TICK_2"; return;}
    }
    else
    {
        float p1 = sp1.toFloat(&ok);
        if (!ok) {err = "invalid P1 value"; return;}
        float p2 = sp2.toFloat(&ok);
        if (!ok) {err = "invalid P2 value"; return;}
        if (p1 >= p2) {err = "invalid range, P2 <= P1"; return;}
    }
}
void DefiMintDialog::fillMintParams(int pool_index)
{
    m_data.dialog_params.insert("pool_address", m_data.pool_addr);
    m_data.dialog_params.insert("token0_address", defi_config.pools.at(pool_index).token0_addr);
    m_data.dialog_params.insert("token1_address", defi_config.pools.at(pool_index).token1_addr);
    m_data.dialog_params.insert("fee", QString::number(defi_config.pools.at(pool_index).fee));
    m_data.dialog_params.insert("token0_amount", amount0LineEdit->text().trimmed());
    m_data.dialog_params.insert("token1_amount", amount1LineEdit->text().trimmed());


    //range
    QString sp1 = p1LineEdit->text().trimmed();
    QString sp2 = p2LineEdit->text().trimmed();
    bool is_tick_range = (sp1.at(0) == QChar('t')); // диапазон указан в виде тиков
    if (is_tick_range)
    {
        sp1 = LString::strTrimLeft(sp1, 1);
        sp2 = LString::strTrimLeft(sp2, 1);
        m_data.dialog_params.insert("tick1", sp1);
        m_data.dialog_params.insert("tick2", sp2);
    }
    else
    {
        float p1 = sp1.toFloat();
        float p2 = sp2.toFloat();

        QPair<QString, QString> token_pair;
        getTokensPairOfPool(token_pair);
        int price_prior_index = defi_config.getPoolTokenPriceIndex(QString("%1/%2").arg(token_pair.first).arg(token_pair.second));
        if (price_prior_index == 1) // необходимо конвертировать ценовой диапазон в значения для токена_0
        {
            float p1_0 = 1/p2;
            float p2_0 = 1/p1;
            p1 = p1_0;
            p2 = p2_0;
        }

        m_data.dialog_params.insert("p1", QString::number(p1, 'f', 8));
        m_data.dialog_params.insert("p2", QString::number(p2, 'f', 8));
    }
}
void DefiMintDialog::slotMint()
{
    bool ok;
    qDebug("DefiMintDialog::slotMint()");
    m_data.dialog_params.clear();
    m_data.dialog_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(txMint));

    // check params
    QString err;
    checkValidityMintParams(err);
    if (!err.isEmpty())
    {
        QMessageBox::warning(this, "Inalid mint params", err);
        m_data.dialog_params.insert("error", err); return;
        return;
    }
    m_data.pool_addr = poolComboBox->itemData(poolComboBox->currentIndex()).toString();
    int pool_index = defi_config.getPoolIndex(m_data.pool_addr);
    if (pool_index < 0)
    {
        QMessageBox::warning(this, "Warning", QString("Not found pool: %1").arg(m_data.pool_addr));
        return;
    }


    //fill mint params
    fillMintParams(pool_index);

    // try mint
    QString btn_caption = buttonBox->button(QDialogButtonBox::Apply)->text().toLower();
    if (btn_caption.contains("mint")) // real TX
    {
        qDebug("----------- REAL MINT CMD----------------");
        m_data.dialog_params.insert(AppCommonSettings::nodejsTxSimulateFieldName(), "no");
        slotApply();
    }
    else // simulate mode
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
    float p1 = js_reply.value("p1").toString().toFloat(); // for token0
    float p2 = js_reply.value("p2").toString().toFloat(); // for token0
    QPair<QString, QString> token_pair;
    getTokensPairOfPool(token_pair);
    int price_prior_index = defi_config.getPoolTokenPriceIndex(QString("%1/%2").arg(token_pair.first).arg(token_pair.second));
    if (price_prior_index == 1) // необходимо конвертировать ценовой диапазон в значения для токена_1
    {
        float p1_1 = 1/p2;
        float p2_1 = 1/p1;
        p1 = p1_1;
        p2 = p2_1;
    }
    previewMintTable->item(row, 0)->setText(QString::number(p1, 'f', 6));
    previewMintTable->item(row, 1)->setText(QString::number(p2, 'f', 6));
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

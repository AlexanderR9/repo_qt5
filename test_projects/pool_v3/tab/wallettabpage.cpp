#include "wallettabpage.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "txdialog.h"
#include "nodejsbridge.h"
#include "txlogrecord.h"


#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

#define ADDRESS_COL         1
#define TOKEN_COL           0
#define BALANCE_COL         5
#define PRICE_COL           4
#define AMOUNT_COL          3

#define AMOUNT_PRECISION            6
#define BALANCE_PRECISION           2



// DefiWalletTabPage
DefiWalletTabPage::DefiWalletTabPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkWallet)
{
    setObjectName("defi_wallet_page");

    //init table
    initTable();

    //init context menu
    initPopupMenu();

    /*



    //init BalanceHistory
    initBalanceHistoryObj();
    */
}
void DefiWalletTabPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Assets list");
    m_table->vHeaderHide();
    m_table->setObjectName("wallet_table");
    QStringList headers;
    headers << "Token" << "Address" << "Decimal" << "Amount" << "Price" << "Balance";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");
    h_splitter->addWidget(m_table);

    /////////////////////////////////////////

    m_integratedTable = new LTableWidgetBox(this);
    m_integratedTable->setTitle("Integrated state");
    m_integratedTable->setObjectName("wallet_integrated_table");
    headers.clear();
    headers << "Value";
    LTable::setTableHeaders(m_integratedTable->table(), headers);
    headers.clear();
    headers << "Balance" << "N assets" << "TX count" <<  "Gas price" << "Chain ID";
    LTable::setTableHeaders(m_integratedTable->table(), headers, Qt::Vertical);
    m_integratedTable->setSelectionMode(QAbstractItemView::NoSelection, QAbstractItemView::NoSelection);
    LTable::createAllItems(m_integratedTable->table());
    h_splitter->addWidget(m_integratedTable);
}
QString DefiWalletTabPage::tickerByAddress(const QString &t_addr) const
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == t_addr)
            return t->item(i, TOKEN_COL)->text().trimmed();
    }
    return QString();
}
void DefiWalletTabPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);
    initTokenList(cid);

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();
}
void DefiWalletTabPage::updatePrices() const
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        QString ticker = t->item(i, TOKEN_COL)->text().trimmed();
        foreach (const DefiToken &v, defi_config.tokens)
        {
            if ((v.name == ticker && !v.is_stable) || (v.isWraped() && v.name == QString("W%1").arg(ticker)))
            {
                quint8 prs = AppCommonSettings::interfacePricePrecision(v.last_price);
                t->item(i, PRICE_COL)->setText(QString::number(v.last_price, 'f', prs));
                break;
            }
        }

        updateBalance(i);
    }
    updateTotalBalance();

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();
}
void DefiWalletTabPage::initTokenList(int cid)
{
    QTableWidget *t = m_table->table();
    m_table->removeAllRows();

    int i_chain = defi_config.chainIndexOf(cid);
    if (i_chain < 0) return;

    QStringList row_data;
    row_data << defi_config.chains.at(i_chain).coin << AppCommonSettings::nativeTokenAddress() <<
         QString::number(AppCommonSettings::nativeTokenDecimal()) << "?" << "---" << "---";

    //add native coin
    LTable::addTableRow(t, row_data);
    int need_native_icon = true;
    //LTable::setTableRowColor(t, 0, "#F0E68C");

    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id != cid)
        {
            if (need_native_icon)
            {
                if (v.name == defi_config.chains.at(i_chain).coin)
                {
                    need_native_icon = false;
                    t->item(0, 0)->setIcon(QIcon(v.fullIconPath()));
                }
            }
            continue;
        }

        row_data.clear();
        row_data << v.name << v.address << QString::number(v.decimal) << "?" << "---" << "---";
        LTable::addTableRow(t, row_data);

        if (v.is_stable)
        {
            t->item(t->rowCount()-1, ADDRESS_COL)->setTextColor("#0000CD");
            t->item(t->rowCount()-1, TOKEN_COL)->setData(Qt::UserRole, "stable");
        }
        else t->item(t->rowCount()-1, ADDRESS_COL)->setTextColor("#4682B4");
        t->item(t->rowCount()-1, AMOUNT_COL)->setTextColor(Qt::gray);

        QTableWidgetItem *item = t->item(t->rowCount()-1, 0);
        if (item) item->setIcon(QIcon(v.fullIconPath()));
    }

    if (need_native_icon)
    {
        t->item(0, 0)->setIcon(QIcon(defi_config.chains.at(i_chain).fullIconPath()));
    }


    t->setIconSize(QSize(24, 24));
    m_integratedTable->table()->item(1, 0)->setText(QString::number(t->rowCount()));

}
void DefiWalletTabPage::sendUpdateDataRequest()
{
    qDebug("DefiWalletTabPage::sendUpdateDataRequest()");
    //prepare json params
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcBalance));
    sendReadNodejsRequest(j_params);
}
void DefiWalletTabPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiWalletTabPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcBalance)) updateAmounts(js_reply);
    else if (req == NodejsBridge::jsonCommandValue(nrcTXCount)) updateIntegratedTable(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(nrcGasPrice)) updateIntegratedTable(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(nrcChainID)) updateIntegratedTable(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txWrap)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txUnwrap)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txTransfer)) checkTxResult(req, js_reply);
    else return;

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();
}
void DefiWalletTabPage::updateIntegratedTable(QString req, const QJsonObject &js_reply)
{
    QTableWidget *t_int = m_integratedTable->table();
    if (req == NodejsBridge::jsonCommandValue(nrcTXCount))
    {
        if (!js_reply.contains("data"))
        {
            t_int->item(2, 0)->setText("err");
            t_int->item(2, 0)->setTextColor(Qt::red);
        }
        else t_int->item(2, 0)->setText(js_reply.value("data").toString());
    }
    else if (req == NodejsBridge::jsonCommandValue(nrcGasPrice))
    {
        if (!js_reply.contains("data"))
        {
            t_int->item(3, 0)->setText("err");
            t_int->item(3, 0)->setTextColor(Qt::red);
        }
        else t_int->item(3, 0)->setText(js_reply.value("data").toString());
    }
    else if (req == NodejsBridge::jsonCommandValue(nrcChainID))
    {
        if (!js_reply.contains("data"))
        {
            t_int->item(4, 0)->setText("err");
            t_int->item(4, 0)->setTextColor(Qt::red);
        }
        else t_int->item(4, 0)->setText(js_reply.value("data").toString());
    }
}
void DefiWalletTabPage::updateAmounts(const QJsonObject &js_reply)
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    if (n_rows <= 0) return;

    QStringList keys = js_reply.keys();
    for (int i=0; i<n_rows; i++)
    {
        QString ticker = t->item(i, TOKEN_COL)->text().trimmed();
        if (keys.contains(ticker))
        {
            t->item(i, AMOUNT_COL)->setText(js_reply.value(ticker).toString());
            t->item(i, AMOUNT_COL)->setTextColor(Qt::black);
        }
        else
        {
            t->item(i, AMOUNT_COL)->setText(QString("err"));
            t->item(i, AMOUNT_COL)->setTextColor(Qt::red);
        }

        updateBalance(i);
    }

    if (t->item(0, PRICE_COL)->text() ==  "---") emit signalGetPrices();
    else updateTotalBalance();
}
bool DefiWalletTabPage::hasBalances() const
{
    bool ok = false;
    if (m_table->table()->rowCount() <= 0) return ok;
    m_table->table()->item(0, AMOUNT_COL)->text().toFloat(&ok);
    return ok;
}
void DefiWalletTabPage::updateBalance(int row) const
{
    QTableWidget *t = m_table->table();
    t->item(row, BALANCE_COL)->setTextColor(Qt::gray);
    t->item(row, BALANCE_COL)->setText("---");

    bool ok;
    float amount = t->item(row, AMOUNT_COL)->text().toFloat(&ok);
    if (!ok) return;

    float p = 1.0;
    if (t->item(row, TOKEN_COL)->data(Qt::UserRole).toString() != "stable")
    {
        p = t->item(row, PRICE_COL)->text().toFloat(&ok);
        if (!ok) return;
    }

    float b = p * amount;
    t->item(row, BALANCE_COL)->setText(QString::number(b, 'f', BALANCE_PRECISION));
    if (b < 1) t->item(row, BALANCE_COL)->setTextColor(Qt::lightGray);
    else t->item(row, BALANCE_COL)->setTextColor(Qt::darkGreen);
}
void DefiWalletTabPage::updateTotalBalance() const
{
    QTableWidget *t_int = m_integratedTable->table();
    t_int->item(0, 0)->setTextColor(Qt::gray);
    t_int->item(0, 0)->setText("???");

    bool ok;
    float tb = -1;
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        float b = t->item(i, BALANCE_COL)->text().toFloat(&ok);
        if (!ok) continue;

        if (tb < 0) tb = b;
        else tb += b;
    }

    if (tb >= 0)
    {
        t_int->item(0, 0)->setText(QString("%1 USDT").arg(QString::number(tb, 'f', 1)));
        t_int->item(0, 0)->setTextColor(Qt::darkGreen);
    }
}
void DefiWalletTabPage::initPopupMenu()
{
    QString path = AppCommonSettings::commonIconsPath(); // icons path

    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Wrap", TxDialogBase::iconByTXType(txWrap));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Unwrap", TxDialogBase::iconByTXType(txUnwrap));
    act_list.append(pair2);
    QPair<QString, QString> pair3("Transfer", TxDialogBase::iconByTXType(txTransfer));
    act_list.append(pair3);
    QPair<QString, QString> pair4("Get prices", QString("%1%2coin.svg").arg(path).arg(QDir::separator()));
    act_list.append(pair4);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotWrap())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotUnwrap())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTransfer())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetAssetPrices())); i_menu++;

    /////////////////////////////////////////////////////////

    //prepare menu actions data for integrated table
    //act_list.clear();
    act_list[0].first = "TX count";
    act_list[0].second = QString("%1%2crc.png").arg(path).arg(QDir::separator());
    act_list[1].first = "Gas price";
    act_list[1].second = QString("%1%2gas.png").arg(path).arg(QDir::separator());
    act_list[2].first = "Chain ID";
    act_list[2].second = QString("%1%2chain.svg").arg(path).arg(QDir::separator());
    act_list.removeLast();
    m_integratedTable->popupMenuActivate(act_list, false);

    //connect OWN slots to popup actions
    i_menu = 0;
    m_integratedTable->connectSlotToPopupAction(i_menu, this, SLOT(slotGetTxCount())); i_menu++;
    m_integratedTable->connectSlotToPopupAction(i_menu, this, SLOT(slotGetGasPrice())); i_menu++;
    m_integratedTable->connectSlotToPopupAction(i_menu, this, SLOT(slotGetChainID())); i_menu++;
}
void DefiWalletTabPage::slotWrap()
{
    if (!hasBalances()) {emit signalError("DefiWalletTabPage: Balances must updated"); return;}

    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiWalletTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    TxDialogData data(txWrap, curChainName());
    data.token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    data.dialog_params.insert("balance", t->item(row, AMOUNT_COL)->text());
    TxWrapDialog d(data, this);

    d.exec();
    if (d.isApply()) sendTxNodejsRequest(data);
    else emit signalMsg("operation was canceled!");
}
void DefiWalletTabPage::slotUnwrap()
{
    if (!hasBalances()) {emit signalError("DefiWalletTabPage: Balances must updated"); return;}

    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiWalletTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    TxDialogData data(txUnwrap, curChainName());
    data.token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    data.dialog_params.insert("balance", t->item(row, AMOUNT_COL)->text());
    TxWrapDialog d(data, this);

    d.exec();
    if (d.isApply()) sendTxNodejsRequest(data);
    else emit signalMsg("operation was canceled!");
}
void DefiWalletTabPage::slotTransfer()
{
    qDebug("DefiWalletTabPage::slotTransfer()");
    if (!hasBalances()) {emit signalError("DefiWalletTabPage: Balances must updated"); return;}

    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiWalletTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    TxDialogData data(txTransfer, curChainName());
    data.token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    data.dialog_params.insert("balance", t->item(row, AMOUNT_COL)->text());
    TxTransferDialog d(data, this);

    d.exec();
    if (d.isApply()) sendTxNodejsRequest(data);
    else emit signalMsg("operation was canceled!");
}
void DefiWalletTabPage::slotGetTxCount()
{
    qDebug("DefiWalletTabPage::slotGetTxCount()");
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcTXCount));
    sendReadNodejsRequest(j_params);
}
void DefiWalletTabPage::slotGetGasPrice()
{
    qDebug("DefiWalletTabPage::slotGetGasPrice()");
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcGasPrice));
    sendReadNodejsRequest(j_params);
}
void DefiWalletTabPage::slotGetChainID()
{
    qDebug("DefiWalletTabPage::slotGetChainID()");
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcChainID));
    sendReadNodejsRequest(j_params);
}
void DefiWalletTabPage::checkTxResult(QString req, const QJsonObject &js_reply)
{
    bool is_simulate = (js_reply.value(AppCommonSettings::nodejsTxSimulateFieldName()).toString() == "yes");
    emit signalMsg("");
    emit signalMsg(QString("//////////// REPLY form TX request [%1] ///////////////").arg(req));
    int result_code = js_reply.value("code").toString().toInt();
    emit signalMsg(QString("simulate_mode = %1").arg(is_simulate?"YES":"NO"));
    emit signalMsg(QString("result_code = %1").arg(result_code));

    if (is_simulate)
    {
        emit signalMsg(QString("estimated_gas = %1").arg(js_reply.value("estimated_gas").toString()));
    }
    else if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        logTxRecord(req, js_reply);
    }
}
void DefiWalletTabPage::logTxRecord(QString req, const QJsonObject &js_reply)
{
    TxLogRecord tx_rec(req, curChainName());
    tx_rec.tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();

    if (req == NodejsBridge::jsonCommandValue(txWrap))
    {
        tx_rec.wallet.token_addr = js_reply.value("token_address").toString();
        tx_rec.wallet.token_amount = js_reply.value("amount").toString().toFloat();
        tx_rec.note = QString("wrap %1").arg(defi_config.nativeTokenName(curChainName()));
    }
    else if (req == NodejsBridge::jsonCommandValue(txUnwrap))
    {
        tx_rec.wallet.token_addr = js_reply.value("token_address").toString();
        tx_rec.wallet.token_amount = js_reply.value("amount").toString().toFloat();
        tx_rec.note = QString("unwrap %1").arg(defi_config.nativeTokenName(curChainName()));
    }
    else if (req == NodejsBridge::jsonCommandValue(txTransfer))
    {
        tx_rec.wallet.token_addr = js_reply.value("token_address").toString().trimmed();
        tx_rec.wallet.token_amount = js_reply.value("amount").toString().toFloat();
        tx_rec.wallet.target_wallet = js_reply.value("to_wallet").toString();
        tx_rec.note = QString("transfer %1").arg(tickerByAddress(tx_rec.wallet.token_addr));
    }
    emit signalNewTx(tx_rec);
}




/*
void JSWalletTab::initBalanceHistoryObj()
{
    m_balanceHistory = new WalletBalanceHistory(this);
    connect(m_balanceHistory, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_balanceHistory, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));

}

*/




/*


void JSWalletTab::slotWrap()
{
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("JSWalletTab: You must select row"); return;}
    QTableWidget *t = m_table->table();

    QStringList tx_params;
    tx_params << "qt_wrap.js";

    //prepare dialog params
    TxDialogData data(txWrap);
    data.token_name = t->item(row, TOKEN_COL)->text().trimmed();
    TxWrapDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        QString s_amount = data.dialog_params.value("amount");
        emit signalMsg(QString("Try send TX wrap"));
        emit signalMsg(QString("PARAMS: token=%1  amount=%2").arg(data.token_name).arg(s_amount));
        if (s_amount == "invalid") {emit signalError(QString("invalid amount, operation canceled!")); return;}

        t->setEnabled(false);
        tx_params << TxWrapDialog::captionByTXType(data.tx_kind).trimmed().toLower();
        tx_params << data.token_name << data.dialog_params.value("amount");
        signalWalletTx(tx_params);
    }
    else emit signalMsg("operation was canceled!");
}
void JSWalletTab::updateChain()
{
    emit signalMsg("Try get chain name ....");
    QStringList tx_params;
    tx_params << "qt_chain.js";
    signalWalletTx(tx_params);
}
void JSWalletTab::slotUnwrap()
{
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("JSWalletTab: You must select row"); return;}
    QTableWidget *t = m_table->table();

    QStringList tx_params;
    tx_params << "qt_wrap.js";

    //prepare dialog params
    TxDialogData data(txUnwrap);
    data.token_name = t->item(row, TOKEN_COL)->text().trimmed();
    TxWrapDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        QString s_amount = data.dialog_params.value("amount");
        emit signalMsg(QString("Try send TX unwrap"));
        emit signalMsg(QString("PARAMS: token=%1  amount=%2").arg(data.token_name).arg(s_amount));
        if (s_amount == "invalid") {emit signalError(QString("invalid amount, operation canceled!")); return;}

        t->setEnabled(false);
        tx_params << TxWrapDialog::captionByTXType(data.tx_kind).trimmed().toLower();
        tx_params << data.token_name << data.dialog_params.value("amount");
        signalWalletTx(tx_params);
    }
    else emit signalMsg("operation was canceled!");
}
void JSWalletTab::slotTransfer()
{
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("JSWalletTab: You must select row"); return;}
    QTableWidget *t = m_table->table();

    QStringList tx_params;
    tx_params << "qt_transfer.js";

    //prepare dialog params
    TxDialogData data(txTransfer);
    data.token_name = t->item(row, TOKEN_COL)->text().trimmed();
    if (data.token_name != SubGraph_CommonSettings::nativeCoinByChain(chainName()))
        data.token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    TxTransferDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        QString s_amount = data.dialog_params.value("amount");
        emit signalMsg(QString("Try send TX unwrap"));
        emit signalMsg(QString("PARAMS: token=%1  amount=%2").arg(data.dialog_params.value("token")).arg(s_amount));
        if (s_amount == "invalid") {emit signalError(QString("invalid amount, operation canceled!")); return;}
        if (!data.dialog_params.contains("to_wallet")) {emit signalError(QString("invalid to_wallet address, operation canceled!")); return;}
        emit signalMsg(QString("TO_WALLET: %1").arg(data.dialog_params.value("to_wallet")));

        t->setEnabled(false);
        tx_params << data.dialog_params.value("token");
        tx_params << data.dialog_params.value("to_wallet") << s_amount;
        signalWalletTx(tx_params);
    }
    else emit signalMsg("operation was canceled!");
}
void JSWalletTab::getBalacesArgs(QStringList &nodejs_args)
{
    nodejs_args.clear();
    if (assetsCount() == 0)
    {
        emit signalError("Assets list is empty");
        return;
    }

    nodejs_args << "qt_balance.js" << QString::number(2);
}
void JSWalletTab::initNativeToken()
{
    QStringList token_data;
    token_data << SubGraph_CommonSettings::nativeTokenAddress();
    token_data << SubGraph_CommonSettings::nativeCoinByChain(chainName());
    token_data << chainName() << QString::number(18);
    addTokenToTable(token_data);
}
int JSWalletTab::assetsCount() const
{
    return m_table->table()->rowCount();
}
QMap<QString, QString> JSWalletTab::assetsTokens() const
{
    QMap<QString, QString> list;
    QTableWidget *t = m_table->table();

    int n = assetsCount();
    for (int i=0; i<n; i++)
    {
        QString t_name = t->item(i, TOKEN_COL)->text().trimmed();
        QString t_addr = t->item(i, ADDRESS_COL)->text().trimmed();
        list.insert(t_name, t_addr);
    }
    return list;
}
QMap<QString, float> JSWalletTab::assetsBalances() const
{
    QMap<QString, float> map;
    QTableWidget *t = m_table->table();

    bool ok = false;
    int n = assetsCount();
    for (int i=0; i<n; i++)
    {
        QString t_addr = t->item(i, ADDRESS_COL)->text().trimmed();
        float amount = t->item(i, BALANCES_COL)->text().trimmed().toFloat(&ok);
        if (ok) map.insert(t_addr, amount);
    }
    return map;
}
void JSWalletTab::loadAssetsFromFile()
{
    m_table->removeAllRows();

    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_ASSETS_FILE);
    emit signalMsg(QString("try load wallet assets[%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("tokens file not found");
        return;
    }

    initNativeToken();

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    foreach (const QString &v, fdata)
    {
        QString fline = v.trimmed();
        if (fline.isEmpty()) continue;
        if (fline.left(1) == "#") continue;

        QStringList token_data = LString::trimSplitList(fline, "/");
        addTokenToTable(token_data);
    }

    loadAssetIcons();
    m_table->resizeByContents();
}
*/



/*
void JSWalletTab::initHttpRequester()
{
    m_priceRequester = new LHttpApiRequester(this);
    m_priceRequester->setHttpProtocolType(hptHttps);
    m_priceRequester->addReqHeader(QString("Content-Type"), QString("application/json"));
    m_priceRequester->addReqHeader(QString("accept"), QString("application/json"));
    m_priceRequester->setApiServer("api.coingecko.com");

    connect(m_priceRequester, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_priceRequester, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_priceRequester, SIGNAL(signalFinished(int)), this, SLOT(slotHttpReqFinished(int)));
}
void JSWalletTab::addTokenToTable(const QStringList &token_data)
{
    if (token_data.count() != 4)
    {
        emit signalError(QString("token file line is invalid [%1]").arg(LString::uniteList(token_data, " / ")));
        return;
    }

    if (token_data.at(2).trimmed().toLower() == m_currentChain) //token line mast be for current chain
    {
        QTableWidget *t = m_table->table();
        QStringList row_data;
        QString t_addr = token_data.first().trimmed().toLower();
        row_data << QString::number(t->rowCount()+1) << token_data.at(1) << t_addr;
        row_data << token_data.last() << "?" << "---";
        LTable::addTableRow(t, row_data);

        t->item(t->rowCount()-1, ADDRESS_COL)->setTextColor("#4682B4");
        t->item(t->rowCount()-1, BALANCES_COL)->setTextColor(Qt::gray);
    }
}
void JSWalletTab::updateBalances(const QJsonObject &j_result)
{
    QStringList keys(j_result.keys());
    int pos = keys.indexOf("type");
    if (pos >= 0) keys.removeAt(pos);

    foreach (const QString &t_addr, keys)
    {
        QString value = j_result.value(t_addr).toString();
        updateTokenBalance(t_addr, value);
    }
    m_table->resizeByContents();

    //update BalancesHistory
    QMap<QString, float> balances(assetsBalances());
    if (!balances.isEmpty())
    {
        m_balanceHistory->updateBalances(balances, chainName());
        emit signalBalancesUpdated(chainName());
    }

    sendHttpReq();
}
void JSWalletTab::updateTokenBalance(const QString &t_addr, const QString &value)
{
    QTableWidget *t = m_table->table();
    int n = assetsCount();
    for (int i=0; i<n; i++)
    {
        QString row_addr = t->item(i, ADDRESS_COL)->text();
        if (row_addr == t_addr.trimmed().toLower())
        {
            bool ok;
            float fb = value.toFloat(&ok);
            if (!ok || fb < 0)
            {
                t->item(i, BALANCES_COL)->setText("invalid");
                t->item(i, BALANCES_COL)->setTextColor(Qt::red);
            }
            else
            {
                t->item(i, BALANCES_COL)->setText(value);
                t->item(i, BALANCES_COL)->setTextColor((fb > 0.1) ? Qt::darkGreen : Qt::gray);
            }
            break;
        }
    }
}
void JSWalletTab::parseJSResult(const QJsonObject &j_result)
{
    qDebug("JSWalletTab::parseJSResult");
    m_table->table()->setEnabled(true);

    if (j_result.keys().contains("type"))
    {
        QString operation = j_result.value("type").toString().trimmed();
        if (operation == "update") updateBalances(j_result);
        else if (operation.contains("tx_")) lookTxAnswer(j_result);
        else emit signalError(QString("invalid answer type: %1").arg(operation));
    }
    else if (j_result.keys().contains("chain"))
    {
        m_currentChain = j_result.value("chain").toString().trimmed().toLower();
        qDebug()<<QString("JSWalletTab,  m_currentChain: [%1]").arg(m_currentChain);
        loadAssetsFromFile();
        if (assetsCount() > 0) emit signalChainUpdated();
        else qWarning("JSWalletTab::parseJSResult WARNING: invalid chain, assetsCount == 0");
    }
    else
    {
        emit signalError("JSWalletTab::parseJSResult  invalid JSON response");
    }

    m_table->resizeByContents();
}
void JSWalletTab::lookTxAnswer(const QJsonObject &j_result)
{
    QString code = j_result.value("result_code").toString().trimmed();
    QString tx_kind = j_result.value("type").toString().trimmed();
    qDebug()<<QString("JSWalletTab::lookTxAnswer, tx_kind: [%1],  result code: [%2]").arg(tx_kind).arg(code);

    if (code == "OK")
    {
        QString hash = j_result.value("tx_hash").toString().trimmed();
        emit signalMsg("TX sended OK!!!");
        emit signalMsg(QString("TX_KIND: [%1],  TX_HASH: [%2]").arg(tx_kind).arg(hash));

        sendTxRecordToLog(j_result);
    }
    else
    {
        emit signalError(QString("TX sending result: %1").arg(code));
    }
}
void JSWalletTab::slotScriptBroken()
{
    m_table->table()->setEnabled(true);
}
void JSWalletTab::slotGetTokenPrice(const QString &t_addr, float &cur_price) const
{
    cur_price = -1;
    QTableWidget *t = m_table->table();
    bool ok = false;
    int n = assetsCount();
    for (int i=0; i<n; i++)
    {
        QString addr = t->item(i, ADDRESS_COL)->text().trimmed();
        if (addr == t_addr)
        {
            if (t->item(i, TOKEN_COL)->text().contains("USD")) cur_price = 1;
            else
            {
                float a = t->item(i, PRICE_COL)->text().toFloat(&ok);
                if (ok) cur_price = a;
            }
            break;
        }
    }
    qDebug()<<QString("JSWalletTab::slotGetTokenPrice  t_addr[%1]  cur_price = %2").arg(t_addr).arg(cur_price);
}
void JSWalletTab::slotGetChainName(QString &s)
{
    s = chainName();
}
float JSWalletTab::findTokenPrice(QString t) const
{
    qDebug()<<QString("JSWalletTab::findTokenPrice t=%1").arg(t);
    float p = -1;
    if (t.length() > 20)
    {
        qDebug("t.length() > 20");
        slotGetTokenPrice(t, p);
        return p;
    }


    QString t_addr = assetsTokens().value(t, QString()).trimmed();
    qDebug()<<QString("t_addr=%1").arg(t_addr);

    if (t_addr.isEmpty()) return p;
    slotGetTokenPrice(t_addr, p);
    return p;
}
void JSWalletTab::sendTxRecordToLog(const QJsonObject &j_result)
{
    QString tx_kind = j_result.value("type").toString().trimmed();
    tx_kind.remove("tx_");
    tx_kind = tx_kind.toLower().trimmed();
    JSTxLogRecord rec(tx_kind, chainName());
    if (j_result.contains("tx_hash"))
        rec.tx_hash = j_result.value("tx_hash").toString().trimmed();

    if (tx_kind.contains("wrap"))
    {
        rec.token_address = j_result.value("token").toString().trimmed();
        rec.token0_size = j_result.value("size").toString().trimmed().toFloat();
    }
    else if (tx_kind.contains("transfer"))
    {
        rec.token_address = j_result.value("token").toString().trimmed();
        rec.token0_size = j_result.value("size").toString().trimmed().toFloat();
        if (j_result.contains("to_wallet"))
            rec.note = QString("TO_WALLET[%1]").arg(j_result.value("to_wallet").toString().trimmed().toLower());
    }
    else
    {
        emit signalError(QString("JSWalletTab: invalid TX_kind - [%1]").arg(tx_kind));
        return;
    }

    qDebug("");
    qDebug()<<QString("tx_kind[%1]  token_address[%2]").arg(tx_kind).arg(rec.token_address);
    rec.current_price = findTokenPrice(rec.token_address);
    qDebug()<<QString("current_price[%1] ").arg(rec.current_price);
    rec.pool_address = "---";
    emit signalSendTxLog(rec);
}
*/


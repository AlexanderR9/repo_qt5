#include "jswallettab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"
#include "lhttp_types.h"
#include "lhttpapirequester.h"


#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>


#define JS_ASSETS_FILE      "token.txt"
#define ADDRESS_COL         2
#define TOKEN_COL           1
#define BALANCES_COL        4
#define PRICE_COL           5



// JSWalletTab
JSWalletTab::JSWalletTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL),
      m_priceRequester(NULL)
{
    setObjectName("js_wallet_tab");

    //init table
    initTable();

    // init context menu
    initPopupMenu();

    //init http obj
    initHttpRequester();
}
void JSWalletTab::loadAssetIcons()
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        QString ticker = t->item(i, TOKEN_COL)->text().trimmed();
        int pos = ticker.indexOf("(");
        if (pos > 0) ticker = ticker.left(pos).trimmed();
        QString icon_name = QString("%1.png").arg(ticker.toLower());
        QString icon_path = QString("%1/crypto/%2").arg(SubGraph_CommonSettings::commonIconsPath()).arg(icon_name);

        QTableWidgetItem *item = t->item(i, 0);
        if (item) item->setIcon(QIcon(icon_path));

        //set token_id coingecko
        QString ids = SubGraph_CommonSettings::tokenIdCoingecko(ticker);
        t->item(i, PRICE_COL)->setData(Qt::UserRole, ids);
    }
    t->setIconSize(QSize(24, 24));
}
void JSWalletTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Wrap", TxDialogBase::iconByTXType(txWrap));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Unwrap", TxDialogBase::iconByTXType(txUnwrap));
    act_list.append(pair2);
    QPair<QString, QString> pair3("Transfer", TxDialogBase::iconByTXType(txTransfer));
    act_list.append(pair3);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotWrap())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotUnwrap())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTransfer())); i_menu++;

}
void JSWalletTab::slotHttpReqFinished(int result)
{
    qDebug()<<QString("JSWalletTab::slotHttpReqFinished: REQUEST FINISHED result=%1").arg(result);
    emit signalMsg(QString("price request finished, result %1").arg(result));
    //if (!updatingRunning()) {qDebug("!updatingRunning()"); return;}

    const LHttpApiReplyData& r = m_priceRequester->lastReply();
    if (r.data.isEmpty()) emit signalMsg("WARNING: REPLY JSON IS EMPTY");
    if (r.isOk())
    {
        qDebug("parse HTTP response");
        parseHttpResponse(r.data);
        m_table->resizeByContents();

        /*
        QStringList keys(r.data.keys());
        if (keys.isEmpty()) qDebug("keys is empty");
        else
        {
            qDebug()<<QString("keys %1, first %2").arg(keys.count()).arg(keys.first());
            const QJsonValue &j_data = r.data.value(keys.first());
            if (j_data.isObject())
            {
                QJsonValue j_price = j_data.toObject().value("usd");
                if (j_price.isString()) qDebug()<<QString("IS_STRING_TYPE: %1").arg(j_price.toString());
                else if (j_price.isDouble()) qDebug()<<QString("IS_DOUBLE_TYPE: %1").arg(j_price.toDouble());

            }
            else qWarning("WARNING j_data is not jobj");

        }
*/


     //   emit signalJsonReply(userSign(), r.data);
    }
    else emit signalError("request fault");
}
void JSWalletTab::parseHttpResponse(const QJsonObject &j_obj)
{
    QStringList keys(j_obj.keys());
    if (keys.isEmpty()) {emit signalError("JSWalletTab: coingecko response is empty"); return;}

    foreach (const QString &v, keys)
    {
        qDebug()<<QString("parseHttpResponse %1").arg(v);
        const QJsonValue &j_data = j_obj.value(v);
        if (j_data.isObject())
        {
            QJsonValue j_price = j_data.toObject().value("usd");
            double price = j_price.toDouble();
            updatePriceItem(v, price);
        }
        else qWarning("WARNING j_data is not jobj");
    }

    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
        if (t->item(i, PRICE_COL)->text() == "---")
            t->item(i, PRICE_COL)->setTextColor(Qt::lightGray);
}
void JSWalletTab::updatePriceItem(QString ids, double value)
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        QString item_ids = t->item(i, PRICE_COL)->data(Qt::UserRole).toString().trimmed();
        if (item_ids == ids)
        {
            t->item(i, PRICE_COL)->setText(QString::number(value, 'f', 3));
            t->item(i, PRICE_COL)->setTextColor(Qt::darkBlue);
        }
    }
}
void JSWalletTab::sendHttpReq()
{
    qDebug("\n\n");
    qDebug("JSWalletTab::sendHttpReq() ..............");
    QString uri = "api/v3/simple/price?";
    prepareCoingeckoParams(uri);
    if (uri.isEmpty()) {signalError("JSWalletTab coingecko ids is empty"); return;}
    m_priceRequester->setUri(uri);

    qDebug("-------------------------------------------");
    emit signalMsg(QString("Send price request URL: %1").arg(m_priceRequester->fullUrl()));
    qDebug()<<m_priceRequester->metadata();
//    emit signalEnableControls(false);
    m_priceRequester->start(hrmGet);
}
void JSWalletTab::prepareCoingeckoParams(QString &uri)
{
    QStringList id_tokens;
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        QString ticker = t->item(i, TOKEN_COL)->text().trimmed();
        if (SubGraph_CommonSettings::isStableToken(ticker)) continue;
        //if (SubGraph_CommonSettings::isWrapedToken(ticker)) ticker = LString::strTrimLeft(ticker, 1);

        QString id = SubGraph_CommonSettings::tokenIdCoingecko(ticker).trimmed();
        if (!id.isEmpty() && !id_tokens.contains(id)) id_tokens.append(id);
    }
    if (id_tokens.isEmpty()) {uri.clear(); return;}

    uri+=QString("ids=");
    foreach (const QString &v, id_tokens)
        uri = QString("%1%2,").arg(uri).arg(v);
    uri = LString::strTrimRight(uri, 1);

    uri+=QString("&vs_currencies=usd");
}
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
    if (data.token_name != SubGraph_CommonSettings::nativeTokenByChain("polygon"))
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
    token_data << SubGraph_CommonSettings::nativeTokenByChain("polygon");
    token_data << "polygon" << QString::number(18);

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
void JSWalletTab::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Assets list");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Number" << "Token" << "Address" << "Decimal" << "Amount" << "Price";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");

    v_splitter->addWidget(m_table);
}
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

    QTableWidget *t = m_table->table();
    QStringList row_data;
    QString t_addr = token_data.first().trimmed().toLower();
    row_data << QString::number(t->rowCount()+1) << token_data.at(1) << t_addr;
    row_data << token_data.last() << "?" << "---";
    LTable::addTableRow(t, row_data);

    t->item(t->rowCount()-1, ADDRESS_COL)->setTextColor("#4682B4");
    t->item(t->rowCount()-1, BALANCES_COL)->setTextColor(Qt::gray);
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

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "update") updateBalances(j_result);
    else if (operation.contains("tx_")) lookTxAnswer(j_result);
    else emit signalError(QString("invalid answer type: %1").arg(operation));

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


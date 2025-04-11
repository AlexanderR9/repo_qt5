#include "jswallettab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>


#define JS_ASSETS_FILE      "token.txt"
#define ADDRESS_COL         2
#define TOKEN_COL           1
#define BALANCES_COL        4

// JSWalletTab
JSWalletTab::JSWalletTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
{
    setObjectName("js_wallet_tab");

    //init table
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Assets list");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Number" << "Token" << "Address" << "Decimal" << "Amount";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");

    v_splitter->addWidget(m_table);
    initPopupMenu();
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

    m_table->resizeByContents();
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
    row_data << token_data.last() << "?";
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


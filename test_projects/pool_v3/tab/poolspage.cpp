#include "poolspage.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "txdialog.h"
#include "txlogrecord.h"
#include "nodejsbridge.h"
#include "lstring.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>


#define ADDRESS_COL             2
#define PAIR_COL                0
#define FEE_COL                 1
#define TICK_COL                3
#define PRICE_COL               4
#define TVL_COL                 5


// DefiPoolsTabPage
DefiPoolsTabPage::DefiPoolsTabPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkPool)
{
    setObjectName("defi_pools_tab_page");

    //init table
    initTable();

    // init popup
    initPopupMenu();
}
void DefiPoolsTabPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Supplied amounts information");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Tokens" << "Fee" << "Address" << "Tick" << "Price" << "TVL (kUSDT)";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");

    h_splitter->addWidget(m_table);
    m_table->resizeByContents();
}
void DefiPoolsTabPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);
    initPoolList(cid);
}
void DefiPoolsTabPage::initPoolList(int cid)
{
    QTableWidget *t = m_table->table();
    m_table->removeAllRows();
    QStringList row_data;
    int last_row = 0;
    foreach (const DefiPoolV3 &v, defi_config.pools)
    {
        if (v.chain_id == cid)
        {
            row_data.clear();
            row_data << v.name << v.strFloatFee() << v.address << "-" << "-" << "-";
            LTable::addTableRow(t, row_data);

            if (v.is_stable) LTable::setTableTextRowColor(t, last_row, "#D2691E");
            last_row++;
        }
    }
    m_table->resizeByContents();
}
void DefiPoolsTabPage::initPopupMenu()
{
    QString path = AppCommonSettings::commonIconsPath(); // icons path

    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state", QString("%1/view-refresh.svg").arg(path));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Swap tokens", TxDialogBase::iconByTXType(txSwap));
    act_list.append(pair2);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetPoolState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTxSwap())); i_menu++;
}
void DefiPoolsTabPage::slotGetPoolState()
{
    qDebug("DefiPoolsTabPage::slotGetPoolState()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiPoolsTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPoolState));
    QString pool_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    int pos = defi_config.getPoolIndex(pool_addr);
    if (pos < 0) {emit signalError(QString("not found pool[%1] in defi_config").arg(pool_addr)); return;}

    j_params.insert("pool_address", pool_addr);
    j_params.insert("token0_address", defi_config.pools.at(pos).token0_addr);
    j_params.insert("token1_address", defi_config.pools.at(pos).token1_addr);
    j_params.insert("fee", QString::number(defi_config.pools.at(pos).fee));

    sendReadNodejsRequest(j_params);
}
bool DefiPoolsTabPage::hasBalances() const
{
    QTableWidget *t = m_table->table();
    int row = m_table->curSelectedRow();
    if (row < 0) return false;
    QStringList token_names = LString::trimSplitList(t->item(row, PAIR_COL)->text().trimmed(), "/");
    if (token_names.count() != 2) return false;

    float a = 0;
    float b = 0;
    emit signalGetTokenBalance(token_names.first(), a);
    emit signalGetTokenBalance(token_names.last(), b);

    return (a >= 0 && b >= 0);
}
int DefiPoolsTabPage::findRowByPool(const QString &pool_addr) const
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    if (n_rows <= 0) return -1;

    for (int i=0; i<n_rows; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == pool_addr)
            return i;
    }
    return -1;
}
bool DefiPoolsTabPage::poolStateUpdated() const
{
    QTableWidget *t = m_table->table();
    int row = m_table->curSelectedRow();
    if (row < 0) return false;

    bool ok = false;
    float p = t->item(row, PRICE_COL)->text().trimmed().toFloat(&ok);

    return (ok && p > 0);
}
void DefiPoolsTabPage::slotTxSwap()
{
    qDebug("DefiPoolsTabPage::slotTxSwap()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiPoolsTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    if (!hasBalances()) {emit signalError("DefiPoolsTabPage: Balances must updated"); return;}
    if (!poolStateUpdated()) {emit signalError("DefiPoolsTabPage: pool state must updated"); return;}


    TxDialogData data(txSwap, curChainName());
    data.pool_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    data.dialog_params.insert("pair", t->item(row, PAIR_COL)->text().trimmed());
    data.dialog_params.insert("fee", t->item(row, FEE_COL)->text().trimmed());

    //request pair balances
    QStringList token_names = LString::trimSplitList(t->item(row, PAIR_COL)->text().trimmed(), "/");
    float b_t0 = 0;
    float b_t1 = 0;
    emit signalGetTokenBalance(token_names.first(), b_t0);
    emit signalGetTokenBalance(token_names.last(), b_t1);
    data.dialog_params.insert("balance0", QString::number(b_t0));
    data.dialog_params.insert("balance1", QString::number(b_t1));

    //request cur price in pool
    data.dialog_params.insert("price", t->item(row, PRICE_COL)->text().trimmed());
    data.dialog_params.insert("price_index", QString::number(defi_config.getPoolTokenPriceIndex(t->item(row, PAIR_COL)->text())));


    TxSwapDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        //qDebug("next swap");
        if (!data.dialog_params.contains("error"))
        {
            QString pool_addr = t->item(row, ADDRESS_COL)->text().trimmed();
            int pos = defi_config.getPoolIndex(pool_addr);
            if (pos < 0) {emit signalError(QString("not found pool[%1] in defi_config").arg(pool_addr)); return;}

            data.dialog_params.insert("pool_address", pool_addr);
            data.dialog_params.insert("token0_address", defi_config.pools.at(pos).token0_addr);
            data.dialog_params.insert("token1_address", defi_config.pools.at(pos).token1_addr);
            data.dialog_params.insert("fee", QString::number(defi_config.pools.at(pos).fee));
        }
        sendTxNodejsRequest(data);
    }
    else emit signalMsg("operation was canceled!");
}
void DefiPoolsTabPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiPoolsTabPage::slotNodejsReply - req kind: [%1]").arg(req);


    if (req == NodejsBridge::jsonCommandValue(nrcPoolState)) updatePoolStateRow(js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txSwap)) checkTxResult(req, js_reply);
    else return;

    m_table->resizeByContents();
}
void DefiPoolsTabPage::updatePoolStateRow(const QJsonObject &js_reply)
{
    qDebug("DefiPoolsTabPage::updatePoolStateRow");
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();


    QString p_addr = js_reply.value("pool_address").toString().trimmed();
    for (int i=0; i<n_rows; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == p_addr)
        {
            int pos = defi_config.getPoolIndex(p_addr);
            if (pos < 0) {emit signalError(QString("not found pool[%1] in defi_config").arg(p_addr)); continue;}

            //update tick
            t->item(i, TICK_COL)->setText(js_reply.value("tick").toString());

            //update price
            QString pair = t->item(i, PAIR_COL)->text().trimmed();
            int price_index = defi_config.getPoolTokenPriceIndex(pair);
            float price = ((price_index == 0) ? js_reply.value("price0").toString().toFloat() : js_reply.value("price1").toString().toFloat());
            int pp = defi_config.pools.at(pos).is_stable ? 8 : AppCommonSettings::interfacePricePrecision(price);
            t->item(i, PRICE_COL)->setText(QString::number(price, 'f', pp));

            //update tvl
            float tvl0 = js_reply.value("tvl0").toString().toFloat();
            float tvl1 = js_reply.value("tvl1").toString().toFloat();
            int t0_pos = defi_config.getTokenIndex(defi_config.pools.at(pos).token0_addr, defi_config.getChainID(curChainName()));
            int t1_pos = defi_config.getTokenIndex(defi_config.pools.at(pos).token1_addr, defi_config.getChainID(curChainName()));
            if (t0_pos >= 0 && t1_pos >= 0)
            {
                float t0_price = defi_config.tokens.at(t0_pos).is_stable ? 1 : defi_config.tokens.at(t0_pos).last_price;
                float t1_price = defi_config.tokens.at(t1_pos).is_stable ? 1 : defi_config.tokens.at(t1_pos).last_price;
                if (t0_price > 0 && t1_price > 0)
                {
                    float tvl = tvl0*t0_price + tvl1*t1_price;
                    tvl /= float(1000);
                    t->item(i, TVL_COL)->setText(QString::number(tvl, 'f', 1));
                }
                else t->item(i, TVL_COL)->setText(QString("%1/%2").arg(tvl0).arg(tvl1));
            }
            else t->item(i, TVL_COL)->setText(QString("%1/%2").arg(tvl0).arg(tvl1));

            break;
        }
    }
}
void DefiPoolsTabPage::checkTxResult(QString req, const QJsonObject &js_reply)
{
    qDebug("DefiPoolsTabPage::checkTxResult(QString req");
    bool is_simulate = (js_reply.value(AppCommonSettings::nodejsTxSimulateFieldName()).toString() == "yes");
    emit signalMsg("");
    emit signalMsg(QString("//////////// REPLY form TX request [%1] ///////////////").arg(req));
    int result_code = js_reply.value("code").toString().toInt();
    emit signalMsg(QString("simulate_mode = %1").arg(is_simulate?"YES":"NO"));
    emit signalMsg(QString("result_code = %1").arg(result_code));

    if (is_simulate)
    {
        emit signalMsg(QString("input_amount = %1").arg(js_reply.value("input_amount").toString()));
        emit signalMsg(QString("out_amount = %1").arg(js_reply.value("out_amount").toString()));
        emit signalMsg(QString("estimated_gas = %1").arg(js_reply.value("estimated_gas").toString()));
    }
    else if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        logTxRecord(req, js_reply);
    }
}
void DefiPoolsTabPage::logTxRecord(QString req, const QJsonObject &js_reply)
{
    qDebug("DefiPoolsTabPage::logTxRecord");

    TxLogRecord tx_rec(req, curChainName());
    tx_rec.tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    tx_rec.pool.pool_addr = js_reply.value("pool_address").toString().trimmed();
    tx_rec.pool.token_in = js_reply.value("tokenIn").toString().trimmed();
    tx_rec.pool.token_sizes.first = js_reply.value("input_amount").toString().toFloat();

    qDebug()<<QString("tx_hash[%1]").arg(tx_rec.tx_hash);
    QTableWidget *t = m_table->table();
    int row = findRowByPool(tx_rec.pool.pool_addr);
    if (row < 0)
    {
        qWarning()<<QString("DefiPoolsTabPage::logTxRecord WARNING selected row(%1) < 0").arg(row);
        signalError(QString("DefiPoolsTabPage: can't find row by pool[%1]").arg(tx_rec.pool.pool_addr));
        return;
    }
    tx_rec.pool.price = t->item(row, PRICE_COL)->text().trimmed().toFloat();

    QString extra_data = "?";
    int t_index = js_reply.value("input_index").toString().toInt();
    QStringList token_names = LString::trimSplitList(t->item(row, PAIR_COL)->text().trimmed(), "/");
    if (token_names.count() == 2)
    {
        if (t_index == 0) extra_data = QString("%1 => %2").arg(token_names.first()).arg(token_names.last());
        else if (t_index == 1) extra_data = QString("%1 => %2").arg(token_names.last()).arg(token_names.first());
        else extra_data = "?????";
    }
    tx_rec.formNote(extra_data);
    //  - swap: pool_addr[0x_addr]; token_in[0x_addr]; token_amount[value]; current_price[value] (адрес пула в котором меняем, входной токен, который отдаем и сколько отдаем, текущая цена)

    qDebug()<<QString("DefiPoolsTabPage::logTxRecord   emit signalNewTx(tx_rec);  tx_rec.note[%1]").arg(tx_rec.note);
    emit signalNewTx(tx_rec);
}





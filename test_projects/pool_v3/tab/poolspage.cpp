#include "poolspage.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "txdialog.h"
#include "txlogrecord.h"
#include "nodejsbridge.h"


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




/*
void DefiApproveTabPage::updatePageBack(QString extra_data)
{
    qDebug()<<QString("DefiApproveTabPage::updatePageBack  extra_data[%1]").arg(extra_data);

    m_table->table()->clearSelection();
    selectRowByCellData(extra_data.trimmed(), ADDRESS_COL);
    slotGetApproved();
}
*/
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
void DefiPoolsTabPage::slotTxSwap()
{
    qDebug("DefiPoolsTabPage::slotTxSwap()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiPoolsTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    /*
    TxDialogData data(txApprove, curChainName());
    data.token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    TxApproveDialog d(data, this);

    d.exec();
    if (d.isApply()) sendTxNodejsRequest(data);
    else emit signalMsg("operation was canceled!");
    */
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
            t->item(i, TICK_COL)->setText(js_reply.value("tick").toString());

            //update price
            QString pair = t->item(i, PAIR_COL)->text().trimmed();
            int price_index = defi_config.getPoolTokenPriceIndex(pair);
            float price = ((price_index == 0) ? js_reply.value("price0").toString().toFloat() : js_reply.value("price1").toString().toFloat());
            t->item(i, PRICE_COL)->setText(QString::number(price, 'f', AppCommonSettings::interfacePricePrecision(price)));

            //update tvl
            int pos = defi_config.getPoolIndex(p_addr);
            if (pos < 0) {emit signalError(QString("not found pool[%1] in defi_config").arg(p_addr)); continue;}
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
    /*
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
    */
}
/*
void DefiApproveTabPage::logTxRecord(QString req, const QJsonObject &js_reply)
{
    TxLogRecord tx_rec(req, curChainName());
    tx_rec.tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    tx_rec.wallet.token_addr = js_reply.value("token_address").toString().trimmed();
    tx_rec.wallet.token_amount = js_reply.value("amount").toString().toFloat();
    tx_rec.wallet.contract_addr = js_reply.value("to_contract").toString();
    //tx_rec.note = QString("approve %1").arg(tickerByAddress(tx_rec.wallet.token_addr));
    tx_rec.formNote(tickerByAddress(tx_rec.wallet.token_addr));


    emit signalNewTx(tx_rec);
}
QString DefiApproveTabPage::tickerByAddress(const QString &t_addr) const
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
*/



#include "positionspage.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "lfile.h"
#include "deficonfig.h"
#include "txdialog.h"
#include "txlogrecord.h"
#include "postxworker.h"



#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>


#define POOL_COL             1
#define RANGE_COL            2
#define PRICE_COL            3
#define ASSETS_COL           4
#define REWARDS_COL          5
#define LIQ_COL              6

#define IN_RANGE_COLOR      QString("#AAFBAA")
#define OUT_RANGE_COLOR     QString("#FFCCCC")


// DefiPositionsPage
DefiPositionsPage::DefiPositionsPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkPositions),
      m_txWorker(NULL)
{
    setObjectName("defi_poositions_tab_page");
    m_positions.clear();

    //init table
    initTable();

    // init popup
    initPopupMenu();


}
bool DefiPositionsPage::hasBalances() const
{
    QString t_name;
    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id == m_txWorker->chainId())
        {
            t_name = v.name;
            break;
        }
    }

    float amount = -1;
    emit signalGetTokenBalance(t_name, amount);
    qDebug()<<QString("DefiPositionsPage::hasBalances()  t_name[%1] cid[%2] amount=%3 ").arg(t_name).arg(m_txWorker->chainId()).arg(amount);
    return (amount >= 0);
}
void DefiPositionsPage::mintPos()
{
    qDebug("DefiPositionsPage::mintPos()");
    //if (!hasBalances()) {emit signalError("You must update wallet balances"); return;}

    m_txWorker->tryTx(txMint, m_positions);
}
void DefiPositionsPage::updatePageBack(QString extra_data)
{
    qDebug("=========DefiPositionsPage::updatePageBack=========");
    Q_UNUSED(extra_data);
    m_table->table()->clearSelection();

    int tx = m_txWorker->lastTx();
    if (tx == txBurn)
    {
        sendUpdateDataRequest();
    }
    else if (tx == txCollect)
    {
        selectRowByCellData(QString::number(m_txWorker->lastPosPid()), 0);
        slotGetSelectedPosState();
    }
    else if (tx == txIncrease || tx == txDecrease || tx == txTakeaway)
    {
        m_txWorker->setReupdatePage(true);
        sendUpdateDataRequest();
    }
    else if (tx == txMint)
    {
        sendUpdateDataRequest();
    }
}
void DefiPositionsPage::initPopupMenu()
{
    QString path = AppCommonSettings::commonIconsPath(); // icons path


    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state (selected)", QString("%1/view-refresh.svg").arg(path));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Get state (with liquidity)", QString("%1/edit-undo.svg").arg(path));
    act_list.append(pair2);
    QPair<QString, QString> pair3("Get state (none liquidity)", QString("%1/edit-redo.svg").arg(path));
    act_list.append(pair3);
    QPair<QString, QString> pair5("Collect rewards", TxDialogBase::iconByTXType(txCollect));
    act_list.append(pair5);
    QPair<QString, QString> pair6("Decrease liquidity", TxDialogBase::iconByTXType(txDecrease));
    act_list.append(pair6);
    QPair<QString, QString> pair7("Take away assets", TxDialogBase::iconByTXType(txTakeaway));
    act_list.append(pair7);
    QPair<QString, QString> pair4("Burn positions (selected)", TxDialogBase::iconByTXType(txBurn));
    act_list.append(pair4);
    QPair<QString, QString> pair8("Increase liquidity", TxDialogBase::iconByTXType(txIncrease));
    act_list.append(pair8);


    //init popup menu actions
    m_table->popupMenuActivate(act_list);
    m_table->setMaxPopupSelectRows(8);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetSelectedPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetLiquidityPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetNoneLiqPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotCollectPosSelected())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotDecreasePosSelected())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTakeawayPosSelected())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotBurnPosSelected())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotIncreasePosSelected())); i_menu++;

}
void DefiPositionsPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Wallet positions list");
    m_table->vHeaderHide();

    QStringList headers;
    headers << "PID" << "Pool" << "Price range" << "Current price" << "Current assets" << "Rewards" << "Liquidity";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_table->setSelectionColor("#BDFF4F");
    h_splitter->addWidget(m_table);

    /////////////////////////////////////////

    m_integratedTable = new LTableWidgetBox(this);
    m_integratedTable->setTitle("Integrated state");
    m_integratedTable->setObjectName("positions_integrated_table");
    headers.clear();
    headers << "Value";
    LTable::setTableHeaders(m_integratedTable->table(), headers);
    headers.clear();
    headers << "Total positions" << "With liquidity" << "Minted positions" <<  "Burned positions" << "Locked sum (USDT)" << "Rewards sum (USDT)";
    LTable::setTableHeaders(m_integratedTable->table(), headers, Qt::Vertical);
    m_integratedTable->setSelectionMode(QAbstractItemView::NoSelection, QAbstractItemView::NoSelection);
    LTable::createAllItems(m_integratedTable->table());
    h_splitter->addWidget(m_integratedTable);

}
void DefiPositionsPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();

    m_txWorker = new PosTxWorker(this, cid, m_table->table());
    connect(m_txWorker, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_txWorker, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_txWorker, SIGNAL(signalGetPosIndexByPid(int, int&)), this, SLOT(slotSetPosIndexByPid(int, int&)));
    connect(m_txWorker, SIGNAL(signalSendTx(const TxDialogData&)), this, SLOT(slotSendTx(const TxDialogData&)));
    connect(m_txWorker, SIGNAL(signalNewTx(const TxLogRecord&)), this, SIGNAL(signalNewTx(const TxLogRecord&)));
    connect(m_txWorker, SIGNAL(signalGetTokenBalance(QString, float&)), this, SIGNAL(signalGetTokenBalance(QString, float&)));
    connect(m_txWorker, SIGNAL(signalTryUpdatePoolState(const QString&)), this, SLOT(slotGetPoolState(const QString&)));

}
void DefiPositionsPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiPositionsPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcPositions))
    {
        updatePositionsData(js_reply);
        if (m_txWorker->needReupdatePage())
        {
            m_txWorker->setReupdatePage(false);
            int tx = m_txWorker->lastTx();
            if (tx == txDecrease || tx == txIncrease || tx == txTakeaway)
            {
                selectRowByCellData(QString::number(m_txWorker->lastPosPid()), 0);
                slotGetSelectedPosState();
            }
        }
    }
    else if (req == NodejsBridge::jsonCommandValue(nrcPosState)) updatePositionsState(js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txBurn)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txCollect)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txDecrease)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txTakeaway)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txMint)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txIncrease)) checkTxResult(req, js_reply);
    else if (req == NodejsBridge::jsonCommandValue(nrcPoolState)) getPoolStateFromPoolPage(js_reply);
    else return;

    updateIntegratedTable();

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();
}
void DefiPositionsPage::getPoolStateFromPoolPage(const QJsonObject &js_reply)
{
    qDebug("DefiPositionsPage::getPoolStateFromPoolPage 1");
    if (!m_txWorker->mintDialogActivated()) return;

    qDebug("DefiPositionsPage::getPoolStateFromPoolPage 2");
    QString p_addr = js_reply.value("pool_address").toString().trimmed();
    QStringList p_state;
    emit signalGetPoolStateFromPoolPage(p_addr, p_state);

    m_txWorker->poolStateReceived(p_state);
}
void DefiPositionsPage::sendUpdateDataRequest()
{
    //prepare json params
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPositions));
    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::updatePositionsState(const QJsonObject &js_reply)
{
    bool ok;
    QStringList keys = js_reply.keys();
    qDebug("received keys:");
    foreach (const QString &v, keys)
    {
         qDebug() << v;
         int pid = v.toInt(&ok);
         if (!ok || pid <= 0) continue;

         int i_pos = posIndexOf(pid);
         if (i_pos >= 0) updatePositionState(i_pos, js_reply.value(v));
    }        
}
void DefiPositionsPage::updatePositionState(int i, const QJsonValue &js_value)
{
    if (i<0 || i>=posCount()) return;
    if (!js_value.isObject()) return;

    QJsonObject j_obj = js_value.toObject();
    if (!j_obj.isEmpty()) m_positions[i].state.update(j_obj);

    //update row table
    const DefiPosition &rec = m_positions.at(i);
    QTableWidget *t = m_table->table();
    t->item(i, RANGE_COL)->setText(rec.interfacePriceRange());
    t->item(i, PRICE_COL)->setText(rec.interfaceCurrentPrice());
    t->item(i, ASSETS_COL)->setText(rec.interfaceAssetAmounts());
    t->item(i, REWARDS_COL)->setText(rec.interfaceRewards());

    t->item(i, PRICE_COL)->setToolTip(QString("tick: %1").arg(rec.state.pool_tick));
    t->item(i, PRICE_COL)->setData(Qt::UserRole, rec.state.price0);
    t->item(i, ASSETS_COL)->setToolTip(rec.interfaceAssetAmountsDesired());
    t->item(i, REWARDS_COL)->setToolTip(rec.interfaceRewardsDesired());

    if (rec.hasLiquidity())
    {
        if (rec.isOutRange())  LTable::setTableRowColor(t, i, OUT_RANGE_COLOR);
        else LTable::setTableRowColor(t, i, IN_RANGE_COLOR);
    }
}
void DefiPositionsPage::updatePositionsData(const QJsonObject &js_reply)
{
    m_positions.clear();
    m_table->removeAllRows();

    qDebug("--DefiPositionsPage::updatePositionsData--");
    bool result = false;
    if (js_reply.contains("result")) result = (js_reply.value("result").toString().trimmed().toLower() == "true");
    int n_pos = js_reply.value("pos_count").toString().trimmed().toInt();

    QString s = QString("Getting positions data result: %1").arg(result ? "OK" : "FAULT");
    if (!result) emit signalError(s);
    else
    {
        emit signalMsg(s);
        emit signalMsg(QString("Current positions: %1").arg(n_pos));
        if (n_pos > 0)
        {
            //need read posdata file
            readNodejsPosFile();
            reloadTableByRecords();
        }
        else emit signalError("positions data is empty");
    }
}
void DefiPositionsPage::readNodejsPosFile()
{
    QString fname = QString("%1/%2").arg(AppCommonSettings::nodejsPath()).arg(AppCommonSettings::positionsListFile());
    if (!LFile::fileExists(fname))
    {
        emit signalError(QString("DefiPositionsPage: file [%1] not found").arg(fname));
        return;
    }

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty())
    {
        emit signalError(QString("DefiPositionsPage: %1").arg(err));
        return;
    }
    if (fdata.isEmpty())
    {
        emit signalError(QString("DefiPositionsPage: positions data is empty"));
        return;
    }

    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;

        DefiPosition   rec;
        rec.fromFileLine(fline);
        if (!rec.invalid())
        {
            rec.calcTIndex(curChainName());
            if (rec.hasLiquidity()) m_positions.insert(0, rec);
            else m_positions.append(rec);
            qDebug()<<rec.toStr();
        }
        else qWarning()<<QString("DefiPositionsPage: WARNING invalid fline_pos: %1").arg(fline);
    }
}
void DefiPositionsPage::reloadTableByRecords()
{
    qDebug()<<QString("DefiPositionsPage::reloadTableByRecords()  pos_count=%1").arg(posCount());
    if (!hasPos()) return;

    QTableWidget *t = m_table->table();
    QStringList row_data;
    int n = posCount();
    for (int i=0; i<n; i++)
    {
        const DefiPosition &rec = m_positions.at(i);

        row_data.clear();
        row_data << QString::number(rec.pid) << poolInfo(rec);
        row_data << QString("-") << QString("-") << QString("-") << QString("-") << rec.liq;
        LTable::addTableRow(t, row_data);
        t->item(i, 0)->setTextColor(Qt::darkGreen);

        if (!rec.hasLiquidity())
            LTable::setTableTextRowColor(t, i, Qt::lightGray);

        int pool_i = defi_config.getPoolIndexByPosition(rec);
        if (pool_i >= 0) t->item(i, POOL_COL)->setToolTip(defi_config.pools.at(pool_i).address);
        else t->item(i, POOL_COL)->setToolTip("WARNING: pool not found");

        t->item(i, RANGE_COL)->setToolTip(rec.interfaceTickRange());
        //t->item(i, RANGE_COL)->setData(Qt::UserRole, rec.interfaceTickRange());
    }

    m_table->resizeByContents();
}
QString DefiPositionsPage::poolInfo(const DefiPosition &rec) const
{
    if (rec.invalid()) return "???";

    int cid = defi_config.getChainID(curChainName());
    if (cid < 0) return "???";

    QString s = QString("%1/%2").arg(defi_config.tokenNameByAddress(rec.token_addrs.first, cid)).arg(defi_config.tokenNameByAddress(rec.token_addrs.second, cid));
    s = QString("%1 (%2)").arg(s).arg(rec.interfaceFee());
    return s;
}
void DefiPositionsPage::slotGetSelectedPosState()
{
    qDebug("DefiPositionsPage::slotGetSelectedPosState()");
    QTableWidget *t = m_table->table();
    QList<int> list = LTable::selectedRows(t);
    if (list.isEmpty()) {emit signalError("DefiPositionsPage: You must select several positions"); return;}

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPosState));
    QJsonArray j_arr_pid;
    QJsonArray j_arr_pool;
    foreach (int row, list)
    {
        QString s_pid = t->item(row, 0)->text().trimmed();
        j_arr_pid.push_back(s_pid);
        QString pool_addr = t->item(row, POOL_COL)->toolTip().trimmed();
        j_arr_pool.push_back(pool_addr);
    }
    j_params.insert("pid_arr", j_arr_pid);
    j_params.insert("pool_addresses", j_arr_pool);

    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::slotGetLiquidityPosState()
{
    qDebug("DefiPositionsPage::slotGetLiquidityPosState()");
    if (m_positions.isEmpty()) return;

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPosState));
    QJsonArray j_arr_pid;
    QJsonArray j_arr_pool;

    QTableWidget *t = m_table->table();
    int n_row = t->rowCount();
    for (int i=0; i<n_row; i++)
    {
        QString s_liq = t->item(i, LIQ_COL)->text().trimmed();
        if (s_liq.length() < 3) continue; // pos has not liq

        j_arr_pid.push_back(t->item(i, 0)->text().trimmed());
        j_arr_pool.push_back(t->item(i, POOL_COL)->toolTip().trimmed());
    }

    j_params.insert("pid_arr", j_arr_pid);
    j_params.insert("pool_addresses", j_arr_pool);
    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::slotGetNoneLiqPosState()
{
    qDebug("DefiPositionsPage::slotGetNoneLiqPosState()");
    if (m_positions.isEmpty()) return;

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPosState));
    QJsonArray j_arr_pid;
    QJsonArray j_arr_pool;

    QTableWidget *t = m_table->table();
    int n_row = t->rowCount();
    for (int i=0; i<n_row; i++)
    {
        QString s_liq = t->item(i, LIQ_COL)->text().trimmed();
        if (s_liq != QString("0")) continue; // pos has liq

        j_arr_pid.push_back(t->item(i, 0)->text().trimmed());
        j_arr_pool.push_back(t->item(i, POOL_COL)->toolTip().trimmed());
    }

    j_params.insert("pid_arr", j_arr_pid);
    j_params.insert("pool_addresses", j_arr_pool);
    sendReadNodejsRequest(j_params);
}
int DefiPositionsPage::posIndexOf(int pid) const
{
    if (m_positions.isEmpty()) return -1;

    int n = posCount();
    for (int i=0; i<n; i++)
        if (m_positions.at(i).pid == pid) return i;

    return -1;
}
void DefiPositionsPage::updateIntegratedTable()
{
    if (m_positions.isEmpty()) return;

    QTableWidget *t = m_integratedTable->table();
    t->item(0, 0)->setText(QString::number(m_positions.count()));

    int n_liq = 0;
    float locked = -1;
    float reward = -1;
    foreach (const DefiPosition &v, m_positions)
    {
        if (v.hasLiquidity()) n_liq++;
        if (!v.state.invalid())
        {
            if (locked < 0) locked = 0;
            if (reward < 0) reward = 0;
            reward += v.rewardSum();
            locked += v.lockedSum();
        }
    }
    t->item(1, 0)->setText(QString::number(n_liq));
    t->item(2, 0)->setText("?");
    t->item(3, 0)->setText("?");
    t->item(4, 0)->setText(QString::number(locked, 'f', 1));
    t->item(5, 0)->setText(QString::number(reward, 'f', 2));

    // set colors
    if (n_liq > 0) t->item(1, 0)->setTextColor(Qt::darkGreen);
    if (reward > 9.9) t->item(5, 0)->setTextColor(Qt::blue);
    else t->item(5, 0)->setTextColor(Qt::lightGray);


}
void DefiPositionsPage::checkTxResult(QString req, const QJsonObject &js_reply)
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

        if (m_txWorker->lastTx() == txMint)
        {
            m_txWorker->emulMintReply(js_reply);
        }
        else
        {
            QString extra_data = m_txWorker->extraDataLastTx(js_reply);
            if (extra_data.contains("invalid")) emit signalError(extra_data);
            emit signalMsg(extra_data);
        }
    }
    else if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        m_txWorker->prepareTxLog(js_reply, m_positions);
    }
}
void DefiPositionsPage::slotGetPoolState(const QString &pool_addr)
{
    qDebug()<<QString("DefiPoolsTabPage::slotGetPoolState() pool_addr[%1]").arg(pool_addr);

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPoolState));
    int pos = defi_config.getPoolIndex(pool_addr);
    if (pos < 0)
    {
        qWarning()<<QString("DefiPositionsPage: WARNING  not found pool[%1] in defi_config").arg(pool_addr);
        emit signalError(QString("not found pool[%1] in defi_config").arg(pool_addr));
        return;
    }

    j_params.insert("pool_address", pool_addr);
    j_params.insert("token0_address", defi_config.pools.at(pos).token0_addr);
    j_params.insert("token1_address", defi_config.pools.at(pos).token1_addr);
    j_params.insert("fee", QString::number(defi_config.pools.at(pos).fee));

    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::slotSetOpenedPosState(QMap<int, QStringList> &map)
{
    map.clear();

    foreach (const DefiPosition &v, m_positions)
    {
        if (!v.hasLiquidity()) continue;
        if (v.state.invalid()) continue;

        int p_index = defi_config.getPoolIndexByPosition(v);
        if (p_index < 0) continue;

        // список заполняется данными по одной позе (у которой есть ликвидность).
        // 6 элементов: pool_addr / tick_range / cur_price(by prior_index) / cur_assets / cur_rewards / 'out' or 'active' range
        QStringList pos_data;
        pos_data.append(defi_config.pools.at(p_index).address);
        pos_data.append(v.interfaceTickRange());
        pos_data.append(v.interfaceCurrentPrice());
        pos_data.append(v.interfaceAssetAmounts());
        pos_data.append(v.interfaceRewards());
        if (v.isOutRange()) pos_data.append("out");
        else pos_data.append("active");

        map.insert(v.pid, pos_data);
    }
}




///////////////////////TX slots/////////////////////////////////////
void DefiPositionsPage::slotCollectPosSelected()
{
    m_txWorker->tryTx(txCollect, m_positions);
}
void DefiPositionsPage::slotBurnPosSelected()
{
    m_txWorker->tryTx(txBurn, m_positions);
}
void DefiPositionsPage::slotDecreasePosSelected()
{
    m_txWorker->tryTx(txDecrease, m_positions);
}
void DefiPositionsPage::slotIncreasePosSelected()
{
    m_txWorker->tryTx(txIncrease, m_positions);
}
void DefiPositionsPage::slotTakeawayPosSelected()
{
    m_txWorker->tryTx(txTakeaway, m_positions);
}
void DefiPositionsPage::slotSendTx(const TxDialogData &data)
{
    sendTxNodejsRequest(data);
}






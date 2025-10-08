#include "positionspage.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "lfile.h"
#include "deficonfig.h"
#include "txdialog.h"
#include "txlogrecord.h"



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
    :BaseTabPage_V3(parent, 20, dpkPositions)
{
    setObjectName("defi_poositions_tab_page");
    m_positions.clear();

    //init table
    initTable();

    // init popup
    initPopupMenu();


}
void DefiPositionsPage::updatePageBack(QString extra_data)
{
    qDebug("=========DefiPositionsPage::updatePageBack=========");
    Q_UNUSED(extra_data);
    sendUpdateDataRequest();
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
    QPair<QString, QString> pair4("Burn positions (selected)", QString("%1/emblem-cancel.svg").arg(path));
    act_list.append(pair4);


    //init popup menu actions
    m_table->popupMenuActivate(act_list);
    m_table->setMaxPopupSelectRows(8);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetSelectedPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetLiquidityPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetNoneLiqPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotBurnPosSelected())); i_menu++;


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
   // initPoolList(cid);

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();

}
void DefiPositionsPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiPositionsPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcPositions)) updatePositionsData(js_reply);
    else if (req == NodejsBridge::jsonCommandValue(nrcPosState)) updatePositionsState(js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txBurn)) checkTxResult(req, js_reply);
    else return;

    updateIntegratedTable();

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();
}
void DefiPositionsPage::sendUpdateDataRequest()
{
    qDebug("DefiPositionsPage::sendUpdateDataRequest()");
    //prepare json params
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPositions));
    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::updatePositionsState(const QJsonObject &js_reply)
{
    qDebug("--DefiPositionsPage::updatePositionsState--");
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
void DefiPositionsPage::slotBurnPosSelected()
{
    qDebug("DefiPositionsPage::slotBurnPosSelected()");
    QTableWidget *t = m_table->table();
    QList<int> list = LTable::selectedRows(t);
    if (list.isEmpty()) {emit signalError("DefiPositionsPage: You must select several positions"); return;}

    //check selected pos state
    bool ok;
    QString err;
    QList<int> pids;
    foreach (int row, list)
    {
        int pid = t->item(row, 0)->text().trimmed().toInt(&ok);
        if (!ok) {err = QString("invalid convert PID(%1) to integer").arg(t->item(row, 0)->text()); break;}

        int j = posIndexOf(pid);
        if (j < 0) {err = QString("can't find PID(%1) to positions container").arg(pid); break;}

        if (m_positions.at(j).state.invalid()) {err = QString("you must update state for position PID(%1)").arg(pid); break;}
        if (m_positions.at(j).hasLiquidity()) {err = QString("position PID(%1) has liquidity").arg(pid); break;}
        if (m_positions.at(j).rewardSum() > 0) {err = QString("position PID(%1) has rewards > 0").arg(pid); break;}

        pids.append(pid);
    }
    if (!err.isEmpty()) {emit signalError(err); return;}

    // init TX dialog
    TxDialogData data(txBurn, curChainName());
    for(int i=0; i<pids.count(); i++)
        data.dialog_params.insert(QString("pid%1").arg(i+1), QString::number(pids.at(i)));


    TxBurnPosDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        sendTxNodejsRequest(data);
    }
    else emit signalMsg("operation was canceled!");
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
        QString extra_data;
        if (js_reply.value("pid_arr").isArray())
        {
            QJsonArray j_pid_arr = js_reply.value("pid_arr").toArray();
            for (int i=0; i<j_pid_arr.count(); i++)
            {
                QString s_pid = j_pid_arr.at(i).toString().trimmed();
                if (extra_data.isEmpty()) extra_data = s_pid;
                else extra_data = QString("%1:%2").arg(extra_data).arg(s_pid);
            }

            emit signalMsg(QString("pid_arr: %1,  count=%2").arg(extra_data).arg(j_pid_arr.count()));
        }
        else emit signalError("can't get j_pid_arr from js_reply");

    }
    else if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        logTxRecord(req, js_reply);
    }
}
void DefiPositionsPage::logTxRecord(QString req, const QJsonObject &js_reply)
{
    TxLogRecord tx_rec(req, curChainName());
    tx_rec.tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();

    QString extra_data;
    if (req == NodejsBridge::jsonCommandValue(txBurn))
    {
        if (js_reply.value("pid_arr").isArray())
        {
            QJsonArray j_pid_arr = js_reply.value("pid_arr").toArray();
            for (int i=0; i<j_pid_arr.count(); i++)
            {
                QString s_pid = j_pid_arr.at(i).toString().trimmed();
                if (extra_data.isEmpty()) extra_data = s_pid;
                else extra_data = QString("%1:%2").arg(extra_data).arg(s_pid);
            }
            emit signalMsg(QString("pid_arr: %1").arg(extra_data));
        }
        else
        {
            extra_data = "???";
            emit signalError("can't get j_pid_arr from js_reply");
        }
    }
    tx_rec.formNote(extra_data);
    emit signalNewTx(tx_rec);
}




#include "jsposmanagertab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"
#include "ethers_js.h"
#include "jstxlogger.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>

#define POOL_COL            1
#define P_RANGE_COL         2
#define T_RANGE_COL         3
#define STATE_COL           5
#define LIQ_COL             7
#define REWARD_COL          6
#define ASSETS_COL          4



// JSPosManagerTab
JSPosManagerTab::JSPosManagerTab(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_tablePos(NULL),
      m_tableLog(NULL)
{
    setObjectName("js_posmanager_tab");

    //init tables
    initTables();

    // init context menu
    initPopupMenu();

}
void JSPosManagerTab::initTables()
{
    m_tablePos = new LSearchTableWidgetBox(this);
    m_tablePos->setTitle("Positions list");
    m_tablePos->vHeaderHide();
    QStringList headers;
    headers << "PID" << "Pool" << "Price range" << "Tick range" << "Assets amount" << "State" << "Unclaimed fees" << "Liquidity";
    LTable::setTableHeaders(m_tablePos->table(), headers);
    m_tablePos->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_tablePos->setSelectionColor("#87CEEB");
    h_splitter->addWidget(m_tablePos);
    m_tablePos->searchExec();

    m_tablePos->sortingOn();
    m_tablePos->addSortingData(LIQ_COL, LTableWidgetBox::sdtNumeric);
    m_tablePos->addSortingData(POOL_COL, LTableWidgetBox::sdtString);


    m_tableLog = new LSearchTableWidgetBox(this);
    m_tableLog->setTitle("Pool log (closed)");
    m_tableLog->vHeaderHide();
    headers.clear();
    headers << "Pool" << "Opened Date/Time" << "Closed Date/Time" << "PID" << "Price range" << "Deposited" << "Claimed" << "Rewards";
    LTable::setTableHeaders(m_tableLog->table(), headers);
    m_tableLog->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_tableLog->setSelectionColor("#87CEEB");
    h_splitter->addWidget(m_tableLog);
    m_tableLog->searchExec();

}
void JSPosManagerTab::updatePidList()
{
    QStringList params;
    m_tableLog->removeAllRows();
    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);

    params << "qt_pidlist.js";
    emit signalPosManagerAction(params);
}
void JSPosManagerTab::parseJSResult(const QJsonObject &j_result)
{
    qDebug()<<LString::symbolString('-');
    qDebug("JSPosManagerTab::parseJSResult");
    qDebug() << j_result;

    emit signalMsg("\n\n\n");
    m_tablePos->setEnabled(true);
    m_tableLog->setEnabled(true);

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "pid_list") jsonPidListReceived(j_result);
    else if (operation == "pos_file_data") jsonPosFileDataReceived(j_result);
    else if (operation == "pos_short_info") jsonPosShortInfoReceived(j_result);
    else if (operation == "pos_state") jsonPosStateReceived(j_result);
    else if (operation == "decrease") jsonTxDecreaseReceived(j_result);
    else if (operation == "increase") jsonTxIncreaseReceived(j_result);
    else if (operation == "collect") jsonTxCollectReceived(j_result);

    m_tablePos->searchExec();
    m_tableLog->searchExec();
}
void JSPosManagerTab::slotScriptBroken()
{
    m_tablePos->setEnabled(true);
    m_tableLog->setEnabled(true);
    m_tablePos->resizeByContents();
    m_tableLog->resizeByContents();
}
void JSPosManagerTab::reloadPidListToTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tablePos->table();
    int n = j_arr.size();
    int n_cols = t->columnCount();
    qDebug()<<QString("JSPosManagerTab::reloadPidListToTable   j_arr.size() %1,  n_cols=%2").arg(n).arg(n_cols);

    for (int i=0; i<n; i++)
    {
        QStringList row_data;
        if (j_arr.at(i).isDouble())
        {
            double pid = j_arr.at(i).toDouble();
            row_data << QString::number(int(pid));
        }
        else if (j_arr.at(i).isString()) row_data << j_arr.at(i).toString();
        else row_data << "invalid_type";

        for (int j=0; j<(n_cols-1); j++) row_data << "-";
        LTable::addTableRow(t, row_data);
        t->item(i, 0)->setTextColor("#6B8E23");
    }
}
void JSPosManagerTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state", QString("%1/r_scale.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1);
    QPair<QString, QString> pair1_1("Get short info", QString("%1/ball_yellow.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1_1);
    QPair<QString, QString> pair2("Increase liquidity", TxDialogBase::iconByTXType(txIncrease));
    act_list.append(pair2);
    QPair<QString, QString> pair3("Decrease liquidity", TxDialogBase::iconByTXType(txDecrease));
    act_list.append(pair3);
    QPair<QString, QString> pair4("Collect tokens", TxDialogBase::iconByTXType(txCollect));
    act_list.append(pair4);

    //init popup menu actions
    m_tablePos->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_tablePos->connectSlotToPopupAction(i_menu, this, SLOT(slotGetPositionState())); i_menu++;
    m_tablePos->connectSlotToPopupAction(i_menu, this, SLOT(slotGetShortInfo())); i_menu++;
    m_tablePos->connectSlotToPopupAction(i_menu, this, SLOT(slotTryIncreaseLiquidity())); i_menu++;
    m_tablePos->connectSlotToPopupAction(i_menu, this, SLOT(slotTryDecreaseLiquidity())); i_menu++;
    m_tablePos->connectSlotToPopupAction(i_menu, this, SLOT(slotTryCollectTokens())); i_menu++;

}
void JSPosManagerTab::slotGetPositionState()
{
   // qDebug("JSPoolTab::slotGetPoolState()");
    int row = m_tablePos->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);

    QStringList params;
    QString pid = m_tablePos->table()->item(row, 0)->text().trimmed();
    QString pool_addr = m_tablePos->table()->item(row, POOL_COL)->data(Qt::UserRole).toString().trimmed();
    params << "qt_posstate.js" << pid << pool_addr;
    emit signalPosManagerAction(params);
}
void JSPosManagerTab::slotGetShortInfo()
{
    int row = m_tablePos->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);


    QStringList params;
    QString pid = m_tablePos->table()->item(row, 0)->text().trimmed();
    params << "qt_show_posdata.js" << pid;
    emit signalPosManagerAction(params);
}
void JSPosManagerTab::slotTryIncreaseLiquidity()
{
    qDebug("JSPosManagerTab::slotTryIncreaseLiquidity()");
    int row = m_tablePos->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_tablePos->table();
    QString cur_state = t->item(row, STATE_COL)->text().trimmed();
    if (cur_state == "-") {emit signalError("You must update state"); return;}

    //prepare dialog params
    TxDialogData increase_data(txIncrease);
    increase_data.token_addr = t->item(row, 0)->text();
    increase_data.token_name = t->item(row, POOL_COL)->text();
    increase_data.dialog_params.insert("current_price", cur_state);
    increase_data.dialog_params.insert("price_range", t->item(row, P_RANGE_COL)->text().trimmed());
    increase_data.dialog_params.insert("tick_range", t->item(row, T_RANGE_COL)->text().trimmed());
    increase_data.dialog_params.insert("approved", "--- to do ----");

    //exec dialog
    TxIncreaseLiqDialog d(increase_data, this);
    d.exec();
    if (d.isApply()) sendIncreaseTx(increase_data, row);
    else emit signalMsg("operation was canceled!");

}
void JSPosManagerTab::slotTryDecreaseLiquidity()
{
    qDebug("JSPosManagerTab::slotTryDecreaseLiquidity()");
    int row = m_tablePos->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_tablePos->table();
    QString cur_state = t->item(row, STATE_COL)->text().trimmed();
    if (cur_state == "-") {emit signalError("You must update state"); return;}
    //cur_state = QString("%1   UNCLAIMED(%2)").arg(cur_state).arg(t->item(row, REWARD_COL)->text());
    QString cur_assets = t->item(row, ASSETS_COL)->text();

    QString s_liq = t->item(row, LIQ_COL)->text().trimmed();
    if (s_liq == "0") {emit signalError("Liqudity is empty in this pos"); return;}


    //prepare dialog params
    TxDialogData decrease_data(txDecrease);
    decrease_data.token_addr = t->item(row, 0)->text();
    decrease_data.token_name = t->item(row, POOL_COL)->text();
    decrease_data.dialog_params.insert("state", cur_state);
    decrease_data.dialog_params.insert("assets", cur_assets);
    decrease_data.dialog_params.insert("range", t->item(row, P_RANGE_COL)->text().trimmed());
    decrease_data.dialog_params.insert("reward", t->item(row, REWARD_COL)->text().trimmed());
    //exec dialog
    TxRemoveLiqDialog d(decrease_data, this);
    d.exec();
    if (d.isApply()) sendDecreaseTx(decrease_data, row);
    else emit signalMsg("operation was canceled!");
}
void JSPosManagerTab::slotTryCollectTokens()
{
    qDebug("JSPosManagerTab::slotTryCollectTokens()");
    int row = m_tablePos->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_tablePos->table();
    QString cur_state = t->item(row, STATE_COL)->text().trimmed();
    if (cur_state == "-") {emit signalError("You must update state"); return;}
    QStringList s_rewards = LString::trimSplitList(t->item(row, REWARD_COL)->text().trimmed(), "/");
    if (s_rewards.count() != 2)  {emit signalError("You must update rewards value"); return;}
    bool ok;
    bool has_rewards = false;
    float fee0 = s_rewards.at(0).trimmed().toFloat(&ok);
    if (ok && fee0 > 0) has_rewards = true;
    float fee1 = s_rewards.at(1).trimmed().toFloat(&ok);
    if (ok && fee1 > 0) has_rewards = true;
    if (!has_rewards ) {emit signalError("rewards is null, not need collect"); return;}

    //prepare dialog params
    TxDialogData collect_data(txCollect);
    collect_data.token_addr = t->item(row, 0)->text();
    collect_data.token_name = t->item(row, POOL_COL)->text();
  //  collect_data.dialog_params.insert("state", t->item(row, REWARD_COL)->text().trimmed());
//    collect_data.dialog_params.insert("range", t->item(row, P_RANGE_COL)->text().trimmed());
    collect_data.dialog_params.insert("reward", t->item(row, REWARD_COL)->text().trimmed());
    collect_data.dialog_params.insert("assets", t->item(row, ASSETS_COL)->text().trimmed());

    //exec dialog
    TxRemoveLiqDialog d(collect_data, this);
    d.exec();
    if (d.isApply()) sendCollectTx(collect_data, row);
    else emit signalMsg("operation was canceled!");
}
void JSPosManagerTab::updateCurrentPrices()
{

}
void JSPosManagerTab::sendIncreaseTx(const TxDialogData &increase_data, int row)
{
    emit signalMsg(QString("Try increase from position, PID=%1 ...").arg(increase_data.token_addr));

    //prepare json params
    QJsonObject j_params;
    j_params.insert("simulate_mode", increase_data.dialog_params.value("simulate_mode"));
    j_params.insert("pid", increase_data.token_addr);
    j_params.insert("pool_address", m_tablePos->table()->item(row, POOL_COL)->data(Qt::UserRole).toString().trimmed());
    j_params.insert("tx_kind", QString("increase"));

    j_params.insert("l_tick", increase_data.dialog_params.value("l_tick").toInt());
    j_params.insert("h_tick", increase_data.dialog_params.value("h_tick").toInt());
    j_params.insert("token0_amount", increase_data.dialog_params.value("token0_amount").toDouble());
    j_params.insert("token1_amount", increase_data.dialog_params.value("token1_amount").toDouble());
    j_params.insert("dead_line", increase_data.dialog_params.value("dead_line").toInt());

    emit signalRewriteParamJson(j_params); //rewrite json-file

    //send TX to node_js script
    sendTx("increase", row);
}
void JSPosManagerTab::sendDecreaseTx(const TxDialogData &decrease_data, int row)
{
    emit signalMsg(QString("Try decrease from position, PID=%1 ...").arg(decrease_data.token_addr));

    //prepare json params
    QJsonObject j_params;
    j_params.insert("pid", decrease_data.token_addr);
    j_params.insert("pool_address", m_tablePos->table()->item(row, POOL_COL)->data(Qt::UserRole).toString().trimmed());
    j_params.insert("dead_line", int(45));
    j_params.insert("simulate_mode", decrease_data.dialog_params.value("simulate_mode"));
    j_params.insert("liq_size", m_tablePos->table()->item(row, LIQ_COL)->text().trimmed());
    j_params.insert("tx_kind", QString("decrease"));
    emit signalRewriteParamJson(j_params); //rewrite json-file

    //send TX to node_js script
    sendTx("decrease", row);
}
void JSPosManagerTab::sendCollectTx(const TxDialogData &collect_data, int row)
{
    emit signalMsg(QString("Try collect unclaimed fees from position, PID=%1 ...").arg(collect_data.token_addr));

    //prepare json params
    QJsonObject j_params;
    j_params.insert("simulate_mode", collect_data.dialog_params.value("simulate_mode"));
    j_params.insert("pid", collect_data.token_addr);
    j_params.insert("dead_line", int(45));
    j_params.insert("pool_address", m_tablePos->table()->item(row, POOL_COL)->data(Qt::UserRole).toString().trimmed());
    j_params.insert("tx_kind", QString("collect"));
    emit signalRewriteParamJson(j_params); //rewrite json-file

    //send TX to node_js script
    sendTx("collect", row);
}
void JSPosManagerTab::sendTx(QString cmd, int row)
{
    QTableWidget *t = m_tablePos->table();
    t->item(row, STATE_COL)->setText(QString("try %1 ...").arg(cmd));
    t->item(row, STATE_COL)->setTextColor("#DC143C");
    m_tablePos->resizeByContents();
    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);

    //invoke node_js script
    QStringList tx_params;
    tx_params << "qt_pos_tx.js" << EthersPage::inputParamsJsonFile();
    emit signalPosManagerAction(tx_params);
}
void JSPosManagerTab::rereadJSPosFileData()
{
    emit signalMsg("\n ##### JSPosManagerTab::rereadJSPosFileData() ####");
    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);

    QStringList params;
    params << "qt_show_posdata.js";
    emit signalPosManagerAction(params);
}
void JSPosManagerTab::sendTxRecordToLog(int row, const QJsonObject &j_result)
{
    qDebug("");
    qDebug("JSPosManagerTab::sendTxRecordToLog");

    QString tx_kind = j_result.value("type").toString().trimmed().toLower();
    QString cur_chain;
    emit signalGetChainName(cur_chain);

    QTableWidget *t = m_tablePos->table();
    JSTxLogRecord rec(tx_kind, cur_chain);
    rec.tx_hash = j_result.value("tx_hash").toString().trimmed();
    rec.pid = j_result.value("pid").toString().trimmed().toUInt();
    rec.pool_address = j_result.value("pool_address").toString().trimmed();
    rec.current_price = j_result.value("current_price").toString().trimmed().toFloat();
    rec.current_tick = j_result.value("pool_tick").toString().trimmed().toInt();
    rec.price_range = cellRangeToLogFormat(t->item(row, P_RANGE_COL)->text());
    rec.tick_range = cellRangeToLogFormat(t->item(row, T_RANGE_COL)->text());

    if (tx_kind == "decrease")
    {
        tokenSizesToLogValues(t->item(row, ASSETS_COL)->text(), rec);
    }
    else if (tx_kind == "collect")
    {
        tokenSizesToLogValues(t->item(row, REWARD_COL)->text(), rec);
    }
    else if (tx_kind == "increase")
    {
        rec.token0_size = j_result.value("token0_amount").toString().trimmed().toFloat();
        rec.token1_size = j_result.value("token1_amount").toString().trimmed().toFloat();
    }


    rec.note = QString::number(rec.token0_size, 'f', SubGraph_CommonSettings::interfacePrecision(rec.token0_size));
    rec.note = QString("%1%2").arg(rec.note).arg(rowTokenName(row, 0));
    rec.note = QString("%1:%2%3").arg(rec.note).arg(QString::number(rec.token1_size, 'f', SubGraph_CommonSettings::interfacePrecision(rec.token1_size))).arg(rowTokenName(row, 1));
    rec.note = QString("%1_AMOUNTS[%2]").arg(tx_kind.toUpper()).arg(rec.note);

    rec.token_address = "---";
    emit signalSendTxLog(rec);
}
QString JSPosManagerTab::cellRangeToLogFormat(QString s) const
{
    QString s_range = s.trimmed();
    s_range.remove("(");
    s_range.remove(")");
    QStringList plist = LString::trimSplitList(s_range, ";");
    if (plist.count() != 2) return QString("???");

    return QString("%1:%2").arg(plist.at(0).trimmed()).arg(plist.at(1).trimmed());
}
void JSPosManagerTab::tokenSizesToLogValues(QString s, JSTxLogRecord &rec) const
{
    rec.token0_size = rec.token1_size = -1;
    QString s_sizes = s.trimmed();
    s_sizes = LString::removeSpaces(s_sizes);
    QStringList list = LString::trimSplitList(s_sizes, "/");
    if (list.count() != 2) return;

    bool ok;
    float a = list.at(0).toFloat(&ok);
    if (ok && a>=0) rec.token0_size = a;
    a = list.at(1).toFloat(&ok);
    if (ok && a>=0) rec.token1_size = a;
}
QString JSPosManagerTab::rowTokenName(int row, quint8 t_index) const
{
    QTableWidget *t = m_tablePos->table();
    if (row >= t->rowCount() || row < 0) return QString("?");
    if (t_index > 1) return QString("?");

    QString s = t->item(row, POOL_COL)->text().trimmed();
    int pos = s.indexOf("(");
    if (pos < 0) return QString("?");

    s = s.left(pos).trimmed();
    s = LString::removeSpaces(s);
    QStringList list = LString::trimSplitList(s, "/");
    if (list.count() != 2) return QString("?");

    if (t_index == 0) return list.at(0);
    return list.at(1);
}



//reseived node_js result
void JSPosManagerTab::jsonPidListReceived(const QJsonObject &j_result)
{
    emit signalMsg("JSON_ANSWER RECEIVED: PidList of potitions");
    m_tablePos->removeAllRows();
    int pos_count = j_result.value("pos_count").toString().trimmed().toInt();
    emit signalMsg(QString("GETTED_POS_COUNT: %1").arg(pos_count));
    QJsonValue j_list = j_result.value("pids");
    if (!j_list.isArray()) {emit signalError("JSON field <pids> is not ARRAY"); return;}

    QJsonArray j_arr = j_list.toArray();
    emit signalMsg(QString("QJsonArray size: %1").arg(j_arr.size()));
    if (j_arr.size() == 0) {emit signalError("<pids> ARRAY is empty"); return;}
    reloadPidListToTable(j_arr);

    //next req, get pos_data from file
    rereadJSPosFileData();
}
void JSPosManagerTab::jsonPosStateReceived(const QJsonObject &j_result)
{
    emit signalMsg("JSON_ANSWER RECEIVED: state of potition");

    qDebug("JSPosManagerTab::jsonPosStateReceived");
    int row = m_tablePos->curSelectedRow();
    QTableWidget *t = m_tablePos->table();

    t->item(row, ASSETS_COL)->setText(j_result.value("assets").toString());
    t->item(row, REWARD_COL)->setText(j_result.value("reward").toString());

    //QString s_state = QString("unclaimed(%1)").arg(j_result.value("reward").toString());
    QString p_location = j_result.value("price_location").toString().toUpper().trimmed();
    QString s_state = QString("P=%1 (%2)").arg(j_result.value("price_current").toString()).arg(p_location);
    s_state = QString("Tick=%1  %2").arg(j_result.value("current_tick").toString().toInt()).arg(s_state);
    QString s_liq = j_result.value("liq").toString().trimmed();
    emit signalMsg(QString("STATE[%1]  LIQ[%2]").arg(s_state).arg(s_liq));

    t->item(row, LIQ_COL)->setText(s_liq);
    if (s_liq == "0") t->item(row, LIQ_COL)->setTextColor(Qt::lightGray);
    else t->item(row, LIQ_COL)->setTextColor(Qt::blue);


    LTable::setTableRowColor(t, row, Qt::white);
    if (s_liq.length() > 4)
    {
        if (p_location.contains("OUT")) LTable::setTableRowColor(t, row, "#FFDCDC");
        else LTable::setTableRowColor(t, row, "#BAFBBA");
    }
    else if (!p_location.contains("OUT"))
    {
        //s_state = QString("P=%1 (IN_RANGE)").arg(j_result.value("price_current").toString());
        s_state.replace("ACTIVE", "IN_RANGE");
    }
    t->item(row, STATE_COL)->setText(s_state);
    t->item(row, STATE_COL)->setTextColor(Qt::black);



    ///////////////////////check need reread fdata///////////////////////////////////
    if (t->item(row, POOL_COL)->text() == "-")
    {
        //next req, get pos_data from file
        rereadJSPosFileData();
    }
}
void JSPosManagerTab::jsonPosShortInfoReceived(const QJsonObject &j_result)
{
    emit signalMsg("JSON_ANSWER RECEIVED: short_info of potition");

    qDebug("JSPosManagerTab::jsonPosShortInfoReceived");
    int row = m_tablePos->curSelectedRow();
    QTableWidget *t = m_tablePos->table();

    QString s_liq = j_result.value("liq").toString().trimmed();
    t->item(row, LIQ_COL)->setText(s_liq);
    if (s_liq == "0") t->item(row, LIQ_COL)->setTextColor(Qt::lightGray);
    else t->item(row, LIQ_COL)->setTextColor(Qt::blue);


    QString pool_info = j_result.value("pool_info").toString().trimmed();
    QString pool_addr = j_result.value("pool_address").toString().trimmed();
    t->item(row, POOL_COL)->setText(pool_info);
    t->item(row, POOL_COL)->setData(Qt::UserRole, pool_addr);
    t->item(row, POOL_COL)->setToolTip(pool_addr);
}
void JSPosManagerTab::jsonTxDecreaseReceived(const QJsonObject &j_result)
{
    qDebug("JSPosManagerTab::jsonTxDecreaseReceived");
    emit signalMsg("JSON_ANSWER RECEIVED: decrease TX");
    int row = m_tablePos->curSelectedRow();
    QTableWidget *t = m_tablePos->table();
    emit signalMsg(QString("PID: %1").arg(j_result.value("pid").toString()));

    bool is_simulate = (j_result.value("is_simulate").toString().trimmed().toLower() == "true");
    QString tx_hash("IS_SIMULATE_MODE");
    if (!is_simulate)
    {
        tx_hash = j_result.value("tx_hash").toString().trimmed().toLower();
        //t->item(row, ASSETS_COL)->setText("?");
        //t->item(row, REWARD_COL)->setText("?");
        t->item(row, LIQ_COL)->setTextColor("#DC143C");
        emit signalMsg(QString("TX_HASH: %1").arg(tx_hash));
        t->item(row, LIQ_COL)->setText(tx_hash);
        t->item(row, STATE_COL)->setText("-");

        sendTxRecordToLog(row, j_result);
    }
    else
    {
        t->item(row, STATE_COL)->setText("DECREASE_SIMULATE_MODE");
        emit signalMsg(QString("IS_SIMULATE_MODE"));
    }
}
void JSPosManagerTab::jsonPosFileDataReceived(const QJsonObject &j_result)
{
    qDebug("JSPosManagerTab::jsonPosFileDataReceived");
    QStringList keys(j_result.keys());

    QTableWidget *t = m_tablePos->table();
    int n_row = t->rowCount();
    if (n_row == 0) {emit signalError("table of positions is empty for sync pod_file_data"); return;}

    for (int i=0; i<n_row; i++)
    {
        QString pid = t->item(i, 0)->text().trimmed();
        if (!keys.contains(pid)) continue;

        QJsonValue jv = j_result.value(pid);
        QStringList pos_data = LString::trimSplitList(jv.toString().trimmed(), "/");
        if (pos_data.count() != 5) {qWarning()<<QString("invalid posdata fline [%1]").arg(jv.toString()); continue;}


        QStringList pool_info = LString::trimSplitList(pos_data.at(1), ";");
        if (pool_info.count() == 3)
            pos_data[1] = QString("%1/%2  (%3)").arg(pool_info.at(0).trimmed()).arg(pool_info.at(1).trimmed()).arg(pool_info.at(2).trimmed());
        t->item(i, POOL_COL)->setText(pos_data.at(1));
        t->item(i, POOL_COL)->setData(Qt::UserRole, pos_data.first());
        t->item(i, POOL_COL)->setToolTip(pos_data.first());

        t->item(i, P_RANGE_COL)->setText(pos_data[2].remove("price_range"));
        t->item(i, T_RANGE_COL)->setText(pos_data[3].remove("tick_range"));

        QString liq = pos_data.last().trimmed();
        t->item(i, LIQ_COL)->setText(liq);
        if (liq == '0') t->item(i, LIQ_COL)->setTextColor(Qt::lightGray);
        else t->item(i, LIQ_COL)->setTextColor(Qt::blue);

        t->item(i, STATE_COL)->setTextColor(Qt::black);
    }

    updateCurrentPrices();
}
void JSPosManagerTab::jsonTxCollectReceived(const QJsonObject &j_result)
{
    qDebug("JSPosManagerTab::jsonTxCollectReceived");
    emit signalMsg("JSON_ANSWER RECEIVED: collect TX");
    int row = m_tablePos->curSelectedRow();
    QTableWidget *t = m_tablePos->table();
    emit signalMsg(QString("PID: %1").arg(j_result.value("pid").toString()));

    bool is_simulate = (j_result.value("is_simulate").toString().trimmed().toLower() == "true");
    QString tx_hash("IS_SIMULATE_MODE");
    if (!is_simulate)
    {
        tx_hash = j_result.value("tx_hash").toString().trimmed().toLower();
        //t->item(row, ASSETS_COL)->setText("?");
        //t->item(row, REWARD_COL)->setText("?");
        t->item(row, LIQ_COL)->setTextColor("#DC143C");
        emit signalMsg(QString("TX_HASH: %1").arg(tx_hash));
        t->item(row, LIQ_COL)->setText(tx_hash);
        t->item(row, STATE_COL)->setText("-");

        sendTxRecordToLog(row, j_result);
    }
    else
    {
        t->item(row, STATE_COL)->setText("COLLECT_SIMULATE_MODE");
        emit signalMsg(QString("IS_SIMULATE_MODE"));
    }
}
void JSPosManagerTab::jsonTxIncreaseReceived(const QJsonObject &j_result)
{
    qDebug("JSPosManagerTab::jsonTxIncreaseReceived");
    emit signalMsg("JSON_ANSWER RECEIVED: increase TX");
    int row = m_tablePos->curSelectedRow();
    QTableWidget *t = m_tablePos->table();
    emit signalMsg(QString("PID: %1").arg(j_result.value("pid").toString()));
    emit signalMsg(QString("RANGE: [%1] / [%2]").arg(t->item(row, P_RANGE_COL)->text()).arg(t->item(row, T_RANGE_COL)->text()));
    QString s_amount = j_result.value("token0_amount").toString().trimmed();
    s_amount = QString("%1 / %2").arg(s_amount).arg(j_result.value("token1_amount").toString().trimmed());
    emit signalMsg(QString("AMOUNTS: %1").arg(s_amount));


    bool is_simulate = (j_result.value("is_simulate").toString().trimmed().toLower() == "true");
    QString tx_hash("IS_SIMULATE_MODE");
    if (!is_simulate)
    {
        tx_hash = j_result.value("tx_hash").toString().trimmed().toLower();
        t->item(row, ASSETS_COL)->setText("?");
        t->item(row, REWARD_COL)->setText("?");
        t->item(row, LIQ_COL)->setTextColor("#DC143C");
        emit signalMsg(QString("TX_HASH: %1").arg(tx_hash));
        t->item(row, LIQ_COL)->setText(tx_hash);
        t->item(row, STATE_COL)->setText("-");

        sendTxRecordToLog(row, j_result);
    }
    else
    {
        t->item(row, STATE_COL)->setText("INCREASE_SIMULATE_MODE");
        emit signalMsg(QString("IS_SIMULATE_MODE"));
    }
}


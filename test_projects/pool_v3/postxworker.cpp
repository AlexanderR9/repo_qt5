#include "postxworker.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "deficonfig.h"
#include "txdialog.h"
#include "txlogrecord.h"
#include "defiposition.h"
#include "defimintdialog.h"



#include <QTableWidget>
#include <QWidget>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>


#define POOL_COL             1
#define RANGE_COL            2
#define PRICE_COL            3
#define ASSETS_COL           4
#define REWARDS_COL          5
#define LIQ_COL              6


//PosTxWorker
PosTxWorker::PosTxWorker(QObject *parent, int cid, QTableWidget *t)
    :LSimpleObject(parent),
    m_table(t),
    m_mintDialog(NULL)
{
    setObjectName("pos_tx_worker");
    m_userSign = cid;

    m_lastTx.reset();
}
void PosTxWorker::tryTx(int tx_kind, const QList<DefiPosition> &pos_data)
{
    if (!m_table) {emit signalError(QString("PosTxWorker: positions table is NULL")); return;}

    m_lastTx.reset();
    m_lastTx.tx_kind = tx_kind;
    switch (tx_kind)
    {
        case txBurn:        {burnSelected(pos_data); break;}
        case txCollect:     {collectSelected(pos_data); break;}
        case txDecrease:    {decreaseSelected(pos_data); break;}
        case txTakeaway:    {takeawaySelected(pos_data); break;}
        case txMint:        {mintPos(); break;}

        default:
        {
            emit signalError(QString("PosTxWorker: invalid tx_kind code (%1)").arg(tx_kind));
            m_lastTx.tx_kind = -1;
            break;
        }
    }
}
QWidget* PosTxWorker::parentWidget() const
{
    return qobject_cast<QWidget*>(parent());
}
QString PosTxWorker::chainName() const
{
    int i = defi_config.chainIndexOf(chainId());
    if (i < 0) return "??";
    return defi_config.chains.at(i).name;
}
QString PosTxWorker::extraDataLastTx(const QJsonObject &js_reply) const
{
    QString extra_data;
    switch (m_lastTx.tx_kind)
    {
        case txBurn:
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
            }
            else extra_data = "BURN: invalid js_reply";
            return extra_data;
        }
        case txCollect:
        {
            if (!m_lastTx.invalid())
            {
                extra_data = m_table->item(m_lastTx.t_row, POOL_COL)->text();
                extra_data = QString("%1  REWARDS(%2)").arg(extra_data).arg(m_table->item(m_lastTx.t_row, REWARDS_COL)->text());
            }
            else extra_data = QString("COLLECT: invalid PID(%1), can't find in table").arg(m_lastTx.pid);
            return extra_data;
        }
        case txDecrease:
        {
            if (!m_lastTx.invalid())
            {
                extra_data = m_table->item(m_lastTx.t_row, POOL_COL)->text();
                extra_data = QString("%1  ASSETS(%2)").arg(extra_data).arg(m_table->item(m_lastTx.t_row, ASSETS_COL)->text());
                extra_data = QString("%1  LIQ(%2)").arg(extra_data).arg(m_table->item(m_lastTx.t_row, LIQ_COL)->text());
            }
            else extra_data = QString("DECREASE: invalid PID(%1), can't find in table").arg(m_lastTx.pid);
            return extra_data;
        }
        case txTakeaway:
        {
            if (!m_lastTx.invalid())
            {
                extra_data = m_table->item(m_lastTx.t_row, POOL_COL)->text();
                extra_data = QString("%1  ASSETS(%2)").arg(extra_data).arg(m_table->item(m_lastTx.t_row, ASSETS_COL)->text());
                extra_data = QString("%1  REWARDS(%2)").arg(extra_data).arg(m_table->item(m_lastTx.t_row, REWARDS_COL)->text());
            }
            else extra_data = QString("TAKE_AWAY: invalid PID(%1), can't find in table").arg(m_lastTx.pid);
            return extra_data;
        }
        case txMint:
        {
            QString pool_addr = js_reply.value("pool_address").toString().trimmed();
            int pool_index = defi_config.getPoolIndex(pool_addr);
            if (pool_index >= 0)
            {
                QString t0_name = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token0_addr, chainId());
                QString t1_name = defi_config.tokenNameByAddress(defi_config.pools.at(pool_index).token1_addr, chainId());
                extra_data = QString("%1/%2 (%3)").arg(t0_name).arg(t1_name).arg(defi_config.pools.at(pool_index).strFloatFee());
            }
            else extra_data = QString("MINT: invalid pool_addr(%1), can't find in defi_config").arg(pool_addr);
            return extra_data;
        }
        default: break;
    }
    return "invalid lastTx code";
}
void PosTxWorker::prepareTxLog(const QJsonObject &js_reply, const QList<DefiPosition> &pos_data)
{
    if (m_lastTx.invalid()) {emit signalError("PosTxWorker: can't prepare  log record, lastTx < 0"); return;}

    TxLogRecord tx_rec(NodejsBridge::jsonCommandValue(lastTx()), chainName());
    tx_rec.tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    if (m_lastTx.pid > 0)
    {
        tx_rec.pool.pid = m_lastTx.pid;
        int j = -2;
        emit signalGetPosIndexByPid(tx_rec.pool.pid , j);
        if (j < 0)
        {
            emit signalError(QString("can't find PID(%1) to positions container").arg(tx_rec.pool.pid));
            m_lastTx.reset();
            return;
        }
        fillTxLogRecord(tx_rec, js_reply, pos_data.at(j));
    }
    else if (lastTx() == txMint)
    {
        fillTxMintLogRecord(tx_rec, js_reply);
    }

    QString extra_data = extraDataLastTx(js_reply);
    extra_data.replace(QChar('/'), QChar('|'));
    tx_rec.formNote(extra_data);

    emit signalNewTx(tx_rec);
}
void PosTxWorker::fillTxMintLogRecord(TxLogRecord &rec, const QJsonObject &js_reply)
{
    rec.pool.pool_addr = js_reply.value("pool_address").toString().trimmed();
    rec.pool.price = js_reply.value("pool_price").toString().toFloat();
    rec.pool.tick = js_reply.value("pool_tick").toString().toInt();
    rec.pool.token_sizes.first = js_reply.value("amount0").toString().toFloat();
    rec.pool.token_sizes.second = js_reply.value("amount1").toString().toFloat();
    rec.pool.price_range.first = js_reply.value("p1").toString().toFloat();
    rec.pool.price_range.second = js_reply.value("p2").toString().toFloat();
    rec.pool.tick_range.first = js_reply.value("tick1").toString().toInt();
    rec.pool.tick_range.second = js_reply.value("tick2").toString().toInt();
}
void PosTxWorker::fillTxLogRecord(TxLogRecord &rec, const QJsonObject &js_reply, const DefiPosition &pos)
{
    Q_UNUSED(js_reply);

    if (m_lastTx.tx_kind == txBurn) return;

    int row = m_lastTx.t_row;
    if (m_lastTx.tx_kind == txCollect || m_lastTx.tx_kind == txDecrease)
    {
        rec.pool.tick = pos.state.pool_tick;
        rec.pool.price = m_table->item(row, PRICE_COL)->text().toFloat();
        rec.pool.pool_addr = m_table->item(row, POOL_COL)->toolTip();
        if (m_lastTx.tx_kind == txDecrease)
        {
            rec.pool.token_sizes.first = pos.state.asset_amounts.first;
            rec.pool.token_sizes.second = pos.state.asset_amounts.second;
        }
    }
    else if (m_lastTx.tx_kind == txTakeaway)
    {
        rec.pool.tick = pos.state.pool_tick;
        rec.pool.price = m_table->item(row, PRICE_COL)->text().toFloat();
        rec.pool.pool_addr = m_table->item(row, POOL_COL)->toolTip();
        rec.pool.token_sizes.first = pos.state.asset_amounts.first;
        rec.pool.token_sizes.second = pos.state.asset_amounts.second;
        rec.pool.reward_sizes.first = pos.state.rewards.first;
        rec.pool.reward_sizes.second = pos.state.rewards.second;
    }
}
int PosTxWorker::tableRowIndexOf(int pid) const
{
    if (!m_table) return -1;

    QString cell_data = QString::number(pid);
    int n_row = m_table->rowCount();
    for (int i=0; i<n_row; i++)
        if (m_table->item(i, 0)->text().trimmed() == cell_data) return i;

    qWarning()<<QString("PosTxWorker::tableRowIndexOf WARNING not found pid %1").arg(pid);
    return -1;
}



///////////////////////TX FUNCTION////////////////////////////

void PosTxWorker::burnSelected(const QList<DefiPosition> &pos_data)
{
    qDebug("PosTxWorker::burnSelected()");
    QList<int> list = LTable::selectedRows(m_table);
    if (list.isEmpty()) {emit signalError("PosTxWorker: You must select several positions"); return;}

    //check selected pos state
    bool ok;
    QString err;
    QList<int> pids;
    foreach (int row, list)
    {
        int pid = m_table->item(row, 0)->text().trimmed().toInt(&ok);
        if (!ok) {err = QString("invalid convert PID(%1) to integer").arg(m_table->item(row, 0)->text()); break;}

        int j = -2;
        emit signalGetPosIndexByPid(pid, j);
        if (j < 0) {err = QString("can't find PID(%1) to positions container").arg(pid); break;}

        if (pos_data.at(j).state.invalid()) {err = QString("you must update state for position PID(%1)").arg(pid); break;}
        if (pos_data.at(j).hasLiquidity()) {err = QString("position PID(%1) has liquidity").arg(pid); break;}
        if (pos_data.at(j).rewardSum() > 0) {err = QString("position PID(%1) has rewards > 0").arg(pid); break;}

        pids.append(pid);
    }
    if (!err.isEmpty()) {emit signalError(err); return;}


    // init TX dialog
    TxDialogData data(lastTx(), chainName());
    for(int i=0; i<pids.count(); i++)
        data.dialog_params.insert(QString("pid%1").arg(i+1), QString::number(pids.at(i)));

    TxBurnPosDialog d(data, parentWidget());
    d.exec();
    if (d.isApply()) emit signalSendTx(data);
    else emit signalMsg("operation was canceled!");
}
void PosTxWorker::collectSelected(const QList<DefiPosition> &pos_data)
{
    qDebug("PosTxWorker::collectSelected()");
    QList<int> list = LTable::selectedRows(m_table);
    if (list.isEmpty()) {emit signalError("PosTxWorker: You must select several positions"); return;}
    if (list.count() > 1) {emit signalError("PosTxWorker: You must select only one position"); return;}


    bool ok;
    m_lastTx.t_row = list.first();
    m_lastTx.pid = m_table->item(m_lastTx.t_row, 0)->text().trimmed().toInt(&ok);
    if (!ok)
    {
        m_lastTx.reset();
        emit signalError(QString("invalid convert PID(%1) to integer").arg(m_table->item(m_lastTx.t_row, 0)->text()));
        return;
    }

    int j = -2;
    emit signalGetPosIndexByPid(m_lastTx.pid, j);
    if (j < 0) {emit signalError(QString("can't find PID(%1) to positions container").arg(m_lastTx.pid)); return;}
    if (pos_data.at(j).state.invalid()) {emit signalError(QString("you must update state for position PID(%1)").arg(m_lastTx.pid)); return;}
    if (pos_data.at(j).rewardSum() < 0.1) {emit signalError(QString("position PID(%1) has rewards nearly 0.0").arg(m_lastTx.pid)); return;}

    // init TX dialog
    TxDialogData data(lastTx(), chainName());
    data.dialog_params.insert(QString("pid"), m_table->item(m_lastTx.t_row, 0)->text());
    data.dialog_params.insert(QString("desc"), m_table->item(m_lastTx.t_row, POOL_COL)->text());
    data.pool_addr = m_table->item(m_lastTx.t_row, POOL_COL)->toolTip();
    QString s_price = QString("%1 (%2)").arg(m_table->item(m_lastTx.t_row, PRICE_COL)->text()).arg(m_table->item(m_lastTx.t_row, PRICE_COL)->toolTip());
    data.dialog_params.insert(QString("price"), s_price);
    data.dialog_params.insert(QString("reward"), m_table->item(m_lastTx.t_row, REWARDS_COL)->text());

    TxCollectRewardDialog d(data, parentWidget());
    d.exec();
    if (d.isApply()) emit signalSendTx(data);
    else emit signalMsg("operation was canceled!");
}
void PosTxWorker::decreaseSelected(const QList<DefiPosition> &pos_data)
{
    qDebug("PosTxWorker::decreaseSelected()");
    QList<int> list = LTable::selectedRows(m_table);
    if (list.isEmpty()) {emit signalError("PosTxWorker: You must select several positions"); return;}
    if (list.count() > 1) {emit signalError("PosTxWorker: You must select only one position"); return;}

    bool ok;
    m_lastTx.t_row = list.first();
    m_lastTx.pid = m_table->item(m_lastTx.t_row, 0)->text().trimmed().toInt(&ok);
    if (!ok)
    {
        m_lastTx.reset();
        emit signalError(QString("invalid convert PID(%1) to integer").arg(m_table->item(m_lastTx.t_row, 0)->text()));
        return;
    }

    int j = -2;
    emit signalGetPosIndexByPid(m_lastTx.pid, j);
    if (j < 0) {emit signalError(QString("can't find PID(%1) to positions container").arg(m_lastTx.pid)); return;}
    if (pos_data.at(j).state.invalid()) {emit signalError(QString("you must update state for position PID(%1)").arg(m_lastTx.pid)); return;}
    if (!pos_data.at(j).hasLiquidity()) {emit signalError(QString("position PID(%1) has't liquidity").arg(m_lastTx.pid)); return;}

    // init TX dialog
    TxDialogData data(lastTx(), chainName());
    data.dialog_params.insert(QString("pid"), m_table->item(m_lastTx.t_row, 0)->text());
    data.dialog_params.insert(QString("desc"), m_table->item(m_lastTx.t_row, POOL_COL)->text());
    data.pool_addr = m_table->item(m_lastTx.t_row, POOL_COL)->toolTip();
    QString s_price = QString("%1 (%2)").arg(m_table->item(m_lastTx.t_row, PRICE_COL)->text()).arg(m_table->item(m_lastTx.t_row, PRICE_COL)->toolTip());
    data.dialog_params.insert(QString("price"), s_price);
    data.dialog_params.insert(QString("assets"), m_table->item(m_lastTx.t_row, ASSETS_COL)->text());
    data.dialog_params.insert(QString("liq"), m_table->item(m_lastTx.t_row, LIQ_COL)->text());

    TxDecreaseLiqDialog d(data, parentWidget());
    d.exec();
    if (d.isApply()) emit signalSendTx(data);
    else emit signalMsg("operation was canceled!");

}
void PosTxWorker::takeawaySelected(const QList<DefiPosition> &pos_data)
{
    qDebug("PosTxWorker::takeawaySelected()");
    QList<int> list = LTable::selectedRows(m_table);
    if (list.isEmpty()) {emit signalError("PosTxWorker: You must select several positions"); return;}
    if (list.count() > 1) {emit signalError("PosTxWorker: You must select only one position"); return;}

    bool ok;
    m_lastTx.t_row = list.first();
    m_lastTx.pid = m_table->item(m_lastTx.t_row, 0)->text().trimmed().toInt(&ok);
    if (!ok)
    {
        m_lastTx.reset();
        emit signalError(QString("invalid convert PID(%1) to integer").arg(m_table->item(m_lastTx.t_row, 0)->text()));
        return;
    }

    int j = -2;
    emit signalGetPosIndexByPid(m_lastTx.pid, j);
    if (j < 0) {emit signalError(QString("can't find PID(%1) to positions container").arg(m_lastTx.pid)); return;}
    if (pos_data.at(j).state.invalid()) {emit signalError(QString("you must update state for position PID(%1)").arg(m_lastTx.pid)); return;}
    if (!pos_data.at(j).hasLiquidity()) {emit signalError(QString("position PID(%1) has't liquidity").arg(m_lastTx.pid)); return;}

    // init TX dialog
    TxDialogData data(lastTx(), chainName());
    data.dialog_params.insert(QString("pid"), m_table->item(m_lastTx.t_row, 0)->text());
    data.dialog_params.insert(QString("desc"), m_table->item(m_lastTx.t_row, POOL_COL)->text());
    data.pool_addr = m_table->item(m_lastTx.t_row, POOL_COL)->toolTip();

    QString s_price = QString("%1 (%2)").arg(m_table->item(m_lastTx.t_row, PRICE_COL)->text()).arg(m_table->item(m_lastTx.t_row, PRICE_COL)->toolTip());
    data.dialog_params.insert(QString("price"), s_price);
    data.dialog_params.insert(QString("price_range"), m_table->item(m_lastTx.t_row, RANGE_COL)->text());
    if (pos_data.at(j).isOutRange()) data.dialog_params.insert(QString("out"), "yes");
    else data.dialog_params.insert(QString("out"), "no");

    data.dialog_params.insert(QString("reward"), m_table->item(m_lastTx.t_row, REWARDS_COL)->text());
    data.dialog_params.insert(QString("assets"), m_table->item(m_lastTx.t_row, ASSETS_COL)->text());
    data.dialog_params.insert(QString("liq"), m_table->item(m_lastTx.t_row, LIQ_COL)->text());

    TxTakeawayLiqDialog d(data, parentWidget());
    d.exec();
    if (d.isApply()) emit signalSendTx(data);
    else emit signalMsg("operation was canceled!");

}
void PosTxWorker::mintPos()
{
    m_lastTx.mint_activated = true;

    TxDialogData data(lastTx(), chainName());
    DefiMintDialog d(data, parentWidget());
    m_mintDialog = &d;
    connect(&d, SIGNAL(signalGetTokenBalance(QString, float&)), this, SIGNAL(signalGetTokenBalance(QString, float&)));
    connect(&d, SIGNAL(signalTryUpdatePoolState(const QString&)), this, SIGNAL(signalTryUpdatePoolState(const QString&)));
    connect(&d, SIGNAL(signalEmulateMint(const TxDialogData&)), this, SLOT(slotEmulateMint(const TxDialogData&)));

    d.exec();
    if (d.isApply())
    {
        m_mintDialog = NULL;
        qDebug("PosTxWorker::mintPos()  send MINT_TX");
        emit signalSendTx(data);
    }
    else
    {
        m_mintDialog = NULL;
        qDebug("cancel mint");
        emit signalMsg("operation was canceled!");
    }
}
void PosTxWorker::poolStateReceived(const QStringList &p_state)
{
    if (m_mintDialog)
        m_mintDialog->poolStateReceived(p_state);
}
void PosTxWorker::emulMintReply(const QJsonObject &js_reply)
{
    if (m_mintDialog)
        m_mintDialog->emulMintReply(js_reply);
}
void PosTxWorker::slotEmulateMint(const TxDialogData &data)
{
    if (data.dialog_params.contains("error"))
    {
        emit signalError(data.dialog_params.value("error"));
        return;
    }

    emit signalSendTx(data);
}





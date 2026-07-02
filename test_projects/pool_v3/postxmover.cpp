#include "postxmover.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "deficonfig.h"
#include "txdialog.h"
#include "txlogrecord.h"
#include "defiposition.h"
#include "defimintdialog.h"
#include "lstring.h"
#include "nodejsbridge.h"



#include <QTableWidget>
#include <QWidget>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>


#define POOL_COL             1
#define RANGE_COL            2
#define ASSETS_COL           4
#define REWARDS_COL          5
#define PRICE_COL            3

/*
#define RANGE_COL            2
#define ASSETS_COL           4
#define REWARDS_COL          5
#define LIQ_COL              6
*/


#define PROGRESS_STEP_INTERVAL      1300
#define STAGE_DELAY                 3
#define STAGE_DELAY_TX              8
#define STAGE_TIMEOUT               20 // seconds



//PosTxMover
PosTxMover::PosTxMover(QObject *parent, int cid, QTableWidget *t)
    :PosTxWorkerBase(parent, cid, t),
      m_timer(NULL),
      m_tsTimeout(-1),
      m_tsDelay(-1),
      m_stage(-1),
      m_moveDialog(NULL),
      js_obj(NULL)
{
    setObjectName("pos_tx_mover");

    m_timer = new QTimer(this);
    m_timer->setInterval(PROGRESS_STEP_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotStageTimer()));
    m_timer->stop();


    js_obj = new NodejsBridge(this, cid);
    connect(js_obj, SIGNAL(signalNodejsReply(const QJsonObject&)), this, SLOT(slotNodejsReply(const QJsonObject&)));
    connect(js_obj, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(js_obj, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(js_obj, SIGNAL(signalFinished(int)), this, SLOT(slotJSScriptFinished(int)));

}
bool PosTxMover::isRunning() const
{
    return m_timer->isActive();
}
void PosTxMover::poolStateReceived(const QJsonObject &js_reply)
{
    if (!isRunning()) return;

    m_moveParams.pool_tick = js_reply.value("tick").toString().toInt();
    m_moveDialog->setPoolTick(m_moveParams.pool_tick);
    m_tsDelay = QDateTime::currentSecsSinceEpoch();
    m_tsTimeout = 0;

    m_moveDialog->setStageText(QString("pool_state done"));
    m_moveDialog->setProgressStopStage();


/*
    QStringList keys = js_reply.keys();

    qDebug("received keys:");
    foreach (const QString &v, keys)
    {
         qDebug() << v;

    }*/

}
void PosTxMover::tryMove(const DefiPosition &position)
{
    // STAGE_1
    qDebug()<<position.toStr();
    if (position.state.invalid())
    {
        emit signalError(QString("you must update state for position PID(%1)").arg(position.pid));
        return;
    }
    if (!position.hasLiquidity())
    {
        emit signalError(QString("position PID(%1) liquidity is closed").arg(position.pid));
        return;
    }
    if (!position.isOutRange())
    {
        emit signalError(QString("position PID(%1) working in range now").arg(position.pid));
        return;
    }
    qDebug()<<QString("POOL: %1").arg(position.poolAddress());

    // STAGE_2
    if (position.state.asset_amounts.first > 0)
    {
        m_moveParams.amount = position.state.asset_amounts.first;
        m_moveParams.token_index = 0;
    }
    else
    {
        m_moveParams.amount = position.state.asset_amounts.second;
        m_moveParams.token_index = 1;
    }
    m_moveParams.pool_addr = position.poolAddress();
    m_moveParams.token0_address = position.token_addrs.first;
    m_moveParams.token1_address = position.token_addrs.second;
    m_moveParams.pool_fee = position.fee;
    m_moveParams.tick_spacing = position.tickSpacing();
    m_moveParams.pid = position.pid;
    m_moveParams.liq = position.liq;
    m_moveParams.taken_asset_amounts = position.state.asset_amounts;
    m_moveParams.taken_rewards = position.state.rewards;


    // init TX dialog

    int row = tableRowIndexOf(position.pid);
    TxDialogData data(txMove, chainName());
    data.dialog_params.insert(QString("pid"), QString::number(position.pid));
    data.dialog_params.insert(QString("desc"), (row >= 0) ? m_table->item(row, POOL_COL)->text() : "???");
    data.pool_addr = position.poolAddress();

    data.dialog_params.insert(QString("price_range"), position.interfacePriceRange());
    data.dialog_params.insert(QString("tick_range"), position.interfaceTickRange());
    data.dialog_params.insert(QString("tick_range_size"), QString::number(position.tickRangeSize()));
    data.dialog_params.insert(QString("assets"), position.interfaceAssetAmounts());
    data.dialog_params.insert(QString("reward"), position.interfaceRewards());

    /*
    data.dialog_params.insert(QString("price_range"), (row >= 0) ? m_table->item(row, RANGE_COL)->text() : "???");
    data.dialog_params.insert(QString("tick_range"), (row >= 0) ? m_table->item(row, RANGE_COL)->toolTip() : "???");
    data.dialog_params.insert(QString("tick_range_size"), position.interfaceRewards());
    data.dialog_params.insert(QString("reward"), (row >= 0) ? m_table->item(row, REWARDS_COL)->text() : "???");
    data.dialog_params.insert(QString("assets"), (row >= 0) ? m_table->item(row, ASSETS_COL)->text() : "???");
    */

    m_moveDialog = new TxMovePositionDialog(data, parentWidget());
    connect(m_moveDialog, SIGNAL(signalStartMove()), this, SLOT(slotStartMove()));

    m_moveDialog->exec();


    if (m_moveDialog->isApply())
    {
        qDebug("TxMovePositionDialog apply");
     //   emit signalSendTx(data);
    }
    else emit signalMsg("operation was canceled!");

    delete m_moveDialog;
    m_moveDialog = NULL;
}
void PosTxMover::slotStageTimer()
{
    qDebug()<<QString("PosTxMover::slotStageTimer()  m_tsDelay=%1  m_tsTimeout=%2").arg(m_tsDelay).arg(m_tsTimeout);
    qint64 ts_cur = QDateTime::currentSecsSinceEpoch();
    if (isDelaing()) // delay now
    {
        m_moveDialog->setDelayProgress();
        if (ts_cur - m_tsDelay > STAGE_DELAY) // delay is over
        {
            m_tsDelay = 0;
            m_tsTimeout = 0;
            nextStage();
        }
        return;
    }

    // check stage timeout
    if (isRunningStage())
    {
        if (ts_cur - m_tsTimeout > STAGE_TIMEOUT) // timeout is over is over
        {
            breakMoving("timeout is over");
        }
    }
    else runStage();
}
void PosTxMover::breakMoving(const QString &text)
{
    m_timer->stop();
    m_tsTimeout = -1;
    m_stage = -1;
    m_moveDialog->breakMoving(text);

    if (js_obj->buzy()) js_obj->breakProcessing();

}
void PosTxMover::finishMoving()
{
    m_timer->stop();
    m_tsTimeout = -1;
    m_stage = msStoped;
    m_moveDialog->finishMoving();
}
void PosTxMover::nextStage()
{
    switch (m_stage)
    {
        case msStoped:
        {
            m_stage = msFixAmount;
            m_moveDialog->setStageText("fix_amount ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msFixAmount:
        {
            m_stage = msTakeAway;
            m_moveDialog->setStageText("take_away assets ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msTakeAway:
        {
            m_stage = msTakeAway_res;
            m_moveDialog->setStageText("take_away TX-result ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msTakeAway_res:
        {
            m_stage = msPoolState;
            m_moveDialog->setStageText("pool_state ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msPoolState:
        {
            m_stage = msCalcNewRange;
            m_moveDialog->setStageText("calc_range ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msCalcNewRange:
        {
            m_stage = msMint;
            m_moveDialog->setStageText("mint ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msMint:
        {
            m_stage = msMint_res;
            m_moveDialog->setStageText("mint TX-result ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msMint_res:
        {
            m_stage = msPosList;
            m_moveDialog->setStageText("position_list ...");
            m_moveDialog->setProgressStartStage();
            break;
        }
        case msPosList:
        {
            finishMoving();
            break;
        }


        default:
        {
            break;
        }
    }

}
void PosTxMover::runStage()
{
    m_tsTimeout = QDateTime::currentSecsSinceEpoch();
    m_moveDialog->setProgressStartStage();
    switch (m_stage)
    {
        case msFixAmount:
        {
            m_tsDelay = m_tsTimeout;
            m_tsTimeout = 0;
            m_moveDialog->setStageText(QString("fix_amount/%1/token_%2").arg(m_moveParams.amount).arg(m_moveParams.token_index));
            m_moveDialog->setProgressStopStage();
            break;
        }
        case msTakeAway:
        {
            // test
            //m_tsDelay = m_tsTimeout + STAGE_DELAY_TX;
            //m_tsTimeout = 0;
            //m_moveDialog->setStageText(QString("take_away TX done"));
            //m_moveDialog->setProgressStopStage();


            // real chain req
            m_tsDelay = 0;
            takeAway();
            break;
        }
        case msTakeAway_res:
        {
            // test
            //m_tsDelay = m_tsTimeout;
            //m_tsTimeout = 0;
            //m_moveDialog->setStageText(QString("take_away_res done"));
            //m_moveDialog->setProgressStopStage();

            // real chain req
            //m_moveParams.last_hash = "0x16e2830cb83efa0dd4cfd8912455a92cc52673b68ca92311ff96b414ab9adf8d";
            m_tsDelay = 0;
            lastTxStatus_res();


            break;
        }
        case msPoolState:
        {
            m_tsDelay = 0;
            emit signalGetPoolState(m_moveParams.pool_addr);
            break;
        }
        case msCalcNewRange:
        {
            m_tsDelay = m_tsTimeout;
            m_tsTimeout = 0;
            calcNextRange();
            m_moveDialog->setStageText(QString("calc_range done"));
            m_moveDialog->setProgressStopStage();
            break;
        }
        case msMint:
        {
            // test
            //m_tsDelay = m_tsTimeout;
            //m_tsTimeout = 0;
            //m_moveDialog->setStageText(QString("mint TX done"));
            //m_moveDialog->setProgressStopStage();


            //real chain
            m_tsDelay = 0;
            nextMint();


            break;
        }
        case msMint_res:
        {
            //m_tsDelay = m_tsTimeout;
            //m_tsTimeout = 0;
            //m_moveDialog->setStageText(QString("mint_res done"));
            //m_moveDialog->setProgressStopStage();

            // real chain req
            //m_moveParams.last_hash = "0x16e2830cb83efa0dd4cfd8912455a92cc52673b68ca92311ff96b414ab9adf8d";
            m_tsDelay = 0;
            lastTxStatus_res();

            break;
        }
        case msPosList:
        {
            m_tsDelay = m_tsTimeout;
            m_tsTimeout = 0;
            m_moveDialog->setStageText(QString("pos_list done"));
            m_moveDialog->setProgressStopStage();
            break;
        }

        default:
        {
            finishMoving();
            break;
        }
    }
}
void PosTxMover::slotStartMove()
{
    qDebug("PosTxMover::slotStartMove()");
    m_tsTimeout = QDateTime::currentSecsSinceEpoch();
    m_tsDelay = m_tsTimeout;
    m_moveDialog->setStageText("starting");
    m_moveDialog->setDelayProgress();
    m_stage = msStoped;

    m_timer->start();
}
void PosTxMover::calcNextRange()
{
    qDebug("PosTxMover::calcNextRange()");
    bool is_left = (m_moveParams.token_index == 1); // признак того что новый диапазон должен быть слева от текущего тика
    int size = int(float(m_moveDialog->tickRangeSize())*m_moveDialog->tickRangeFactor());
    if (m_moveParams.tick_spacing > 1)
    {
        while (2 > 1)
        {
            if (size % m_moveParams.tick_spacing == 0) break;
            size++;
        }
    }
    qDebug()<<QString("next range size %1").arg(size);

    if (is_left)
    {
        m_moveParams.tick_range.second = m_moveParams.pool_tick;
        if (m_moveParams.tick_spacing > 1)
        {
            while (2 > 1)
            {
                if (m_moveParams.tick_range.second % m_moveParams.tick_spacing == 0) break;
                m_moveParams.tick_range.second--;
            }
            m_moveParams.tick_range.second -= m_moveParams.tick_spacing;
        }
        m_moveParams.tick_range.first = m_moveParams.tick_range.second - size;
    }
    else
    {
        m_moveParams.tick_range.first = m_moveParams.pool_tick;
        if (m_moveParams.tick_spacing > 1)
        {
            while (2 > 1)
            {
                if (m_moveParams.tick_range.first % m_moveParams.tick_spacing == 0) break;
                m_moveParams.tick_range.first++;
            }
            m_moveParams.tick_range.first += m_moveParams.tick_spacing;
        }
        else m_moveParams.tick_range.first++;
        m_moveParams.tick_range.second = m_moveParams.tick_range.first + size;
    }

    m_moveDialog->setNextTickRange(m_moveParams.tick_range.first, m_moveParams.tick_range.second);
}
void PosTxMover::takeAway()
{
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(txTakeaway));
    j_params.insert("simulate_mode", "no");
    j_params.insert("liq", m_moveParams.liq);
    j_params.insert("pid", QString::number(m_moveParams.pid));
    j_params.insert("gas_unit_price", gasField());
    emit signalRewriteJsonFile(j_params, AppCommonSettings::txParamsNodeJSFile()); //rewrite params json-file

    jsScriptRun();
}
QString PosTxMover::gasField() const
{
    int chain_index = defi_config.chainIndexOf(chainId());
    float gp = defi_config.chains.at(chain_index).gas_unit_price;
    QString s_gp = "-1.0";
    if (gp > 0)
    {
        if (gp < 0.01) s_gp = QString::number(gp, 'f', 6);
        else s_gp = QString::number(gp);
    }
    return s_gp;
}
void PosTxMover::lastTxStatus_res()
{
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcTXStatus));
    j_params.insert("tx_hash", m_moveParams.last_hash);
    emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

    jsScriptRun();
}
void PosTxMover::nextMint()
{
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(txMint));
    j_params.insert("simulate_mode", "no");
    j_params.insert("gas_unit_price", gasField());
    j_params.insert("fee", QString::number(m_moveParams.pool_fee));
    j_params.insert("tick1", QString::number(m_moveParams.tick_range.first));
    j_params.insert("tick2", QString::number(m_moveParams.tick_range.second));

    j_params.insert("pool_address", m_moveParams.pool_addr);
    j_params.insert("token0_address", m_moveParams.token0_address);
    j_params.insert("token1_address", m_moveParams.token1_address);

    if (m_moveParams.token_index == 1)
    {
        j_params.insert("token1_amount", QString::number(m_moveParams.amount));
        j_params.insert("token0_amount", QString::number(-1));
    }
    else
    {
        j_params.insert("token0_amount", QString::number(m_moveParams.amount));
        j_params.insert("token1_amount", QString::number(-1));
    }


    emit signalRewriteJsonFile(j_params, AppCommonSettings::txParamsNodeJSFile()); //rewrite params json-file

    jsScriptRun();
}
void PosTxMover::jsScriptRun()
{
    QStringList args;
    if (m_stage == msTakeAway || m_stage == msMint)
    {
        args << "tx_writer.js" << AppCommonSettings::txParamsNodeJSFile();
    }
    else if (m_stage == msTakeAway_res || m_stage == msMint_res)
    {
        args << "reader.js" << AppCommonSettings::readParamsNodeJSFile();
    }
    js_obj->slotRunScriptArgs(args);
}


// -----------------NodejsReply----------------------
void PosTxMover::slotNodejsReply(const QJsonObject &js_reply)
{

    qDebug("PosTxMover::slotNodejsReply OK!!!");
    if (!isRunning()) return;


    QStringList keys = js_reply.keys();
    if (keys.isEmpty()) {breakMoving("js_reply is empty"); return;}

    if (m_stage == msTakeAway)
    {
        if (!js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
        {
            breakMoving("take_away TX fault");
            return;
        }

        m_tsDelay = QDateTime::currentSecsSinceEpoch() + STAGE_DELAY_TX;
        m_tsTimeout = 0;
        m_moveDialog->setStageText(QString("take_away TX done"));
        m_moveDialog->setProgressStopStage();

        m_moveParams.last_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
        qDebug()<<QString("m_moveParams.last_hash [%1]").arg(m_moveParams.last_hash);

        prepareTakeawayTxLog();
    }
    else if (m_stage == msTakeAway_res || m_stage == msMint_res)
    {
        QString status = js_reply.value("status").toString().trimmed();
        if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
        {
            if (status.toLower() == "ok")
            {
                m_tsDelay = QDateTime::currentSecsSinceEpoch();
                m_tsTimeout = 0;
                m_moveDialog->setStageText(QString("TX status OK"));
                m_moveDialog->setProgressStopStage();
            }
            else
            {
                breakMoving(QString("TX_status: [%1]").arg(status));
            }
            txStatusDone(js_reply);
        }
        else breakMoving("reply has't field <tx_hash>");
    }
    else if (m_stage == msMint)
    {
        if (!js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
        {
            breakMoving("mint TX fault");
            return;
        }

        m_tsDelay = QDateTime::currentSecsSinceEpoch() + STAGE_DELAY_TX;
        m_tsTimeout = 0;
        m_moveDialog->setStageText(QString("mint TX done"));
        m_moveDialog->setProgressStopStage();

        m_moveParams.last_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
        qDebug()<<QString("m_moveParams.last_hash [%1]").arg(m_moveParams.last_hash);

        prepareMintTxLog(js_reply);

    }


}
void PosTxMover::slotJSScriptFinished(int code)
{
    qDebug()<<QString("PosTxMover::slotJSScriptFinished code=%1").arg(code);
    if (!isRunning()) return;

    if (code != 0)
    {
        breakMoving(QString("Nodejs fault, code(%1)").arg(code));
    }
}
void PosTxMover::prepareTakeawayTxLog()
{
    int row = tableRowIndexOf(m_moveParams.pid);
    TxLogRecord tx_rec(NodejsBridge::jsonCommandValue(txTakeaway), chainName());
    tx_rec.tx_hash = m_moveParams.last_hash;
    tx_rec.pool.pid = m_moveParams.pid;
    tx_rec.pool.tick = m_moveParams.pool_tick;
    tx_rec.pool.price0 = m_table->item(row, PRICE_COL)->data(Qt::UserRole).toFloat();
    tx_rec.pool.pool_addr = m_table->item(row, POOL_COL)->toolTip();
    tx_rec.pool.token_sizes.first = m_moveParams.taken_asset_amounts.first;
    tx_rec.pool.token_sizes.second = m_moveParams.taken_asset_amounts.second;
    tx_rec.pool.reward_sizes.first = m_moveParams.taken_rewards.first;
    tx_rec.pool.reward_sizes.second = m_moveParams.taken_rewards.second;
    parseTickRange(m_table->item(row, RANGE_COL)->toolTip(), tx_rec);


    // prepare extra data for TX record
    QString extra_data = m_table->item(row, POOL_COL)->text();
    extra_data = QString("%1  ASSETS(%2)").arg(extra_data).arg(m_table->item(row, ASSETS_COL)->text());
    extra_data = QString("%1  REWARDS(%2)").arg(extra_data).arg(m_table->item(row, REWARDS_COL)->text());
    extra_data.replace(QChar('/'), QChar('|'));
    tx_rec.formNote(extra_data);

    // send record to TX-log
    emit signalNewTx(tx_rec);
}
void PosTxMover::prepareMintTxLog(const QJsonObject &js_reply)
{
    //int row = tableRowIndexOf(m_moveParams.pid);
    TxLogRecord tx_rec(NodejsBridge::jsonCommandValue(txMint), chainName());
    tx_rec.tx_hash = m_moveParams.last_hash;

    tx_rec.pool.pool_addr = js_reply.value("pool_address").toString().trimmed();
    tx_rec.pool.price0 = js_reply.value("pool_price").toString().toFloat();
    tx_rec.pool.tick = js_reply.value("pool_tick").toString().toInt();
    tx_rec.pool.token_sizes.first = js_reply.value("amount0").toString().toFloat();
    tx_rec.pool.token_sizes.second = js_reply.value("amount1").toString().toFloat();
    tx_rec.pool.price_range.first = js_reply.value("p1").toString().toFloat();
    tx_rec.pool.price_range.second = js_reply.value("p2").toString().toFloat();
    tx_rec.pool.tick_range.first = js_reply.value("tick1").toString().toInt();
    tx_rec.pool.tick_range.second = js_reply.value("tick2").toString().toInt();


    // prepare extra data for TX record
    QString extra_data;
    int pool_index = defi_config.getPoolIndex(m_moveParams.pool_addr);
    if (pool_index >= 0)
    {
        QString t0_name = defi_config.token0NameByPoolAddr(m_moveParams.pool_addr);
        QString t1_name = defi_config.token1NameByPoolAddr(m_moveParams.pool_addr);
        extra_data = QString("%1/%2 (%3)").arg(t0_name).arg(t1_name).arg(defi_config.pools.at(pool_index).strFloatFee());
    }
    else extra_data = QString("MINT: invalid pool_addr(%1), can't find in defi_config").arg(m_moveParams.pool_addr);
    extra_data.replace(QChar('/'), QChar('|'));
    tx_rec.formNote(extra_data);

    // send record to TX-log
    emit signalNewTx(tx_rec);
}
void PosTxMover::txStatusDone(const QJsonObject &js_reply)
{
    QString status = js_reply.value("status").toString().trimmed().toLower();
    if (status == "ok" || status == "fault")
    {
        qDebug("status finished");
        // prepare data for TXPage
        QMap<QString, QString> map;
        map.insert("hash", m_moveParams.last_hash);
        map.insert("status", status);
        map.insert("fee", js_reply.value("fee").toString());
        map.insert("gas_used", js_reply.value("gas_used").toString());
        emit signalStrategyTxStatus(map);
    }
    else qWarning()<<QString("wrong status: %1").arg(status);
}





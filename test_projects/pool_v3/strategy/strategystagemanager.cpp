#include "strategystagemanager.h"
#include "strategystepdialogstruct.h"
#include "nodejsbridge.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "lcommonsettings.h"
#include "lfile.h"
#include "defiposition.h"


#include <QDebug>
#include <QTimer>
#include <QJsonObject>
#include <QJsonValue>

#define STAGE_TIMER_INTERVAL    1500
#define STAGE_TIMEOUT           45 // seconds
#define MIN_SWAP_PART_ERR       3 // %, насколько должен отличатся объем приоритеного токена от заданной величины чтобы применить своп



//StrategyStageManagerObj
StrategyStageManagerObj::StrategyStageManagerObj(StrategyStepDialogData &data, QObject *parent)
    :LSimpleObject(parent),
      m_timer(NULL),
      m_data(data)
{
    setObjectName("stage_manager_obj");

    m_timer = new QTimer(this);
    m_timer->setInterval(STAGE_TIMER_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTick()));

}
void StrategyStageManagerObj::run()
{
    if (m_actionStages.isEmpty()) return;

    m_stageState.reset();
    m_userSign = -1;
    m_timer->start();
}
QString StrategyStageManagerObj::captionByStage(int stage)
{
    switch (stage)
    {
        case sssGetPoolState: return QString("Getting pool state");
        case sssGetWalletBalances: return QString("Update wallet balances");
        case sssGetWalletBalancesAfterSwap: return QString("Get wallet balances after swap");
        case sssReadLineSettings: return QString("Read line settings");
        case sssDefineLineLiq: return QString("Define line liq size");

        case sssCalcSwapPart: return QString("Calc swap part");
        case sssCheckWalletTokenAmounts: return QString("Check wallet assets size");

        case sssTxSwap: return QString("TX swap");
        case sssGetTxSwapResult: return QString("Check TX-swap result");
        case sssGetPriceRangeNextPos: return QString("Define next price range");

        case sssTxMint: return QString("TX mint");
        case sssGetTxMintResult: return QString("Check TX-mint result");
        case sssGetPositionsList: return QString("Get wallet positions list");

        case sssGetStepPositionState: return QString("Getting position state");
        case sssSavePositionState: return QString("Save position state");
        case sssTxClosePosition: return QString("TX takeaway liq");
        case sssGetTxClosePosResult: return QString("Check TX-takeaway result");
        case sssCalcStepResult: return QString("Calc total result");


        default:    break;
    }
    return "?";
}
QString StrategyStageManagerObj::kindByStage(int stage)
{
    switch (stage)
    {
        case sssGetStepPositionState:
        case sssGetTxClosePosResult:
        case sssGetPoolState:
        case sssGetTxSwapResult:
        case sssGetTxMintResult:
        case sssGetPriceRangeNextPos:
        case sssGetPositionsList:
        case sssGetWalletBalancesAfterSwap:
        case sssGetWalletBalances: return QString("ETHERS_READ");

        case sssTxClosePosition:
        case sssTxMint:
        case sssTxSwap: return QString("ETHERS_TX");

        case sssCalcSwapPart:
        case sssCalcStepResult:
        case sssSavePositionState:
        case sssReadLineSettings:
        case sssCheckWalletTokenAmounts:
        case sssDefineLineLiq: return QString("LOCAL_CALC");

        default:    break;
    }
    return "?";
}
void StrategyStageManagerObj::fillStagesList(int a_type)
{
    m_actionStages.clear();
    switch (a_type)
    {
        case ssaCloseStep:
        {
            m_actionStages << sssGetWalletBalances << sssGetStepPositionState << sssSavePositionState <<
                      sssTxClosePosition << sssGetTxClosePosResult << sssGetWalletBalances << sssCalcStepResult;
            break;
        }
        case ssaFirstStep:
        {
            /* orig
        m_actionStages << sssGetWalletBalances << sssGetPoolState << sssReadLineSettings << sssCalcSwapPart <<
              sssCheckWalletTokenAmounts << sssTxSwap << sssGetTxSwapResult << sssGetWalletBalancesAfterSwap <<
              sssGetPriceRangeNextPos << sssTxMint << sssGetTxMintResult << sssGetPositionsList << sssGetWalletBalances;

              */


        m_actionStages << sssGetWalletBalances << sssGetPoolState << sssReadLineSettings << sssCalcSwapPart <<
              sssCheckWalletTokenAmounts << sssTxSwap << sssGetTxSwapResult << sssGetWalletBalancesAfterSwap <<
              sssGetPriceRangeNextPos << sssTxMint << sssGetTxMintResult << sssGetPositionsList << sssGetWalletBalances;


       // m_actionStages << sssGetWalletBalances << sssReadLineSettings << sssGetPriceRangeNextPos;

            break;

        }
        case ssaNextStep:
        {
            m_actionStages << sssGetWalletBalances << sssGetPoolState << sssDefineLineLiq <<
                  sssCalcSwapPart << sssCheckWalletTokenAmounts << sssTxSwap << sssGetTxSwapResult << sssGetPriceRangeNextPos <<
                  sssTxMint << sssGetTxMintResult << sssGetPositionsList << sssGetWalletBalances;
            break;
        }
        default:
        {
            qWarning()<<QString("StrategyStageManagerObj::fillStagesList WARNING invalid action %1").arg(a_type);
            break;
        }
    }

}
void StrategyStageManagerObj::slotTimerTick()
{
    qDebug("StrategyStageManagerObj::slotTimerTick()");
    if (m_actionStages.isEmpty() && m_userSign < 0)
    {
        m_timer->stop();
        emit signalFinishedAll();
        return;
    }

    if (m_data.delayRunning())
    {
        qDebug()<<QString("delayRunning  delay_after_tx=%1").arg(m_data.delay_after_tx);
        m_data.delay_after_tx--;
        if (m_data.delay_after_tx == 0) continueAfterDelay();
        return;
    }

    checkStageState();
    if (isRunning()) {checkTimeout(); return;}

    if (m_userSign < 0)
    {
        m_userSign = m_actionStages.takeFirst();
        m_stageState.reset();
        m_stageState.ts_start = QDateTime::currentDateTime();
        emit signalStartStage(m_userSign);
    }
    else
    {
        m_userSign = -1;
        emit signalStageFinished(m_stageState.result);
    }
}
bool StrategyStageManagerObj::isRunning() const
{
    return (m_timer->isActive() && currentStage() > 0 && m_stageState.ts_start.isValid() && m_stageState.result.isEmpty());
}
void StrategyStageManagerObj::checkTimeout()
{
    int t_cur = QDateTime::currentDateTime().toSecsSinceEpoch();
    int t_diff = t_cur - m_stageState.ts_start.toSecsSinceEpoch();
    if (t_diff > STAGE_TIMEOUT)
    {
        m_userSign = -1;
        m_timer->stop();
        emit signalStageFinished("Timeout");
        emit signalFinishedAll();
    }
}
void StrategyStageManagerObj::checkStageState()
{
    qDebug()<<QString("check stage state: %1").arg(currentStage());
    if (!needToEthersReq())
    {
        if (currentStage() == sssReadLineSettings)
        {
            m_stageState.result = "ok";
        }
        else if (currentStage() == sssCalcSwapPart)
        {
            QString s0 = QString::number(m_data.swap_info.first, 'f', AppCommonSettings::interfacePricePrecision(m_data.swap_info.first));
            QString s1 = QString::number(m_data.swap_info.second, 'f', AppCommonSettings::interfacePricePrecision(m_data.swap_info.second));
            m_stageState.note = QString("%1 / %2").arg(s0).arg(s1);
            m_stageState.result = "ok";
        }
        else if (currentStage() == sssCheckWalletTokenAmounts)
        {
            if (m_stageState.note.trimmed().toLower().contains("reject")) m_stageState.result = "fault";
            else m_stageState.result = "ok";
        }

    }
    else
    {
        if (currentStage() == sssTxSwap || currentStage() == sssGetTxSwapResult || currentStage() == sssGetWalletBalancesAfterSwap)
        {
            if (m_data.none_swap)
            {
                m_stageState.note = "NONE_SWAP";
                m_stageState.result = "ok";
            }
        }

    }
}
void StrategyStageManagerObj::stop()
{
    m_userSign = -1;
    m_timer->stop();
    emit signalStageFinished("Breaked");
    emit signalFinishedAll();
}
void StrategyStageManagerObj::execCurrentStage()
{
    if (m_userSign == sssGetWalletBalances) getWalletTokenAmounts();
    else if (m_userSign == sssGetPoolState) getPoolState();
    else if (m_userSign == sssReadLineSettings) readLineSettings();
    else if (m_userSign == sssCalcSwapPart) calcSwapParts();
    else if (m_userSign == sssCheckWalletTokenAmounts) checkBalancesBeforeSwap();
    else if (m_userSign == sssTxSwap) makeSwap_TX();
    else if (m_userSign == sssGetTxSwapResult)
    {
        m_data.delay_after_tx = -55;
        checkTxSwapStatus();
    }
    else if (m_userSign == sssGetWalletBalancesAfterSwap) getWalletTokenAmounts();
    else if (m_userSign == sssGetPriceRangeNextPos) getNextPosRange();
    else if (m_userSign == sssTxMint) makeMint_TX();
    else if (m_userSign == sssGetTxMintResult)
    {
        m_data.delay_after_tx = -55;
        checkTxSwapStatus();
    }
    else if (m_userSign == sssGetPositionsList) getPositionsList();


}
void StrategyStageManagerObj::jsScriptRun()
{
    QStringList args;
    QString kind = StrategyStageManagerObj::kindByStage(currentStage());
    if (kind.trimmed().toLower().contains("_tx"))
    {
        qDebug("StrategyStageManagerObj::jsScriptRun()  WRITE");
        args << "tx_writer.js" << AppCommonSettings::txParamsNodeJSFile();
    }
    else
    {
        qDebug("StrategyStageManagerObj::jsScriptRun()  READ");
        args << "reader.js" << AppCommonSettings::readParamsNodeJSFile();
    }

    emit signalRunScriptArgs(args);
}
bool StrategyStageManagerObj::needToEthersReq() const
{
    QString kind = StrategyStageManagerObj::kindByStage(currentStage());
    return kind.trimmed().toLower().contains("ethers");
}
void StrategyStageManagerObj::setFaultResult(QString note)
{
    m_stageState.result = "fault";
    m_stageState.note = note;
}
quint16 StrategyStageManagerObj::txDelay() const
{
    quint16 a = lCommonSettings.paramValue("tx_delay").toUInt();
    if (a < 10) return 10;
    if (a > 300) return 300;
    return a;
}
void StrategyStageManagerObj::continueAfterDelay()
{
    qDebug("StrategyStageManagerObj::continueAfterDelay()");
    m_stageState.ts_start = QDateTime::currentDateTime();
    m_data.delay_after_tx = -1;
    emit signalDelayAfterTx(m_data.delay_after_tx);

    if (m_userSign == sssGetTxSwapResult) checkTxSwapStatus();
    else if (m_userSign == sssGetTxMintResult) checkTxMintStatus();
}
void StrategyStageManagerObj::readNodejsPosFile()
{

    QString fname = QString("%1/%2").arg(AppCommonSettings::nodejsPath()).arg(AppCommonSettings::positionsListFile());

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty())
    {
        m_stageState.note = "Err positions file";
        emit signalError(QString("DefiPositionsPage: %1").arg(err));
        return;
    }
    if (fdata.isEmpty())
    {
        m_stageState.note = "positions is empty";
        return;
    }

    foreach (const QString &fline, fdata)
    {
        DefiPosition   rec;
        rec.fromFileLine(fline);
        if (rec.invalid()) continue;

        if (rec.hasLiquidity())
        {
            if (rec.token_addrs.first.trimmed() == m_data.pool_token_addrs.first.trimmed() &&
                    rec.token_addrs.second.trimmed() == m_data.pool_token_addrs.second.trimmed() &&
                    rec.tick_range.first == m_data.pos_range_tick.first &&
                    rec.tick_range.second == m_data.pos_range_tick.second &&
                    rec.fee == m_data.pool_fee)
            {
                m_data.mint_pid = rec.pid;
                break;
            }
        }
    }


}




// -----------------NodejsReply----------------------
void StrategyStageManagerObj::slotNodejsReply(const QJsonObject &js_reply)
{
    qDebug("StrategyStageManagerObj::slotNodejsReply OK!!!");
    QStringList keys = js_reply.keys();
    if (keys.isEmpty()) {setFaultResult("js_reply is empty"); return;}

    if (currentStage() == sssGetWalletBalances) readWalletAssetsBalanceReply(js_reply);
    else if (currentStage() == sssGetPoolState) readPoolStateReply(js_reply);
    else if (currentStage() == sssTxSwap) readSwapTxReply(js_reply);
    else if (currentStage() == sssGetTxSwapResult) readSwapTxStatusReply(js_reply);
    else if (currentStage() == sssGetWalletBalancesAfterSwap) readWalletAssetsBalanceAfterSwapReply(js_reply);
    else if (currentStage() == sssGetPriceRangeNextPos) readPriceRangeNextPosReply(js_reply);
    else if (currentStage() == sssTxMint) readMintTxReply(js_reply);
    else if (currentStage() == sssGetTxMintResult) readMintTxStatusReply(js_reply);
    else if (currentStage() == sssGetPositionsList) readPositionsReply(js_reply);



}
void StrategyStageManagerObj::slotJSScriptFinished(int code)
{
    qDebug()<<QString("StrategyStageManagerObj::slotJSScriptFinished code=%1").arg(code);
    if (code != 0)
    {
        setFaultResult(QString("code = %1").arg(code));
    }
}
void StrategyStageManagerObj::readWalletAssetsBalanceReply(const QJsonObject &js_reply)
{
    QStringList keys = js_reply.keys();
    QString t0 = m_data.pool_tickers.first;
    QString t1 = m_data.pool_tickers.second;
    if (!keys.contains(t0) || !keys.contains(t1)) {setFaultResult("not found tickers"); return;}

    if (m_data.mintDone())
    {
        m_data.wallet_balances_after_mint.first = js_reply.value(t0).toString().toFloat();
        m_data.wallet_balances_after_mint.second = js_reply.value(t1).toString().toFloat();
        m_stageState.note = QString("%1 / %2").arg(m_data.wallet_balances_after_mint.first).arg(m_data.wallet_balances_after_mint.second);
    }
    else
    {
        m_data.wallet_assets_balance.first = js_reply.value(t0).toString().toFloat();
        m_data.wallet_assets_balance.second = js_reply.value(t1).toString().toFloat();
        m_stageState.note = QString("%1 / %2").arg(m_data.wallet_assets_balance.first).arg(m_data.wallet_assets_balance.second);
    }
    m_stageState.result = "ok";
}
void StrategyStageManagerObj::readPoolStateReply(const QJsonObject &js_reply)
{
    m_data.pool_tick = js_reply.value("tick").toString().toInt();
    if (m_data.prior_price_i == 1) m_data.pool_price = js_reply.value("price1").toString().toFloat();
    else m_data.pool_price = js_reply.value("price0").toString().toFloat();

    // calc tvl
    float p_t0 = (defi_config.isStableToken(m_data.pool_tickers.first) ? 1.0 : defi_config.lastPriceByTokenName(m_data.pool_tickers.first));
    float p_t1 = (defi_config.isStableToken(m_data.pool_tickers.second) ? 1.0 : defi_config.lastPriceByTokenName(m_data.pool_tickers.second));
    float tvl0 = js_reply.value("tvl0").toString().toFloat();
    float tvl1 = js_reply.value("tvl1").toString().toFloat();
    float tvl = tvl0*p_t0 + tvl1*p_t1;

    m_stageState.note = QString("tvl=%1 / price=%2 / tick=%3").arg(tvl).arg(m_data.pool_price).arg(m_data.pool_tick);
    m_stageState.result = "ok";
}
void StrategyStageManagerObj::readSwapTxReply(const QJsonObject &js_reply)
{
    m_stageState.note = "done";
    m_stageState.result = "fault";

    int result_code = js_reply.value("code").toString().toInt();
    if (result_code != 0) {m_stageState.note = QString("error: %1").arg(result_code); return;}
    QString tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    if (tx_hash.length() < 30) {m_stageState.note = QString("is_emulate"); return;}

    // TX was done
    emit signalTxWasDone(js_reply);
    m_stageState.result = "ok";
}
void StrategyStageManagerObj::readSwapTxStatusReply(const QJsonObject &js_reply)
{
    m_stageState.note = "done";
    m_stageState.result = "fault";
    QString status = js_reply.value("status").toString().trimmed();
    if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        if (status.toLower() == "ok") {m_stageState.result = "ok"; m_data.swap_done = true;}
        if (status.isEmpty()) m_stageState.note = QString("status: ?");
        else m_stageState.note = QString("status: %1").arg(status);

        emit signalTxStatusDone(js_reply);
    }
    else qWarning("StrategyStageManagerObj: WARNING nodejs reply has't field <tx_hash>");
}
void StrategyStageManagerObj::readWalletAssetsBalanceAfterSwapReply(const QJsonObject &js_reply)
{
    QStringList keys = js_reply.keys();
    QString t0 = m_data.pool_tickers.first;
    QString t1 = m_data.pool_tickers.second;
    if (!keys.contains(t0) || !keys.contains(t1)) {setFaultResult("not found tickers"); return;}

    m_data.wallet_balances_after_swap.first = js_reply.value(t0).toString().toFloat();
    m_data.wallet_balances_after_swap.second = js_reply.value(t1).toString().toFloat();
    m_stageState.note = QString("%1 / %2").arg(m_data.wallet_balances_after_swap.first).arg(m_data.wallet_balances_after_swap.second);
    m_stageState.result = "ok";

    // корректируем ликвидность линии
    m_data.line_liq.first += (m_data.wallet_balances_after_swap.first - m_data.wallet_assets_balance.first);
    m_data.line_liq.second += (m_data.wallet_balances_after_swap.second - m_data.wallet_assets_balance.second);

}
void StrategyStageManagerObj::readPriceRangeNextPosReply(const QJsonObject &js_reply)
{
    if (js_reply.contains("error"))
    {
        m_stageState.result = "fault";
        return;
    }

    m_data.pos_range.first = js_reply.value("p1").toString().toFloat();
    m_data.pos_range.second = js_reply.value("p2").toString().toFloat();
    m_data.pos_range_tick.first = js_reply.value("tick1").toString().toInt();
    m_data.pos_range_tick.second = js_reply.value("tick2").toString().toInt();
    if (m_data.prior_price_i == 1)
    {
        float p2 = float(1)/m_data.pos_range.first;
        m_data.pos_range.first = float(1)/m_data.pos_range.second;
        m_data.pos_range.second = p2;
    }

    m_data.real_mint_amounts.first = js_reply.value("amount0").toString().toFloat();
    m_data.real_mint_amounts.second = js_reply.value("amount1").toString().toFloat();
    m_stageState.result = "ok";

    m_stageState.note = QString("[%1 : %2]").arg(m_data.pos_range_tick.first).arg(m_data.pos_range_tick.second);

}
void StrategyStageManagerObj::readMintTxReply(const QJsonObject &js_reply)
{
    qDebug()<<QString("StrategyStageManagerObj::readMintTxReply");

    m_stageState.note = "done";
    m_stageState.result = "fault";


    int result_code = js_reply.value("code").toString().toInt();
    if (result_code != 0) {m_stageState.note = QString("error: %1").arg(result_code); return;}
    QString tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    if (tx_hash.length() < 30) {m_stageState.note = QString("is_emulate"); return;}

    //test
    //m_stageState.result = "ok";
    //return;
    //------

    // TX was done
    emit signalTxWasDone(js_reply);
    m_stageState.result = "ok";

}
void StrategyStageManagerObj::readMintTxStatusReply(const QJsonObject &js_reply)
{
    qDebug("StrategyStageManagerObj::readMintTxStatusReply()");

    m_stageState.note = "done";
    m_stageState.result = "fault";
    QString status = js_reply.value("status").toString().trimmed();
    if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        if (status.toLower() == "ok") {m_stageState.result = "ok"; }
        if (status.isEmpty()) m_stageState.note = QString("status: ?");
        else m_stageState.note = QString("status: %1").arg(status);

        emit signalTxStatusDone(js_reply);
    }
    else qWarning("StrategyStageManagerObj: WARNING nodejs reply has't field <tx_hash>");
}
void StrategyStageManagerObj::readPositionsReply(const QJsonObject &js_reply)
{
    qDebug("StrategyStageManagerObj::readPositionsReply");
    m_stageState.result = "fault";

    bool result_ok = false;
    if (js_reply.contains("result")) result_ok = (js_reply.value("result").toString().trimmed().toLower() == "true");
    int n_pos = js_reply.value("pos_count").toString().trimmed().toInt();
    QString("n_pos = %1").arg(n_pos);

    if (!result_ok)
    {
        m_stageState.note = QString("result: FAULT");
        return;
    }

    readNodejsPosFile();
    m_stageState.note = QString("Minted PID: %1").arg(m_data.mint_pid);
    if (m_data.mint_pid > 0) m_stageState.result = "ok";
}



//private funcs
void StrategyStageManagerObj::getWalletTokenAmounts()
{
    if (currentStage() == sssGetWalletBalancesAfterSwap && m_data.none_swap) return;

    if (needToEthersReq())
    {
        QJsonObject j_params;
        j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcBalance));
        emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

        jsScriptRun();
    }
}
void StrategyStageManagerObj::getPoolState()
{
    if (needToEthersReq())
    {
        QJsonObject j_params;
        j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPoolState));
        //int pos = defi_config.getPoolIndex(m_data.pool_addr);
        j_params.insert("pool_address", m_data.pool_addr);
        j_params.insert("token0_address", m_data.pool_token_addrs.first);
        j_params.insert("token1_address", m_data.pool_token_addrs.second);
        j_params.insert("fee", QString::number(m_data.pool_fee));
        emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

        jsScriptRun();
    }
}
void StrategyStageManagerObj::readLineSettings()
{
    //test
    //m_data.pool_price = 0.105;
    //m_data.wallet_assets_balance.first = 5000;
    //m_data.wallet_assets_balance.second = 500;
    //m_data.tx_swap_hash = "0xf990fcaef81b9b0441666661735c261144d69f4d83c8afcd16f856ae91ec917a";
    //m_data.none_swap = false;

    //m_data.line_liq.first = 0.56;
    //m_data.line_liq.second = 2.04;
    //m_data.pos_range_tick.first = -301820;
    //m_data.pos_range_tick.second = -297610;
    //return;
/////////////////////////////////////////////////

    m_stageState.note = QString("START_LIQ: %1 %2").arg(m_data.start_line_liq).
            arg((m_data.prior_amount_i==0) ? m_data.pool_tickers.first : m_data.pool_tickers.second);

    m_data.line_liq.first = m_data.line_liq.second = 0;
    if (m_data.first_token_index == 0 && m_data.prior_amount_i == 1)
    {
        m_data.line_liq.first = m_data.start_line_liq/m_data.pool_price;
        if (m_data.prior_price_i == m_data.prior_amount_i)
            m_data.line_liq.first = m_data.start_line_liq*m_data.pool_price;
        return;
    }
    if (m_data.first_token_index == 1 && m_data.prior_amount_i == 0)
    {
        m_data.line_liq.second = m_data.start_line_liq/m_data.pool_price;
        if (m_data.prior_price_i == m_data.prior_amount_i)
            m_data.line_liq.second = m_data.start_line_liq*m_data.pool_price;
        return;
    }

    if (m_data.first_token_index == 0) m_data.line_liq.first = m_data.start_line_liq;
    else m_data.line_liq.second = m_data.start_line_liq;
}
void StrategyStageManagerObj::calcSwapParts()
{
    m_data.none_swap = true;
    float min_part_err = float(MIN_SWAP_PART_ERR)/float(100);

    qDebug("StrategyStageManagerObj::calcSwapParts()");
    //quint8 i_prior = defi_config.getPriorAmountIndexByPoolAddr(m_data.pool_addr);
    float liq0 = 0;
    float liq1 = 0;
    float t = float(m_data.prior_asset_size)/float(100);
    m_data.swap_info.first = m_data.swap_info.second = 0;
    if (m_data.pool_price == 0) return;

    if (m_data.prior_amount_i == 0)
    {
        liq0 = m_data.line_liq.first;
        liq1 = m_data.line_liq.second*m_data.pool_price;
        if (m_data.prior_amount_i == m_data.prior_price_i) liq1 = m_data.line_liq.second/m_data.pool_price;

        float sum = liq0 + liq1; // полная ликвидность на текущем шаге (в приоритетном токене)
        if (sum == 0) {qWarning("WARNING: sum == 0"); return;}
        float p_liq0 = liq0/sum;
        if (qAbs(p_liq0 - t) < min_part_err) return; // отличие от требуемой величины менее 1%, менять не будем

        float d_liq = qAbs(p_liq0 - t);
        m_data.none_swap = false;
        if (p_liq0 - t < 0) // не хватает приоритетного токена
        {
            m_data.swap_info.first = sum*d_liq;
            m_data.swap_info.second = -1*m_data.swap_info.first/m_data.pool_price;
        }
        else  // не хватает 2-го токена
        {
            m_data.swap_info.first = -1*sum*d_liq;
            m_data.swap_info.second = -1*m_data.swap_info.first/m_data.pool_price;
        }
    }
    else
    {
        qDebug("prior_amount_i == 1");
        liq0 = m_data.line_liq.first*m_data.pool_price;
        if (m_data.prior_amount_i == m_data.prior_price_i) liq0 = m_data.line_liq.first/m_data.pool_price;
        liq1 = m_data.line_liq.second;
        qDebug()<<QString("pool_price=%1  liq0=%2 liq1=%3  t=%4").arg(m_data.pool_price).arg(liq0).arg(liq1).arg(t);
        float sum = liq0 + liq1; // полная ликвидность на текущем шаге (в приоритетном токене)
        if (sum == 0) {qWarning("WARNING: sum == 0"); return;}
        float p_liq1 = liq1/sum;
        if (qAbs(p_liq1 - t) < min_part_err) return; // отличие от требуемой величины менее 1%, менять не будем

        float d_liq = qAbs(p_liq1 - t);
        //qDebug()<<QString("p_liq1=%1  d_liq=%2").arg(p_liq1).arg(d_liq);
        m_data.none_swap = false;
        if (p_liq1 - t < 0) // не хватает приоритетного токена
        {
            m_data.swap_info.second = sum*d_liq;
            m_data.swap_info.first = -1*m_data.swap_info.second/m_data.pool_price;
        }
        else  // не хватает 2-го токена
        {
            m_data.swap_info.second = -1*sum*d_liq;
            m_data.swap_info.first = -1*m_data.swap_info.second/m_data.pool_price;
        }
    }
}
void StrategyStageManagerObj::checkBalancesBeforeSwap()
{
    m_stageState.note = "NONE_SWAP";
    qDebug()<<QString("checkBalancesBeforeSwap: walet balances: %1/%2").arg(m_data.wallet_assets_balance.first).arg(m_data.wallet_assets_balance.second);
    if (m_data.swap_info.first > 0 &&  m_data.swap_info.second < 0)
    {
        qDebug("CASE_1");
        if (m_data.wallet_assets_balance.second > qAbs(m_data.swap_info.second)) m_stageState.note = "SUCCESS";
        else m_stageState.note = "REJECTED";
    }
    if (m_data.swap_info.first < 0 &&  m_data.swap_info.second > 0)
    {
        qDebug("CASE_2");
        if (m_data.wallet_assets_balance.first > qAbs(m_data.swap_info.first)) m_stageState.note = "SUCCESS";
        else m_stageState.note = "REJECTED";
    }
}
void StrategyStageManagerObj::makeSwap_TX()
{
    m_data.tx_swap_hash.clear();
    if (m_data.none_swap) return;

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(txSwap));
    j_params.insert("simulate_mode", "no");
    emit signalAddGasPriceField(j_params);

    // fill pool data
    j_params.insert("pool_address", m_data.pool_addr);
    j_params.insert("token0_address", m_data.pool_token_addrs.first);
    j_params.insert("token1_address", m_data.pool_token_addrs.second);
    j_params.insert("fee", QString::number(m_data.pool_fee));

    // fill input
    float v_input = qAbs(m_data.swap_info.first);
    quint8 i_input = 0;
    if (m_data.swap_info.second < 0)
    {
        v_input = qAbs(m_data.swap_info.second);
        i_input = 1;
    }
    j_params.insert("input_amount", QString::number(v_input, 'f', 6));
    j_params.insert("input_index", QString::number(i_input));


    emit signalRewriteJsonFile(j_params, AppCommonSettings::txParamsNodeJSFile()); //rewrite params json-file for TX

    jsScriptRun();

}
void StrategyStageManagerObj::checkTxSwapStatus()
{
    qDebug("StrategyStageManagerObj::checkTxSwapStatus()");
    if (m_data.none_swap) {m_data.delay_after_tx = -1; return;}

    if (m_data.needDelay())
    {
        qDebug("send delay command");
        m_data.delay_after_tx = txDelay();
        emit signalDelayAfterTx(m_data.delay_after_tx);
        return;
    }

    qDebug("continue checkTxSwapStatus");
    // send nodejs req
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcTXStatus));
    j_params.insert("tx_hash", m_data.tx_swap_hash);
    emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

    jsScriptRun();

}
void StrategyStageManagerObj::getNextPosRange()
{
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPosRange));

    //int pos = defi_config.getPoolIndex(m_data.pool_addr);
    //j_params.insert("pool_address", m_data.pool_addr);
    //j_params.insert("token0_address", defi_config.pools.at(pos).token0_addr);
    //j_params.insert("token1_address", defi_config.pools.at(pos).token1_addr);
    //j_params.insert("fee", QString::number(defi_config.pools.at(pos).fee));

    j_params.insert("pool_address", m_data.pool_addr);
    j_params.insert("token0_address", m_data.pool_token_addrs.first);
    j_params.insert("token1_address", m_data.pool_token_addrs.second);
    j_params.insert("fee", QString::number(m_data.pool_fee));


    j_params.insert("range_width", QString::number(m_data.price_width));
    j_params.insert("price_index", QString::number(m_data.prior_price_i));
    j_params.insert("amount0", QString::number(m_data.line_liq.first, 'f', 6));
    j_params.insert("amount1", QString::number(m_data.line_liq.second, 'f', 6));

    emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file
    jsScriptRun();
}
void StrategyStageManagerObj::makeMint_TX()
{
    qDebug("StrategyStageManagerObj::makeMint_TX()");
    m_data.tx_mint_hash.clear();

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(txMint));
    j_params.insert("simulate_mode", "no");
    emit signalAddGasPriceField(j_params);

    // fill pool data  
    j_params.insert("pool_address", m_data.pool_addr);
    j_params.insert("token0_address", m_data.pool_token_addrs.first);
    j_params.insert("token1_address", m_data.pool_token_addrs.second);
    j_params.insert("fee", QString::number(m_data.pool_fee));


    // fill mint params
    j_params.insert("tick1", QString::number(m_data.pos_range_tick.first));
    j_params.insert("tick2", QString::number(m_data.pos_range_tick.second));
    j_params.insert("token0_amount", QString::number(m_data.real_mint_amounts.first));
    j_params.insert("token1_amount", QString::number(-1));


    emit signalRewriteJsonFile(j_params, AppCommonSettings::txParamsNodeJSFile()); //rewrite params json-file for TX
    jsScriptRun();
}
void StrategyStageManagerObj::checkTxMintStatus()
{
    qDebug("StrategyStageManagerObj::checkTxMintStatus()");
    if (m_data.needDelay())
    {
        qDebug("send delay command");
        m_data.delay_after_tx = txDelay();
        emit signalDelayAfterTx(m_data.delay_after_tx);
        return;
    }

    qDebug("continue checkTxMintStatus");
    // send nodejs req
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcTXStatus));
    j_params.insert("tx_hash", m_data.tx_mint_hash);
    emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

    jsScriptRun();
}
void StrategyStageManagerObj::getPositionsList()
{
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPositions));
    emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

    jsScriptRun();
}





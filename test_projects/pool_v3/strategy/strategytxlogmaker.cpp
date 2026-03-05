#include "strategytxlogmaker.h"
#include "strategystepdialogstruct.h"
#include "appcommonsettings.h"
#include "nodejsbridge.h"
#include "txlogrecord.h"



#include <QJsonObject>
#include <QJsonValue>



//StrategyTxLogMaker
StrategyTxLogMaker::StrategyTxLogMaker(StrategyStepDialogData &data, QObject *parent)
    :LSimpleObject(parent),
      m_data(data)
{
    setObjectName("stage_tx_log_maker_obj");


}
void StrategyTxLogMaker::slotTxWasDone(const QJsonObject &js_reply)
{
    qDebug("StrategyTxLogMaker::slotTxWasDone");
    SM_TxInfo tx_info;
    tx_info.req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    tx_info.hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    m_txList.append(tx_info);

    if (tx_info.req == NodejsBridge::jsonCommandValue(txSwap))
    {
        makeSwapLog(js_reply);
    }
}
void StrategyTxLogMaker::slotTxStatusDone(const QJsonObject &js_reply)
{
    // test code
    //SM_TxInfo tx_info_test;
    //tx_info_test.req = "swap";
    //tx_info_test.hash = "0xf990fcaef81b9b0441666661735c261144d69f4d83c8afcd16f856ae91ec917a";
    //m_txList.append(tx_info_test);
    /////////////////////

    qDebug("StrategyTxLogMaker::slotTxStatusDone");
    QString hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed();
    if (m_txList.isEmpty() || hash.isEmpty()) return;

    int i = m_txList.count() - 1;
    if (m_txList.at(i).hash != hash) return;

    QString status = js_reply.value("status").toString().trimmed().toLower();
    if (status == "ok" || status == "fault")
    {
        qDebug("status finished");
        m_txList[i].status = status;
        m_txList[i].gas_fee = js_reply.value("fee").toString().toFloat();

        // prepare data for TXPage
        QMap<QString, QString> map;
        map.insert("hash", hash);
        map.insert("status", status);
        map.insert("fee", js_reply.value("fee").toString());
        map.insert("gas_used", js_reply.value("gas_used").toString());
        emit signalStrategyTxStatus(map);
    }
    else qWarning()<<QString("wrong status: %1").arg(status);
}
void StrategyTxLogMaker::makeSwapLog(const QJsonObject &js_reply)
{
    qDebug("StrategyTxLogMaker::makeSwapLog");

    m_data.tx_swap_hash = m_txList.last().hash;

    TxLogRecord tx_rec(m_txList.last().req, m_data.cur_chain);
    tx_rec.tx_hash = m_txList.last().hash;
    tx_rec.pool.pool_addr = m_data.pool_addr;
    tx_rec.pool.token_in = js_reply.value("tokenIn").toString().trimmed();
    tx_rec.pool.token_sizes.first = js_reply.value("input_amount").toString().toFloat();
    tx_rec.pool.price0 = m_data.pool_price;
    if (m_data.prior_price_i == 1) tx_rec.pool.price0 = 1/m_data.pool_price;

    QString extra_data = "?";
    int input_index = js_reply.value("input_index").toString().toInt();
    if (input_index == 0)
    {
        //QString s0 = QString::number(-1*m_data.swap_info.first, 'f', AppCommonSettings::interfacePricePrecision(-1*m_data.swap_info.first));
        //QString s1 = QString::number(m_data.swap_info.second, 'f', AppCommonSettings::interfacePricePrecision(m_data.swap_info.second));
        //extra_data = QString("%1%2 => %3%4").arg(s0).arg(m_data.pool_tickers.first).arg(s1).arg(m_data.pool_tickers.second);
        extra_data = QString("%1 => %2").arg(m_data.pool_tickers.first).arg(m_data.pool_tickers.second);
    }
    else
    {
        //QString s0 = QString::number(m_data.swap_info.first, 'f', AppCommonSettings::interfacePricePrecision(m_data.swap_info.first));
        //QString s1 = QString::number(-1*m_data.swap_info.second, 'f', AppCommonSettings::interfacePricePrecision(-1*m_data.swap_info.second));
        //extra_data = QString("%1%2 => %3%4").arg(s1).arg(m_data.pool_tickers.second).arg(s0).arg(m_data.pool_tickers.first);
        extra_data = QString("%1 => %2").arg(m_data.pool_tickers.second).arg(m_data.pool_tickers.first);
    }
    tx_rec.formNote(extra_data);

    qDebug()<<QString("extra_data [%1]").arg(extra_data);
    qDebug()<<QString("note [%1]").arg(tx_rec.note);
    qDebug()<<QString("hash [%1]").arg(tx_rec.tx_hash);


    emit signalStrategyTx(tx_rec);
}



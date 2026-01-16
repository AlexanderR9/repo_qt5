#include "txlogrecord.h"
#include "appcommonsettings.h"
#include "lstring.h"
#include "nodejsbridge.h"


#define FEE_COIN_PRECISION          8
#define FEE_CENT_PRECISION          4


///////////////TxLogRecord//////////////////
TxLogRecord::TxLogRecord(QString type, QString chain)
{
    reset();
    tx_kind = type;
    chain_name = chain;
    dt = QDateTime::currentDateTime();
}
void TxLogRecord::reset()
{
    tx_hash.clear();
    tx_kind = "unknown";
    chain_name = "?";
    dt = QDateTime();
    note = "empty";

    status.reset();
    wallet.reset();
    pool.reset();
}

bool TxLogRecord::resultOk() const
{
    return (status.result.trimmed() == "OK");
}
bool TxLogRecord::resultFault() const
{
    return (status.result.trimmed() == "FAULT");
}
bool TxLogRecord::isFinishedStatus() const
{
    return (resultOk() || resultFault());
}
void TxLogRecord::parseDetails(const QString &fdata)
{
    QStringList fields = LString::trimSplitList(fdata, ";");
    foreach (const QString &v, fields)
    {
        int pos = v.indexOf("[");
        if (pos > 0)
        {
            QString f_name = v.left(pos);
            QString f_value = LString::strBetweenStr(v, "[", "]");
            parseDetail(f_name, f_value);
        }
    }
}
void TxLogRecord::parseStatus(const QStringList &rec_data)
{
    //   записи в файле имеют вид: tx_hash / STATUS(OK/FAULT) / gas_used / gas_fee_coin / gas_fee_usd_cents
    status.result = rec_data.at(1);
    status.gas_used = rec_data.at(2).toUInt();
    status.fee_coin = rec_data.at(3).toFloat();
    status.fee_cent = rec_data.at(4).toFloat();
}
void TxLogRecord::parseDetail(QString field, QString value)
{
    if (field == "token_addr") wallet.token_addr = value;
    else if (field == "token_amount") wallet.token_amount = value.toFloat();
    else if (field == "note") note = value;
    else if (field == "to_contract") wallet.contract_addr = value;
    else if (field == "to_wallet") wallet.target_wallet = value;
    // for swap TX
    else if (field == "pool_addr") pool.pool_addr = value;
    else if (field == "token_in") pool.token_in = value;
    else if (field == "token_amount_in") pool.token_sizes.first = value.toFloat();
    else if (field == "current_price") pool.price = value.toFloat();
    // pos TX
    else if (field == "tick") pool.tick = value.toInt();
    else if (field == "pid") pool.pid = value.toInt();
    else if (field == "token_sizes") parseFloatPair(value, pool.token_sizes);
    else if (field == "reward_sizes") parseFloatPair(value, pool.reward_sizes);
    else if (field == "price_range") parseFloatPair(value, pool.price_range);
    else if (field == "tick_range") parseIntPair(value, pool.tick_range);


}
void TxLogRecord::parseFloatPair(QString value, QPair<float, float> &pair)
{
    pair.first = pair.second = -1;
    QStringList list = LString::trimSplitList(value, ":");
    if (list.count() != 2) return;

    bool ok;
    float a = list.first().toFloat(&ok);
    if (ok) pair.first = a;
    a = list.last().toFloat(&ok);
    if (ok) pair.second = a;
}
void TxLogRecord::parseIntPair(QString value, QPair<int, int> &pair)
{
    pair.first = pair.second = 0;
    QStringList list = LString::trimSplitList(value, ":");
    if (list.count() != 2) return;

    bool ok;
    int a = list.first().toInt(&ok);
    if (ok) pair.first = a;
    a = list.last().toInt(&ok);
    if (ok) pair.second = a;
}
bool TxLogRecord::invalid() const
{
    if (tx_hash.length() < 30 || tx_hash.left(2) != "0x") return true;
    if (tx_kind.isEmpty() || tx_kind == "unknown") return true;
    if (chain_name.length()< 3) return true;
    if (!dt.isValid() || dt.date().year() < 2023) return true;
    return false;
}
void TxLogRecord::setDateTime(QString s_date, QString s_time)
{
    if (!s_date.trimmed().isEmpty())
    {
        QDate d = QDate::fromString(s_date.trimmed(), AppCommonSettings::dateUserMask());
        dt.setDate(d);
    }
    if (!s_time.trimmed().isEmpty())
    {
        QTime t = QTime::fromString(s_time.trimmed(), AppCommonSettings::timeUserMask());
        dt.setTime(t);
    }
}
QString TxLogRecord::strDate() const
{
    return dt.date().toString(AppCommonSettings::dateUserMask());
}
QString TxLogRecord::strTime() const
{
    return dt.time().toString(AppCommonSettings::timeUserMask());
}
QString TxLogRecord::listFileLine() const
{
    if (invalid()) return QString();

    //    записи в файле имеют вид: tx_hash / date / time / chain_name / tx_kind
    QString s = QString("%1 / %2 / %3").arg(tx_hash).arg(strDate()).arg(strTime());
    s = QString("%1 / %2 / %3").arg(s).arg(chain_name).arg(tx_kind);
    return s;
}
QString TxLogRecord::statusFileLine() const
{
    if (invalid()) return QString();
    //if (!isFinishedStatus()) return QString();

    //   записи в файле имеют вид: tx_hash / STATUS(OK/FAULT) / gas_used / gas_fee_coin / gas_fee_usd_cents
    QString s = QString("%1 / %2 / %3").arg(tx_hash).arg(status.result).arg(status.gas_used);
    s.append(" / " + QString::number(status.fee_coin, 'f', FEE_COIN_PRECISION));
    s.append(" / " + QString::number(status.fee_cent, 'f', FEE_CENT_PRECISION));
    return s;
}
QString TxLogRecord::detailsFileLine() const
{
    if (invalid()) return QString();

    //    записи в файле имеют вид: tx_hash / status / (набор полей, соответствующий tx_kind, через ';')
    QString s = QString("%1 / %2").arg(tx_hash).arg(status.result);
    QString additions;
    if (tx_kind == NodejsBridge::jsonCommandValue(txWrap) || tx_kind == NodejsBridge::jsonCommandValue(txUnwrap))
    {
        additions = QString("token_addr[%1]; token_amount[%2];").arg(wallet.token_addr).arg(wallet.token_amount);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txTransfer) || tx_kind == NodejsBridge::jsonCommandValue(txApprove))
    {
        additions = QString("token_addr[%1]; token_amount[%2];").arg(wallet.token_addr).arg(wallet.token_amount);
        bool is_transfer = (tx_kind == NodejsBridge::jsonCommandValue(txTransfer));
        if (is_transfer) additions = QString("%1 to_wallet[%2];").arg(additions).arg(wallet.target_wallet);
        else additions = QString("%1 to_contract[%2];").arg(additions).arg(wallet.contract_addr);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txSwap))
    {
        additions = QString("pool_addr[%1]; token_in[%2];").arg(pool.pool_addr).arg(pool.token_in);
        additions = QString("%1 token_amount_in[%2];").arg(additions).arg(pool.token_sizes.first);
        additions = QString("%1 current_price[%2];").arg(additions).arg(pool.price);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txCollect) || tx_kind == NodejsBridge::jsonCommandValue(txDecrease))
    {
        additions = QString("pool_addr[%1]; pid[%2];").arg(pool.pool_addr).arg(pool.pid);
        additions = QString("%1 current_price[%2]; tick[%3];").arg(additions).arg(pool.price).arg(pool.tick);
        if (tx_kind == NodejsBridge::jsonCommandValue(txDecrease))
        {
            QString sa0 = QString::number(pool.token_sizes.first, 'f', AppCommonSettings::interfacePricePrecision(pool.token_sizes.first));
            QString sa1 = QString::number(pool.token_sizes.second, 'f', AppCommonSettings::interfacePricePrecision(pool.token_sizes.second));
            additions = QString("%1 token_sizes[%2:%3];").arg(additions).arg(sa0).arg(sa1);
        }
        additions = QString("%1 tick_range[%2:%3];").arg(additions).arg(pool.tick_range.first).arg(pool.tick_range.second);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txTakeaway))
    {
        additions = QString("pool_addr[%1]; pid[%2];").arg(pool.pool_addr).arg(pool.pid);
        additions = QString("%1 current_price[%2]; tick[%3];").arg(additions).arg(pool.price).arg(pool.tick);

        QString sa0 = QString::number(pool.token_sizes.first, 'f', AppCommonSettings::interfacePricePrecision(pool.token_sizes.first));
        QString sa1 = QString::number(pool.token_sizes.second, 'f', AppCommonSettings::interfacePricePrecision(pool.token_sizes.second));
        additions = QString("%1 token_sizes[%2:%3];").arg(additions).arg(sa0).arg(sa1);

        sa0 = QString::number(pool.reward_sizes.first, 'f', AppCommonSettings::interfacePricePrecision(pool.reward_sizes.first));
        sa1 = QString::number(pool.reward_sizes.second, 'f', AppCommonSettings::interfacePricePrecision(pool.reward_sizes.second));
        additions = QString("%1 reward_sizes[%2:%3];").arg(additions).arg(sa0).arg(sa1);

        additions = QString("%1 tick_range[%2:%3];").arg(additions).arg(pool.tick_range.first).arg(pool.tick_range.second);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txMint) || tx_kind == NodejsBridge::jsonCommandValue(txIncrease))
    {
        additions = QString("pool_addr[%1]; pid[%2];").arg(pool.pool_addr).arg(pool.pid);
        additions = QString("%1 current_price[%2]; tick[%3];").arg(additions).arg(pool.price).arg(pool.tick);

        QString sa0 = QString::number(pool.token_sizes.first, 'f', AppCommonSettings::interfacePricePrecision(pool.token_sizes.first));
        QString sa1 = QString::number(pool.token_sizes.second, 'f', AppCommonSettings::interfacePricePrecision(pool.token_sizes.second));
        additions = QString("%1 token_sizes[%2:%3];").arg(additions).arg(sa0).arg(sa1);

        sa0 = QString::number(pool.price_range.first, 'f', AppCommonSettings::interfacePricePrecision(pool.price_range.first));
        sa1 = QString::number(pool.price_range.second, 'f', AppCommonSettings::interfacePricePrecision(pool.price_range.second));
        additions = QString("%1 price_range[%2:%3];").arg(additions).arg(sa0).arg(sa1);

        additions = QString("%1 tick_range[%2:%3];").arg(additions).arg(pool.tick_range.first).arg(pool.tick_range.second);
    }
    additions = QString("%1 note[%2]").arg(additions).arg(note);

    s = QString("%1 / %2").arg(s).arg(additions);
    return s;
}
void TxLogRecord::formNote(QString extra_data)
{
    note = tx_kind;
    if (tx_kind == NodejsBridge::jsonCommandValue(txWrap) || tx_kind == NodejsBridge::jsonCommandValue(txUnwrap)
            || tx_kind == NodejsBridge::jsonCommandValue(txApprove) || tx_kind == NodejsBridge::jsonCommandValue(txTransfer))
    {
        note = QString("%1 %2 %3").arg(note).arg(QString::number(wallet.token_amount, 'f', AppCommonSettings::interfacePricePrecision(wallet.token_amount))).arg(extra_data);
        if (tx_kind == NodejsBridge::jsonCommandValue(txApprove)) note = QString("%1 to %2").arg(note).arg(wallet.contract_addr);
        if (tx_kind == NodejsBridge::jsonCommandValue(txTransfer)) note = QString("%1 to %2").arg(note).arg(wallet.target_wallet);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txSwap))
    {
        note = QString("%1 %2 %3").arg(note).arg(QString::number(pool.token_sizes.first, 'f', AppCommonSettings::interfacePricePrecision(wallet.token_amount))).arg(extra_data);
        note = QString("%1  price=%2").arg(note).arg(QString::number(pool.price));
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txBurn))
    {
        note = QString("%1 PID_ARR => %2").arg(note).arg(extra_data);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txCollect) || tx_kind == NodejsBridge::jsonCommandValue(txDecrease))
    {
        note = QString("%1 %2").arg(note).arg(extra_data);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txTakeaway))
    {
        int k = extra_data.indexOf(QChar(')'));
        if (k > 0) extra_data = extra_data.left(k+1);
        note = QString("%1 %2").arg(note).arg(extra_data);

        float a0 = pool.token_sizes.first + pool.reward_sizes.first;
        float a1 = pool.token_sizes.second + pool.reward_sizes.second;
        QString sa0 = QString::number(a0, 'f', AppCommonSettings::interfacePricePrecision(a0));
        QString sa1 = QString::number(a1, 'f', AppCommonSettings::interfacePricePrecision(a1));
        note = QString("%1  TAKEN_ASSETS(%2 | %3)").arg(note).arg(sa0).arg(sa1);
    }
    else if (tx_kind == NodejsBridge::jsonCommandValue(txMint) || tx_kind == NodejsBridge::jsonCommandValue(txIncrease))
    {
        note = QString("%1 %2").arg(note).arg(extra_data);

        float p1 = pool.price_range.first;
        float p2 = pool.price_range.second;
        QString sp1 = QString::number(p1, 'f', AppCommonSettings::interfacePricePrecision(p1));
        QString sp2 = QString::number(p2, 'f', AppCommonSettings::interfacePricePrecision(p2));
        note = QString("%1  RANGE(%2 : %3)").arg(note).arg(sp1).arg(sp2);


        float a0 = pool.token_sizes.first;
        float a1 = pool.token_sizes.second;
        QString sa0 = QString::number(a0, 'f', AppCommonSettings::interfacePricePrecision(a0));
        QString sa1 = QString::number(a1, 'f', AppCommonSettings::interfacePricePrecision(a1));
        note = QString("%1  ASSETS_AMOUNT(%2 | %3)").arg(note).arg(sa0).arg(sa1);
    }
}


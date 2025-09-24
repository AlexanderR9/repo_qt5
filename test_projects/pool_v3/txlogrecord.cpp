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
        additions = QString("pool_addr[%1]; token_in[%1];").arg(pool.pool_addr).arg(pool.token_in);
        additions = QString("%1 token_amount_in[%2];").arg(additions).arg(pool.token_sizes.first);
        additions = QString("%1 current_price[%2];").arg(additions).arg(pool.price);
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
    }
}


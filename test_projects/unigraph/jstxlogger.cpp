#include "jstxlogger.h"
#include "lfile.h"
#include "ltime.h"
#include "subcommonsettings.h"


#include <QDir>

#define TOKEN_SIZE_PRECISION        4
#define PRICE_PRECISION             6




//JSTxLogger
JSTxLogger::JSTxLogger(QObject *parent)
    :LSimpleObject(parent),
      m_currentChain(QString())
{
    m_userSign = 100;
    setObjectName("jstx_logger_obj");


}
void JSTxLogger::slotAddLog(const JSTxLogRecord &rec)
{
    if (rec.invalid())
    {
        emit signalError("JSTxLogger: record is invalid");
        return;
    }
    QString fline = rec.toFileLine().trimmed() + QChar('\n');

    QString err;
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(txLogFile());
    if (!LFile::fileExists(fname)) err = LFile::writeFile(fname, fline);
    else err = LFile::appendFile(fname, fline);

    if (!err.isEmpty())
        emit signalError(QString("JSTxLogger::slotAddLog, %1").arg(err));

    emit signalStartTXDelay(); //запустить диалоговое окно для блокировки интерфейса на определенную задержку
}



///////////////JSTxLogRecord//////////////////
JSTxLogRecord::JSTxLogRecord(QString type, QString chain)
{
    reset();
    tx_kind = type;
    chain_name = chain;
    dt = QDateTime::currentDateTime();

}
void JSTxLogRecord::reset()
{
    tx_hash.clear();
    tx_kind = "unknown";
    chain_name = "?";
    dt = QDateTime();
    pool_address.clear();
    token_address.clear();

    pid = current_tick = 0;
    token0_size = token1_size = current_price = -1;
    tick_range = "0:0";
    price_range = "-1.0:-1.0";
    note = "none";
}
QString JSTxLogRecord::toFileLine() const
{
    QString s = LTime::strDateTime(dt);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(chain_name).arg(tx_hash).arg(tx_kind);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(pool_address).arg(token_address).arg(pid);
    s = QString("%1 / %2").arg(s).arg(QString::number(token0_size, 'f', TOKEN_SIZE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(QString::number(token1_size, 'f', TOKEN_SIZE_PRECISION));
    s = QString("%1 / %2 / %3").arg(s).arg(QString::number(current_price, 'f', PRICE_PRECISION)).arg(current_tick);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(tick_range).arg(price_range).arg(note);
    return s;
}
void JSTxLogRecord::fromFileLine(const QString &fline)
{
    reset();

}
bool JSTxLogRecord::invalid() const
{
    if (tx_hash.isEmpty() || tx_kind == "unknown" || chain_name.length() < 3) return true;
    if (!dt.isValid() || dt.date().year() < 2020) return true;
    return false;
}








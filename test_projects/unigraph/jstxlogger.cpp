#include "jstxlogger.h"
#include "lfile.h"
#include "ltime.h"
#include "lstring.h"
#include "subcommonsettings.h"


#include <QDir>
#include <QDebug>

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
void JSTxLogger::reloadLogFile()
{
    m_logData.clear();
    emit signalMsg("");
    emit signalMsg("Try load log file data ........");

    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(txLogFile());
    if (!LFile::fileExists(fname)) {emit signalError(QString("log file[%1] not found").arg(txLogFile())); return;}

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(QString("can not read log file[%1]").arg(txLogFile())); return;}

    foreach (const QString &fline, fdata)
    {
        QString s = fline.trimmed();
        if (s.isEmpty()) continue;

        JSTxLogRecord rec;
        rec.fromFileLine(s);
        if (rec.invalid()) qWarning()<<QString("JSTxLogger: WARNING - invalid fileline [%1]").arg(s);
        else
        {
            if (rec.chain_name == m_currentChain) m_logData.append(rec);
        }
    }
    emit signalMsg(QString("Loaded %1 records from log file.").arg(logSize()));
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

    pid = current_tick = line_step = 0;
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
    s = QString("%1 / %2 / %3 / %4 / %5").arg(s).arg(tick_range).arg(price_range).arg(line_step).arg(note);
    return s;
}
void JSTxLogRecord::fromFileLine(const QString &fline)
{
    reset();
    if (fline.trimmed().isEmpty()) return;
    QStringList list = LString::trimSplitList(fline, "/");
    if (list.count() != 15) return;

    int i = 0;
    dt = QDateTime::fromString(list.at(i).trimmed(), "dd.MM.yyyy hh:mm:ss"); i++;
    chain_name = list.at(i).trimmed(); i++;
    tx_hash = list.at(i).trimmed(); i++;
    tx_kind = list.at(i).trimmed(); i++;
    pool_address = list.at(i).trimmed(); i++;
    token_address = list.at(i).trimmed(); i++;
    pid = list.at(i).trimmed().toULong(); i++;
    token0_size = list.at(i).trimmed().toFloat(); i++;
    token1_size = list.at(i).trimmed().toFloat(); i++;
    current_price = list.at(i).trimmed().toFloat(); i++;
    current_tick = list.at(i).trimmed().toInt(); i++;
    tick_range = list.at(i).trimmed(); i++;
    price_range = list.at(i).trimmed(); i++;
    line_step = list.at(i).trimmed().toUInt(); i++;
    note = list.at(i).trimmed(); i++;
}
bool JSTxLogRecord::invalid() const
{
    if (tx_hash.isEmpty() || tx_kind == "unknown" || chain_name.length() < 3) return true;
    if (!dt.isValid() || dt.date().year() < 2020) return true;
    return false;
}
void JSTxLogRecord::fromPriceRange(float &p1, float &p2) const
{
    p1 = p2 = -1;
    QStringList list = LString::trimSplitList(price_range, ":");
    if (list.count() == 2)
    {
        bool ok;
        p1 = list.at(0).toFloat(&ok);
        if (!ok || p1 < 0) p1 = -1;
        p2 = list.at(1).toFloat(&ok);
        if (!ok || p2 < 0) p2 = -1;
    }
}







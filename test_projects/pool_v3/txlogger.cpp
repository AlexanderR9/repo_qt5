#include "txlogger.h"
#include "lfile.h"
#include "ltime.h"
#include "lstring.h"
#include "appcommonsettings.h"
#include "deficonfig.h"


#include <QDir>
#include <QDebug>



//DefiTxLogger
DefiTxLogger::DefiTxLogger(QString chain_name, QObject *parent)
    :LSimpleObject(parent),
      m_chain(chain_name.trimmed().toLower())
{
    setObjectName("defi_tx_logger_obj");

}
void DefiTxLogger::reloadLogFiles()
{
    m_logData.clear();
    emit signalMsg("");
    emit signalMsg("Try load TX log-files data ........");

    loadTxListFile();
    loadTxDetailsFile();
    loadTxStateFile();
    //qDebug("reloadLogFiles finished");
}
void DefiTxLogger::loadTxListFile()
{
    //check file
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txListFile());
    if (!LFile::fileExists(fname)) {emit signalError(QString("DefiTxLogger: log file not found [%1]").arg(fname)); return;}

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(QString("DefiTxLogger: %1").arg(err)); return;}

    //parse file list
    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;
        if (fline.trimmed().at(0) == QChar('#')) continue;

        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 5) continue;

        TxLogRecord tx(rec_data.last(), rec_data.at(3));
        if (tx.chain_name != m_chain) continue;
        tx.tx_hash = rec_data.first();
        tx.setDateTime(rec_data.at(1), rec_data.at(2));
        if (!tx.invalid()) m_logData.append(tx);
    }
}
void DefiTxLogger::loadTxStateFile()
{
    //check file
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txStateFile());
    if (!LFile::fileExists(fname)) {emit signalError(QString("DefiTxLogger: log file not found [%1]").arg(fname)); return;}

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(QString("DefiTxLogger: %1").arg(err)); return;}

}
void DefiTxLogger::loadTxDetailsFile()
{
    //check file
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txDetailsFile());
    if (!LFile::fileExists(fname)) {emit signalError(QString("DefiTxLogger: log file not found [%1]").arg(fname)); return;}

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(QString("DefiTxLogger: %1").arg(err)); return;}

    //parse file data
    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;
        if (fline.trimmed().at(0) == QChar('#')) continue;
        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 3) continue;

        QString tx_hash = rec_data.first();
        for (int i=0; i<m_logData.count(); i++)
        {
            if (m_logData.at(i).tx_hash == tx_hash)
            {
                m_logData[i].parseDetails(rec_data.last());
                break;
            }
        }
    }
}
void DefiTxLogger::addNewRecord(const TxLogRecord &rec)
{
    QString tx_base = (rec.listFileLine().trimmed() + "\n");
    QString tx_details = (rec.detailsFileLine().trimmed() + "\n");
    if (tx_base.isEmpty() || tx_details.isEmpty()) {emit signalError("DefiTxLogger: can't add new tx_record to log-files"); return;}

    QString err;
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txListFile());
    if (!LFile::fileExists(fname)) err = LFile::writeFile(fname, tx_base);
    else err = LFile::appendFile(fname, tx_base);
    if (!err.isEmpty()) {emit signalError(QString("DefiTxLogger: can't add new tx_record to: %1").arg(fname)); return;}

    fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txDetailsFile());
    if (!LFile::fileExists(fname)) err = LFile::writeFile(fname, tx_details);
    else err = LFile::appendFile(fname, tx_details);
    if (!err.isEmpty()) {emit signalError(QString("DefiTxLogger: can't add new tx_record to: %1").arg(fname)); return;}
}
void DefiTxLogger::updateRecStatus(const QString &hash, QString status, float fee_native, quint32 gas_used)
{
    if (logEmpty()) return;

    int n = logSize();
    for (int i=n-1; i>=0; i--)
    {
        if (m_logData.at(i).tx_hash == hash)
        {
            m_logData[i].status.result = status;
            m_logData[i].status.fee_coin = fee_native;
            m_logData[i].status.gas_used = gas_used;

            float p = defi_config.lastPriceByTokenName(defi_config.nativeTokenName(m_chain));
            qDebug()<<QString("DefiTxLogger::updateRecStatus - cur price nativeTokenName %1").arg(p);
            if (p > 0) m_logData[i].status.fee_cent = (p*fee_native*float(100));
            else m_logData[i].status.fee_cent = -1;

            rewriteFileLineByRec(m_logData.at(i));

            break;
        }
    }
}
void DefiTxLogger::rewriteFileLineByRec(const TxLogRecord &rec)
{

}
const TxLogRecord* DefiTxLogger::recByHash(const QString &hash) const
{
    if (logEmpty()) return NULL;

    foreach (const TxLogRecord &v, m_logData)
        if (v.tx_hash == hash) return &v;
    return NULL;
}


/*
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
*/




/*
void DefiTxLogRecord::reset()
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
QString DefiTxLogRecord::toFileLine() const
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
void DefiTxLogRecord::fromFileLine(const QString &fline)
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
bool DefiTxLogRecord::invalid() const
{
    if (tx_hash.isEmpty() || tx_kind == "unknown" || chain_name.length() < 3) return true;
    if (!dt.isValid() || dt.date().year() < 2020) return true;
    return false;
}
void DefiTxLogRecord::fromPriceRange(float &p1, float &p2) const
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
*/







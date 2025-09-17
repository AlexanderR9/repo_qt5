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

    //parse file data
    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;
        if (fline.trimmed().at(0) == QChar('#')) continue;
        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 5) continue;

        QString tx_hash = rec_data.first();
        for (int i=0; i<m_logData.count(); i++)
        {
            if (m_logData.at(i).tx_hash == tx_hash)
            {
                m_logData[i].parseStatus(rec_data);
                break;
            }
        }
    }
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
    m_logData.append(rec);
    QString tx_base(m_logData.last().listFileLine().trimmed() + "\n");
    QString tx_details(m_logData.last().detailsFileLine().trimmed() + "\n");
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

    int pos = indexOf(hash);
    if (pos < 0) {emit signalError(QString("DefiTxLogger: not found rec in tx_log with hash [%1]").arg(hash)); return;}

    m_logData[pos].status.result = status;
    m_logData[pos].status.fee_coin = fee_native;
    m_logData[pos].status.gas_used = gas_used;

    float p = defi_config.lastPriceByTokenName(defi_config.nativeTokenName(m_chain));
    qDebug()<<QString("DefiTxLogger::updateRecStatus - cur price nativeTokenName %1").arg(p);
    if (p > 0) m_logData[pos].status.fee_cent = (p*fee_native*float(100));
    else m_logData[pos].status.fee_cent = -1;

    rewriteStatusFiles(m_logData.at(pos));
}
void DefiTxLogger::rewriteStatusFiles(const TxLogRecord &rec)
{
    if (!rec.isFinishedStatus()) return;

    // rewrite status file
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txStateFile());
    QString rewriting_line = rec.statusFileLine().trimmed();
    rewriting_line = LString::removeSymbol(rewriting_line, QChar('\n'));
    rewriteFileRecState(fname, rec, rewriting_line);

    // rewrite status file
    fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txDetailsFile());
    rewriting_line = rec.detailsFileLine().trimmed();
    rewriting_line = LString::removeSymbol(rewriting_line, QChar('\n'));
    rewriteFileRecState(fname, rec, rewriting_line);
}
void DefiTxLogger::rewriteFileRecState(QString fname, const TxLogRecord &rec, const QString &rewriting_line)
{
    if (fname.trimmed().isEmpty()) {emit signalError("DefiTxLogger: log filename is empty"); return;}

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(QString("DefiTxLogger: %1").arg(err));

    int hash_pos = LString::listIndexOf(fdata, rec.tx_hash.trimmed(), true);
    if (hash_pos < 0) fdata.append(rewriting_line);
    else fdata.replace(hash_pos, rewriting_line);

    LString::removeEmptyStrings(fdata);
    err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(QString("DefiTxLogger: %1").arg(err));
}
const TxLogRecord* DefiTxLogger::recByHash(const QString &hash) const
{
    if (logEmpty()) return NULL;

    foreach (const TxLogRecord &v, m_logData)
        if (v.tx_hash == hash) return &v;
    return NULL;
}
int DefiTxLogger::indexOf(const QString &hash) const
{
    if (logEmpty()) return -1;

    int n = logSize();
    for (int i=0; i<n; i++)
        if (m_logData.at(i).tx_hash == hash) return i;
    return -1;
}








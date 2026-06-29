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
void DefiTxLogger::mergeRecords(QString &err)
{
    err.clear();
    if (!LFile::dirExists(AppCommonSettings::mergeDataPath()))
    {
        err = QString("merge folder[%1] not found").arg(AppCommonSettings::mergeDataPath());
        return;
    }

    //load base tx_list file
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txListFile());
    QStringList fbasedata;
    err = LFile::readFileSL(fname, fbasedata);
    if (!err.isEmpty()) {err = QString("Base TX file [%1] not found").arg(fname); return;}
    QList<TxLogRecord> base_data;
    foreach (const QString &fline, fbasedata)
    {
        if (fline.trimmed().isEmpty() || fline.trimmed().at(0) == QChar('#')) continue;
        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 5) continue;

        // имя сети не проверяется
        TxLogRecord tx(rec_data.last(), rec_data.at(3));
        tx.tx_hash = rec_data.first().trimmed();
        tx.setDateTime(rec_data.at(1), rec_data.at(2));
        if (!tx.invalid()) base_data.append(tx); // запись корректна
    }
    emit signalMsg(QString("base records of tx_list file:  %1").arg(base_data.count()));


    // -------- load merge_data folder --------------
    QStringList txlist_list;
    QStringList txstate_list;
    QStringList txdetails_list;
    QString merge_path = QString("%1%2").arg(AppCommonSettings::mergeDataPath()).arg(QDir::separator());

    // load merge     txListFile
    fname = QString("%1%2").arg(merge_path).arg(AppCommonSettings::txListFile());
    if (!LFile::fileExists(fname)) {err = QString("Merging TX file [%1] not found").arg(fname); return;}
    err = LFile::readFileSL(fname, txlist_list);
    if (!err.isEmpty()) return;

    // load merge     txStateFile
    fname = QString("%1%2").arg(merge_path).arg(AppCommonSettings::txStateFile());
    if (!LFile::fileExists(fname)) {err = QString("Merging TX file [%1] not found").arg(fname); return;}
    err = LFile::readFileSL(fname, txstate_list);
    if (!err.isEmpty()) return;

    // load merge     txDetailsFile
    fname = QString("%1%2").arg(merge_path).arg(AppCommonSettings::txDetailsFile());
    if (!LFile::fileExists(fname)) {err = QString("Merging TX file [%1] not found").arg(fname); return;}
    err = LFile::readFileSL(fname, txdetails_list);
    if (!err.isEmpty()) return;

    emit signalMsg("Merging files readed OK!");
    emit signalMsg(QString("txlist_list size: %1").arg(txlist_list.count()));
    emit signalMsg(QString("txstate_list size: %1").arg(txstate_list.count()));
    emit signalMsg(QString("txdetails_list size: %1").arg(txdetails_list.count()));

    //parse merge data
    QList<TxLogRecord> merging_data;
    foreach (const QString &fline, txlist_list)
    {
        if (fline.trimmed().isEmpty() || fline.trimmed().at(0) == QChar('#')) continue;
        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 5) continue;

        // имя сети не проверяется
        TxLogRecord tx(rec_data.last(), rec_data.at(3));
        tx.tx_hash = rec_data.first().trimmed();
        tx.setDateTime(rec_data.at(1), rec_data.at(2));
        if (!tx.invalid()) // запись корректна
        {
            if (LString::listIndexOf(txstate_list, tx.tx_hash) >= 0 &&
                    LString::listIndexOf(txdetails_list, tx.tx_hash) >= 0) merging_data.append(tx);
        }
    }
    if (merging_data.isEmpty()) {err = "merging_data is empty"; return;}

    // ищем записи, хеши которых отсуствуют в основном наборе файлов tx_*
    QList<int> need_idexes;
    for (int i=0; i<merging_data.count(); i++)
    {
        if (hasRecordByHash(merging_data.at(i).tx_hash, base_data)) continue;
        qDebug()<<QString("NEED_MERGE_REC: %1 / %2").arg(merging_data.at(i).tx_hash).arg(merging_data.at(i).chain_name);
        need_idexes.append(i);
    }
    emit signalMsg(QString("find %1 records for merging").arg(need_idexes.count()));
    if (need_idexes.isEmpty()) {emit signalMsg("Base TX log-data is FULL!!"); return;} // добавлять нечего, журнал актуален

    int n = merging_data.count();
    for (int i=n-1; i>=0; i--)
        if (!need_idexes.contains(i)) merging_data.removeAt(i); // удаляем ненужные записи
    emit signalMsg(QString("merging_data size %1").arg(merging_data.count()));


    //merge and rewrite base files
    insertMergingRecords(merging_data, base_data);
    addDetailsMergingRecords(merging_data, txstate_list, txdetails_list);

}
void DefiTxLogger::removeRecord(const QString &hash, bool &ok)
{
    ok = false;
    int i_rec = indexOf(hash);
    if (i_rec < 0) return;

   // qDebug()<<QString("DefiTxLogger::removeRecord  finded tx_rec, index=%1, list size %2").arg(i_rec).arg(logSize());
    m_logData.removeAt(i_rec);
  //  qDebug()<<QString("list size %1").arg(logSize());

    ok = true;
    excludeRecFromFileByHash(hash, "tx_state.txt", ok);
    if (!ok) {qWarning("result: FALED"); return;}
    else qDebug("OK!");

    excludeRecFromFileByHash(hash, "tx_details.txt", ok);
    if (!ok) {qWarning("result: FALED"); return;}
    else qDebug("OK!");

    excludeRecFromFileByHash(hash, "tx_list.txt", ok);
    if (!ok) {qWarning("result: FALED"); return;}
    else qDebug("OK!");
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
    qDebug()<<QString("DefiTxLogger::updateRecStatus  hash[%1] status[%2]").arg(hash).arg(status);
    if (logEmpty()) return;


    int pos = indexOf(hash);
    if (pos < 0) {emit signalError(QString("DefiTxLogger: not found rec in tx_log with hash [%1]").arg(hash)); return;}
    qDebug()<<QString("find rec by pos %1").arg(pos);

    m_logData[pos].status.result = status.trimmed().toUpper();
    m_logData[pos].status.fee_coin = fee_native;
    m_logData[pos].status.gas_used = gas_used;

    float p = defi_config.lastPriceByTokenName(defi_config.nativeTokenName(m_chain));
    qDebug()<<QString("DefiTxLogger::updateRecStatus - cur price nativeTokenName %1").arg(p);
    if (p > 0) m_logData[pos].status.fee_cent = (p*fee_native*float(100));
    else m_logData[pos].status.fee_cent = -1;

    rewriteStatusFiles(m_logData.at(pos));
}
void DefiTxLogger::excludeRecFromFileByHash(const QString &hash, QString fname, bool &ok)
{
  //  qDebug()<<QString("try remove record from [%1].........").arg(fname);
    if (fname.trimmed().isEmpty()) return;
    fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(fname.trimmed());
    if (!LFile::fileExists(fname)) return;

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty())
    {
        emit signalError(err);
        ok = false;
        return;
    }
    if (fdata.isEmpty()) {qWarning("DefiTxLogger::excludeRecFromFileByHash WARNING fdata is empty"); return;}
    //qDebug()<<QString("fdata size %1").arg(fdata.count());

    QStringList fdata_next;
    bool fdata_has_hash = false;
    foreach (const QString &fline, fdata)
    {
        QString s(fline.trimmed());
        if (s.length() < 20) continue;
        if (!s.contains(hash)) fdata_next.append(fline);
        else fdata_has_hash = true; // найдена запись с таким хешом
    }
    if (fdata_next.isEmpty()) {qWarning("DefiTxLogger::excludeRecFromFileByHash WARNING fdata_next is empty"); return;}
    if (!fdata_has_hash) {qDebug("record not found in this file"); return;} // запись не найдена, перезаписывать файл не требуется

    //qDebug()<<QString("rewite file, fdata_next size %1").arg(fdata_next.count());
    err = LFile::writeFileSL(fname, fdata_next);
    if (!err.isEmpty()) {emit signalError(err); ok = false;}
    else ok = true;
}
void DefiTxLogger::rewriteStatusFiles(const TxLogRecord &rec)
{
    qDebug("DefiTxLogger::rewriteStatusFiles");
    if (!rec.isFinishedStatus()) {qWarning("WARNING: rec is not FinishedStatus"); return;}

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

    qDebug()<<QString("rewriteFileRecState  fname[%1]").arg(fname);
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
bool DefiTxLogger::hasRecord(const QString &hash) const
{
    return (indexOf(hash) >= 0);
}
void DefiTxLogger::insertMergingRecords(const QList<TxLogRecord> &merging_data, const QList<TxLogRecord> &base_data)
{
    if (merging_data.isEmpty()) return;

    QStringList fdata;
    QList<int> added_indexes;
    foreach (const TxLogRecord &base_rec, base_data)
    {
        if (!merging_data.isEmpty())
        {
            for (int i=0; i<merging_data.count(); i++)
            {
                if (added_indexes.contains(i)) continue;

                QDateTime ts = merging_data.at(i).dt;
                if (base_rec.dt > ts)
                {
                    fdata.append(merging_data.at(i).listFileLine());
                    added_indexes.append(i);
                }
            }
        }

        fdata.append(base_rec.listFileLine());
    }

    for (int i=0; i<merging_data.count(); i++)
    {
        if (!added_indexes.contains(i))
            fdata.append(merging_data.at(i).listFileLine());
    }

    emit signalMsg(QString("try rewite TX_list file, new records list %1").arg(fdata.count()));
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::txListFile());
    QString err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(QString("DefiTxLogger: %1").arg(err));

}
void DefiTxLogger::addDetailsMergingRecords(const QList<TxLogRecord> &merging_data, const QStringList &txstate_list, const QStringList &txdetails_list)
{
    QString path = QString("%1%2").arg(AppCommonSettings::appDataPath()).arg(QDir::separator());

    // merge txStateFile
    QString fname_state = QString("%1%2").arg(path).arg(AppCommonSettings::txStateFile());
    emit signalMsg(QString("try rewite TX file: %1").arg(AppCommonSettings::txStateFile()));
    addDetailsMergingRecordsToFile(fname_state, merging_data, txstate_list);

    // merge txDetailsFile
    QString fname_details = QString("%1%2").arg(path).arg(AppCommonSettings::txDetailsFile());
    emit signalMsg(QString("try rewite TX file: %1").arg(AppCommonSettings::txDetailsFile()));
    addDetailsMergingRecordsToFile(fname_details, merging_data, txdetails_list);
}
void DefiTxLogger::addDetailsMergingRecordsToFile(QString fname, const QList<TxLogRecord> &merging_data, const QStringList &details)
{
    QStringList add_data;
    foreach (const TxLogRecord &rec, merging_data)
    {
        int pos = LString::listIndexOf(details, rec.tx_hash);
        if (pos >= 0) add_data.append(details.at(pos));
    }
    qDebug()<<QString("fname[%1],  add_data size %2").arg(fname).arg(add_data.count());

    QString err = LFile::appendFileSL(fname, add_data);
    if (!err.isEmpty()) emit signalError(QString("DefiTxLogger: %1").arg(err));
}
bool DefiTxLogger::hasRecordByHash(const QString &hash, const QList<TxLogRecord> &recs_data)
{
    foreach (const TxLogRecord &rec, recs_data)
        if (rec.tx_hash == hash) return true;

    return false;
}




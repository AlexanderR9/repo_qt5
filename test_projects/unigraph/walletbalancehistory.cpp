#include "walletbalancehistory.h"
#include "lfile.h"
#include "lstring.h"
#include "ltime.h"
#include "subcommonsettings.h"


#include <QDir>
#include <QDebug>

#define JS_BALANCE_FILE          "defi/balance_history.txt"



// WalletBalanceHistory
WalletBalanceHistory::WalletBalanceHistory(QObject *parent)
    :LSimpleObject(parent)
{
    m_lastSnapshots.clear();

    loadHistoryFile();
    //qDebug()<<QString("------------WalletBalanceHistory::WalletBalanceHistory  m_lastSnapshots size %1").arg(m_lastSnapshots.count());
    out();
}
void WalletBalanceHistory::loadHistoryFile()
{
    m_lastSnapshots.clear();
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(JS_BALANCE_FILE);
    emit signalMsg(QString("try load local file [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("BALANCE local_file not found");
        return;
    }
    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    foreach (const QString &line, fdata)
    {
        qDebug()<<QString("loadHistoryFile fline[%1]").arg(line);
        SnapshotPointAsset rec;
        rec.fromFileLine(line); //парсим текущую строку файла
        if (!rec.invalid()) {initLastRecord(rec);} //если строка валидна, то оцениваем в текущим списком m_lastSnapshots
        else qWarning()<<QString("WalletBalanceHistory::loadHistoryFile WARNING: can't read file record [%1]").arg(line);
    }
}
void WalletBalanceHistory::initLastRecord(const SnapshotPointAsset &rec)
{
    //if (m_lastSnapshots.isEmpty()) {m_lastSnapshots.append(rec); return;}
    int i_asset = findAssetRec(rec.token_address);
    if (i_asset < 0)  {m_lastSnapshots.append(rec); return;} //нет записи с таким адресом

    if (rec.time > m_lastSnapshots.at(i_asset).time) //запись более свежая, нужно ее заменить
    {
        m_lastSnapshots[i_asset].setData(rec);
    }

    /*
    int n = m_lastSnapshots.count();
    int need_replace = -1;
    bool not_found = true;
    for (int i=0; i>n; i++)
    {
        if (m_lastSnapshots.at(i).token_address == rec.token_address)
        {
            not_found = false;
            if (rec.time > m_lastSnapshots.at(i).time) need_replace = i;
            break;
        }
    }
    if (not_found) {m_lastSnapshots.append(rec); return;}

    if (need_replace >= 0)
    {
        m_lastSnapshots.removeAt(need_replace);
        m_lastSnapshots.append(rec);
    }
    */
}
void WalletBalanceHistory::updateBalances(const QMap<QString, float> &map)
{
    qDebug()<<QString("WalletBalanceHistory::updateBalances  map size %1").arg(map.size());

    if (map.isEmpty()) return;
    QStringList addrs(map.keys()); //token addresses by wallet
    //QDateTime dt = QDateTime::currentDateTime();

    //записи для токенов в которых были обнаружены изменения баланса более чем на 0.01%
    //соответственно их нужно добавить в файл истории.
    QList<SnapshotPointAsset> new_records;
    foreach (const QString &addr, addrs)
    {
        float cur_balance = map.value(addr); //new received balance value
        compareTokenBalances(addr, cur_balance, new_records);

/*

        float last_balance = lastAssetBalance(addr);
        qDebug()<<QString("   %1: last_balance=%2  new_balance=%3").arg(addr).arg(last_balance).arg(cur_balance);
        if (last_balance < 0) //not found addr in m_lastSnapshots
        {
            qDebug("last_balance < 0,  rec not found");
            SnapshotPointAsset next_point(dt, addr, cur_balance);
            new_records.append(next_point);
            m_lastSnapshots.append(next_point);
        }
        else
        {
            float d = qAbs(last_balance - cur_balance);
            float p01 = last_balance/float(10000);
            qDebug()<<QString("d_balance=%1,  0.01% = %2").arg(d).arg(p01);
            if (d > p01) //need update last value
            {
                qDebug("      d > p01,  need update last value");
                SnapshotPointAsset next_point(dt, addr, cur_balance);
                new_records.append(next_point);
                int pos = findAssetRec(addr);
                if (pos >= 0) m_lastSnapshots.removeAt(pos);
                m_lastSnapshots.append(next_point);
            }
        }
        */
    }

    qDebug()<<QString("WalletBalanceHistory::updateBalances  new records %1").arg(new_records.count());
    out();

    appendFreshPointToFile(new_records);
}
void WalletBalanceHistory::compareTokenBalances(const QString &t_addr, float cur_balance, QList<SnapshotPointAsset> &new_records)
{
    int i_asset = findAssetRec(t_addr);
    if (i_asset < 0)  //нет записи с таким адресом
    {
        SnapshotPointAsset next_point(QDateTime::currentDateTime(), t_addr, cur_balance);
        new_records.append(next_point);
        m_lastSnapshots.append(next_point);
        return;
    }

    //compare
    float last_balance = m_lastSnapshots.at(i_asset).balance;
    qDebug()<<QString("   %1: last_balance=%2  new_balance=%3").arg(t_addr).arg(last_balance).arg(cur_balance);
    float d = qAbs(last_balance - cur_balance);
    float p01 = last_balance/float(10000);
    qDebug()<<QString("d_balance=%1,  0.01% = %2").arg(d).arg(p01);
    if (d > p01) //need update last value
    {
        qDebug("      d > p01,  need update last value");
        SnapshotPointAsset next_point(QDateTime::currentDateTime(), t_addr, cur_balance);
        new_records.append(next_point);
        m_lastSnapshots[i_asset].setData(next_point);
    }
}
float WalletBalanceHistory::lastAssetBalance(const QString &t_addr) const
{
    foreach (const SnapshotPointAsset &rec, m_lastSnapshots)
    {
        if (rec.token_address == t_addr) return rec.balance;
    }
    return -1;
}
int WalletBalanceHistory::findAssetRec(const QString &t_addr) const
{
    if (m_lastSnapshots.isEmpty()) return -1;
    for (int i=0; i<m_lastSnapshots.count(); i++)
        if (m_lastSnapshots.at(i).token_address == t_addr) return i;
    return -1;
}
void WalletBalanceHistory::appendFreshPointToFile(const QList<SnapshotPointAsset> &points)
{
    if (points.isEmpty()) return;

    //QString err;
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(JS_BALANCE_FILE);
    emit signalMsg(QString("try append new balances points to local file [%1].........").arg(fname));

    QStringList fdata;
    foreach (const SnapshotPointAsset &rec, points)
        fdata << rec.toFileLine();

    QString err = LFile::appendFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg("done!");
}
void WalletBalanceHistory::out()
{
    qDebug()<<QString("------------ WalletBalanceHistory: last_points %1 ----------------").arg(m_lastSnapshots.count());
    if (m_lastSnapshots.isEmpty()) return;

    foreach (const SnapshotPointAsset &rec, m_lastSnapshots)
        qDebug() << rec.toStr();

}




///////////////////////////////////////////
void WalletBalanceHistory::SnapshotPointAsset::fromFileLine(const QString &fline)
{
    reset();
    QString s = fline.trimmed();
    if (s.isEmpty()) return;

    QStringList list = LString::trimSplitList(s, "/");
    if (list.count() == 3)
    {
        time = QDateTime::fromString(list.first().trimmed(), timeMask());
        token_address = list.at(1).trimmed().toLower();
        bool ok = false;
        balance = list.last().trimmed().toFloat(&ok);
        if (!ok) balance = -1;
    }
}
bool WalletBalanceHistory::SnapshotPointAsset::invalid() const
{
    if (!time.isValid() || balance < 0) return true;
    if (token_address.length()<30) return true;
    return false;
}
void WalletBalanceHistory::SnapshotPointAsset::setData(const SnapshotPointAsset &other)
{
    time = other.time;
    token_address = other.token_address;
    balance = other.balance;
}
QString WalletBalanceHistory::SnapshotPointAsset::toFileLine() const
{
    if (invalid()) return "invalid";

    QString fline = time.toString(timeMask());
    fline = QString("%1 / %2 / %3").arg(fline).arg(token_address).arg(QString::number(balance, 'f', 8));
    return fline;
}
QString WalletBalanceHistory::SnapshotPointAsset::toStr() const
{
    QString s("WalletBalanceHistory:");
    if (invalid()) return QString("%1  INVALID_STATE").arg(s);
    s = QString("%1  token[%2]  ts[%3]").arg(s).arg(token_address).arg(time.toString(timeMask()));
    s = QString("%1  BALANCE=%2").arg(s).arg(QString::number(balance, 'f', 4));
    return s;
}


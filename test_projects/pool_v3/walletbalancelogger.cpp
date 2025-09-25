#include "walletbalancelogger.h"
#include "appcommonsettings.h"
#include "lfile.h"
#include "lstring.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>

#define LOG_BALANCE_PRECISION       6


//WalletBalanceLogger
WalletBalanceLogger::WalletBalanceLogger(const QStringList &t_list, QObject *parent)
    :LSimpleObject(parent),
      m_sensivity(1.5),
      m_chain(QString("?")),
      m_trackingTokens(t_list)
{
    setObjectName("balances_logger_obj");
    m_log.clear();

}
void WalletBalanceLogger::loadLogFile(QString chain_name)
{
    m_log.clear();
    m_chain = chain_name.trimmed().toLower();

    //check file
    QString fname = balanceFileName();
    if (!LFile::fileExists(fname))
    {
        emit signalError(QString("WalletBalanceLogger: log file not found [%1]").arg(fname));
        initEmptyFile();
    }

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(QString("WalletBalanceLogger: %1").arg(err)); return;}

    parseFileData(fdata);
    checkAbsentBalances();
}
void WalletBalanceLogger::parseFileData(const QStringList &fdata)
{
    bool ok = false;
    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;

        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 4) continue;
        if (rec_data.at(1).trimmed() != m_chain) continue;

        TokenBalanceRecord rec(m_chain);
        rec.name = rec_data.at(0);
        rec.time_point = rec_data.at(3).toUInt();
        rec.balance = rec_data.at(2).toFloat(&ok);
        if (!ok) signalError(QString("WalletBalanceLogger: can't convert (%1) to float balance").arg(rec_data.at(2)));
        else m_log.append(rec);
    }
}
void WalletBalanceLogger::initEmptyFile()
{
    QString fname = balanceFileName();
    qDebug()<<QString("WalletBalanceLogger::initEmptyFile() [%1]").arg(fname);
    emit signalMsg("creating log file .....");
    QString err = LFile::fileCreate(fname);
    if (err.isEmpty()) emit signalMsg("done!");
    else emit signalError(QString("WalletBalanceLogger: %1").arg(err));
}
QString WalletBalanceLogger::balanceFileName() const
{
    return QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::balanceLogFile());;
}
void WalletBalanceLogger::checkAbsentBalances()
{
    foreach (const QString &token, m_trackingTokens)
    {
        if (lastBalance(token) < 0) // need add zero recors
        {
            addNewRecToFile(token, 0);
        }
    }
}
float WalletBalanceLogger::lastBalance(QString t_name) const
{
    if (m_log.isEmpty()) return -1;

    int n = logSize();
    for (int i=n-1; i>=0; i--)
        if (m_log.at(i).name == t_name) return m_log.at(i).balance;

    return -1;
}
void WalletBalanceLogger::addNewRecToFile(const QString &token, float cur_balance)
{
    TokenBalanceRecord rec(m_chain);
    rec.name = token;
    rec.time_point = QDateTime::currentDateTime().toSecsSinceEpoch();
    rec.balance = cur_balance;

    QString fline = QString("%1 \n").arg(rec.fileLine());
    QString fname = balanceFileName();
    QString err = LFile::appendFile(fname, fline);
    if (!err.isEmpty()) emit signalError(QString("WalletBalanceLogger: %1").arg(err));

    m_log.append(rec);
}
float WalletBalanceLogger::recBalanceDeviation(int i_rec) const
{
    if (m_log.isEmpty()) return 0;

    int n = logSize();
    if (i_rec < 1 || i_rec >= n) return 0;

    const TokenBalanceRecord &rec =  recAt(i_rec);
    for (int i=(i_rec-1); i>=0; i--)
    {
        if (recAt(i).name == rec.name)
            return (rec.balance - recAt(i).balance);
    }
    return 0;
}
void WalletBalanceLogger::setSensivityChainging(float p)
{
    if (p < 0.1) m_sensivity = 0.1;
    else if (p > 10) m_sensivity = 10;
    else m_sensivity = p;
}
void WalletBalanceLogger::receiveNewValue(const QString &token, float cur_amount)
{
    if (!m_trackingTokens.contains(token)) return;

    float v_last = lastBalance(token);
    if ((cur_amount == 0) && (v_last == 0)) return;

    float d = cur_amount - v_last;
    float d_per = ((v_last == 0) ? 100 : (d*float(100)/v_last));
    if (qAbs(d_per) > m_sensivity) // need new rec
    {
        addNewRecToFile(token, cur_amount);
        emit signalAddNewRecord(d);
    }
}




///////////////////////////////////////////
QString WalletBalanceLogger::TokenBalanceRecord::fileLine() const
{
    QString s = QString("%1 / %2 / %3").arg(name).arg(chain).arg(QString::number(balance, 'f', LOG_BALANCE_PRECISION));
    return QString("%1 / %2").arg(s).arg(time_point);
}



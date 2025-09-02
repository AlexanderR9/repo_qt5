#include "tokenpricelogger.h"
#include "lfile.h"
#include "lstring.h"
#include "appcommonsettings.h"
#include "deficonfig.h"


#include <QDir>
#include <QDebug>
#include <QDateTime>


#define LOG_PRICE_PRECISION          4
#define SENSITIVITY_LIMIT            0.5 // %


//TokenPriceLogger
TokenPriceLogger::TokenPriceLogger(QObject *parent)
    :LSimpleObject(parent)
{
    setObjectName("token_prices_logger_obj");
    m_log.clear();

    //loadFile();
}
void TokenPriceLogger::loadLogFile()
{
    //check file
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::priceLogFile());
    if (!LFile::fileExists(fname)) {emit signalError(QString("TokenPriceLogger: log file not found [%1]").arg(fname)); return;}

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(QString("TokenPriceLogger: %1").arg(err)); return;}

    //parse file list
    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;
        if (fline.trimmed().at(0) == QChar('#')) continue;

        QStringList rec_data = LString::trimSplitList(fline, "/");
        if (rec_data.count() != 4) continue;

        TokenPriceRecord rec;
        rec.name = rec_data.at(0);
        rec.address = rec_data.at(1);
        rec.price = rec_data.at(2).toFloat();
        rec.time_point = rec_data.at(3).toUInt();
        m_log.append(rec);
    }

    qDebug()<<QString("TokenPriceLogger::loadFile() loaded %1 records").arg(logSize());
}
void TokenPriceLogger::checkLastPrices()
{
    for (int j=0; j<defi_config.tokens.count(); j++)
    {
        if (defi_config.tokens.at(j).is_stable) continue;

        QString t_name = defi_config.tokens.at(j).name;
        QString t_addr = defi_config.tokens.at(j).address;
        float t_price = defi_config.tokens.at(j).last_price;
        if (t_price <= 0) continue;

        float own_price = getLastPrice(t_name, t_addr);
        if (own_price < 0 || priceDeviation(own_price, t_price) > SENSITIVITY_LIMIT)
        {
            // need create new log record
            TokenPriceRecord new_rec;
            new_rec.name = t_name;
            new_rec.address = t_addr;
            new_rec.price = t_price;
            new_rec.time_point = QDateTime::currentDateTime().toSecsSinceEpoch();
            m_log.append(new_rec);

            addLastRecToFile();
        }
    }
}
float TokenPriceLogger::priceDeviation(float prev, float cur) const
{
    if (prev <= 0 || cur <= 0) return -1;
    float d = cur - prev;
    return (float(100)*(d/prev));
}
float TokenPriceLogger::getLastPrice(QString t_name, QString t_addr) const
{
    if (logEmpty()) return -1;

    int n = logSize();
    for (int i=n-1; i>=0; i--)
    {
        const TokenPriceRecord &rec = m_log.at(i);
        if (rec.name == t_name && rec.address == t_addr) return rec.price;
    }
    return -1;
}
void TokenPriceLogger::addLastRecToFile()
{
    const TokenPriceRecord &rec = m_log.last();
    QString fline = QString("%1 / %2 / %3").arg(rec.name).arg(rec.address).arg(QString::number(rec.price, 'f', LOG_PRICE_PRECISION));
    fline = QString("%1 / %2 \n").arg(fline).arg(rec.time_point);

    QString err;
    QString fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(AppCommonSettings::priceLogFile());
    if (!LFile::fileExists(fname)) err = LFile::writeFile(fname, fline);
    else err = LFile::appendFile(fname, fline);
    if (!err.isEmpty()) {emit signalError(QString("TokenPriceLogger: can't add new price_record to: %1").arg(fname)); return;}
}


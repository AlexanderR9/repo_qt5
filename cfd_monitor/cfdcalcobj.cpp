#include "cfdcalcobj.h"
#include "lstatic.h"
#include "lfile.h"

#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QApplication>

#define CFD_DATA_FILETYPE   "txt"
#define CFD_DATA_FOLDER     "data"


//CFDCalcObj
CFDCalcObj::CFDCalcObj(QObject *parent)
    :LSimpleObject(parent)
{

}
void CFDCalcObj::slotNewPrice(QString ticker, double price)
{
    loadTickerFile(ticker);
    addToFile(ticker, price);

    QStringList list;
    list << ticker << QString::number(price, 'f', 1) << "---" << "---" << "---";
    list.append(LStatic::strCurrentDateTime());
    emit signalUpdateCFDTable(list);

}
void CFDCalcObj::addToFile(const QString &ticker, double &price)
{
    CFDFileRecord rec(QTime::currentTime(), price);
    QDateTime dt = QDateTime::currentDateTime();
    rec.year = dt.date().year();
    rec.month = dt.date().month();
    rec.day = dt.date().day();


    if (m_currentData.isEmpty())
    {

    }
    else
    {

    }

    m_currentData.append(rec);
}
void CFDCalcObj::loadTickerFile(const QString &ticker)
{
    m_currentData.clear();
    QString fname = filePathByTicker(ticker);

    QStringList list;
    QString err = LFile::readFileSL(fname, list);
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }


}
QString CFDCalcObj::filePathByTicker(const QString &ticker) const
{
    QString path = QString("%1%2%3").arg(QApplication::applicationDirPath()).arg(QDir::separator()).arg(CFD_DATA_FOLDER);
    path = QString("%1%2%3.%4").arg(path).arg(QDir::separator()).arg(ticker).arg(CFD_DATA_FILETYPE);
    return path;
}


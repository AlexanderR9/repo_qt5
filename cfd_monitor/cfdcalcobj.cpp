#include "cfdcalcobj.h"
#include "lstatic.h"
#include "lfile.h"

#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QApplication>

#define CFD_DATA_FILETYPE   "txt"
#define CFD_DATA_FOLDER     "data"
#define CFD_DATA_TIMEFORMAT "hh:mm"


//CFDCalcObj
CFDCalcObj::CFDCalcObj(QObject *parent)
    :LSimpleObject(parent)
{

}
void CFDCalcObj::slotNewPrice(QString ticker, double price)
{
    loadTickerFile(ticker);

    bool ok;
    addToFile(ticker, price, ok);

    QStringList list;
    list << ticker << QString::number(price, 'f', 2) << "---" << "---" << "---";
    if (!ok) list.append("err");
    else list.append(LStatic::strCurrentDateTime());
    emit signalUpdateCFDTable(list);

}
void CFDCalcObj::addToFile(const QString &ticker, double &price, bool &ok)
{
    ok = false;
    CFDFileRecord rec(QTime::currentTime(), price);
    QDateTime dt = QDateTime::currentDateTime();
    rec.year = dt.date().year();
    rec.month = dt.date().month();
    rec.day = dt.date().day();
    if (rec.invalid()) return;


    m_currentData.append(rec);
    saveTickerFile(ticker);
    ok = true;
}
void CFDCalcObj::saveTickerFile(const QString &ticker)
{
    if (m_currentData.isEmpty()) return;
    if (ticker.trimmed().isEmpty()) return;
    QString fname = filePathByTicker(ticker);

    QStringList f_data;

    int cur_year = -1;
    int cur_month = -1;
    int cur_day = -1;
    for (int i=0; i<m_currentData.count(); i++)
    {
        const CFDFileRecord &rec = m_currentData.at(i);
        if (cur_year < 0)
        {
            cur_year = rec.year;
            f_data.append(QString("[year : %1]").arg(rec.year));
            cur_day = -1;
            cur_month = -1;
            continue;
        }
        if (cur_year != rec.year && cur_year == (rec.year-1))
        {
            if (cur_year != (rec.year-1))
            {
                qWarning()<<QString("CFDCalcObj::saveTickerFile WARNING next_year(%1) invalid").arg(rec.year);
                break;
            }
            cur_year = rec.year;
            f_data.append(QString("[year : %1]").arg(rec.year));
            cur_day = -1;
            cur_month = -1;
            continue;
        }

        //fill month data
        cur_month = rec.month;
        f_data.append(QString("[month : %1]").arg(rec.month));
        int n_days = QDate(cur_year, cur_month, 1).daysInMonth();
        for (cur_day=1; cur_day<=n_days; cur_day++)
        {
            QString day_prices;
            if (i >= m_currentData.count() || cur_month != m_currentData.at(i).month)
            {
                f_data.append(day_prices);
                continue;
            }
            while (cur_day == m_currentData.at(i).day)
            {
                day_prices = QString("%1 %2(%3);").arg(day_prices).arg(QString::number(m_currentData.at(i).price, 'f', 2)).arg(m_currentData.at(i).time.toString(CFD_DATA_TIMEFORMAT));
                i++;
                if (i >= m_currentData.count()) break;
            }
            f_data.append(day_prices);
        }
    }

    QString err = LFile::writeFileSL(fname, f_data);
    if (!err.isEmpty()) emit signalError(err);

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

    int cur_year = -1;
    int cur_month = -1;
    int cur_day = -1;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.isEmpty()) continue;

        if (cur_year < 0)
        {
            cur_year = CFDFileRecord::getYearFileValue(s);
            cur_day = -1;
            cur_month = -1;
            continue;
        }
        if (s.contains("year"))
        {
            int a = CFDFileRecord::getYearFileValue(s);
            if (a == (cur_year+1)) cur_year++;
            else
            {
                qWarning()<<QString("CFDCalcObj::loadTickerFile WARNING next_year(%1) invalid").arg(a);
                break;
            }
            cur_month = -1;
            cur_day = -1;
            continue;
        }
        if (cur_month < 0 || s.contains("month"))
        {
            cur_month = CFDFileRecord::getMonthFileValue(s);
            cur_day = 0;
            continue;
        }

        cur_day++;
        if (cur_day > QDate(cur_year, cur_month, 1).daysInMonth())
        {
            qWarning()<<QString("CFDCalcObj::loadTickerFile WARNING cur_day(%1) > daysInMonth(%2)").arg(cur_day).arg(QDate(cur_year, cur_month, 1).daysInMonth());
            cur_day = -1;
            cur_month = -1;
            continue;
        }

        QStringList p_list = LStatic::trimSplitList(s, ";");
        for (int j=0; j<p_list.count(); j++)
        {
            CFDFileRecord rec(cur_year, cur_month, cur_day);
            CFDFileRecord::getPricePointFileValue(p_list.at(j), rec.price, rec.time);
            if (!rec.invalid()) m_currentData.append(rec);
        }
    }
}
QString CFDCalcObj::filePathByTicker(const QString &ticker) const
{
    QString path = QString("%1%2%3").arg(QApplication::applicationDirPath()).arg(QDir::separator()).arg(CFD_DATA_FOLDER);
    path = QString("%1%2%3.%4").arg(path).arg(QDir::separator()).arg(ticker).arg(CFD_DATA_FILETYPE);
    return path;
}

///////////////////////////////////////////
int CFDFileRecord::getYearFileValue(const QString &s)
{
    QString s1 = LStatic::strBetweenStr(s, "[", "]");
    if (s1.isEmpty()) return -1;

    QStringList list = LStatic::trimSplitList(s1, ":");
    if (list.count() != 2) return -1;
    if (list.first() != "year") return -1;

    bool ok;
    int a = list.last().toInt(&ok);
    if (!ok || a < 2000 || a > QDate::currentDate().year()) return -1;
    return a;
}
int CFDFileRecord::getMonthFileValue(const QString &s)
{
    QString s1 = LStatic::strBetweenStr(s, "[", "]");
    if (s1.isEmpty()) return -1;

    QStringList list = LStatic::trimSplitList(s1, ":");
    if (list.count() != 2) return -1;
    if (list.first() != "month") return -1;

    bool ok;
    int a = list.last().toInt(&ok);
    if (!ok || a < 1 || a > 12) return -1;
    return a;
}
void CFDFileRecord::getPricePointFileValue(const QString &s, double &p, QTime &t)
{
    p = -1;
    if (s.trimmed().isEmpty()) return;

    int pos = s.indexOf("(");
    if (pos < 0 || s.right(1) != ")") return;

    bool ok;
    p = s.left(pos).toDouble(&ok);
    QString s_time = LStatic::strBetweenStr(s, "(", ")").trimmed();
    t = QTime::fromString(s_time, CFD_DATA_TIMEFORMAT);

    if (!t.isValid() || t.isNull() || p < 0) p = -1;
}
bool CFDFileRecord::invalid() const
{
    if (year == 0 || month == 0 || day == 0) return true;
    if (!time.isValid() || time.isNull() || price <= 0) return true;
    return false;
}

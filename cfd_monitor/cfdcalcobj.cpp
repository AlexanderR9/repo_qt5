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
#define CFD_DATA_DATEFORMAT "dd.MM.yyyy"


//CFDCalcObj
CFDCalcObj::CFDCalcObj(QObject *parent)
    :LSimpleObject(parent)
{

}
void CFDCalcObj::slotGetLastPrice(const QString &ticker, double &price, int &hours_ago)
{
    price = -1;
    hours_ago = -1;
    loadTickerFile(ticker); //загрузить текущие данные из файла для ticker
    if (!m_currentData.isEmpty())
    {
        price = m_currentData.last().price;
        hours_ago = m_currentData.last().dt().secsTo(QDateTime::currentDateTime())/3600;
    }
}
void CFDCalcObj::slotNewPrice(QString ticker, double price)
{
    loadTickerFile(ticker); //загрузить текущие данные из файла для ticker
    if (!needRecalc(price))
    {
        qDebug()<<QString("NOT NEED RECACL price_now - price_last < 0.05 cent");
        return; //цена почти не изменилась
    }

    bool ok;
    addToFile(ticker, price, ok); //добавить новую точку в файл и в контейнер m_currentData

    QStringList list;
    if (!ok) //произошла ошибка при добавлении точки в файл
    {
        list.append(QDate::currentDate().toString(CFD_DATA_DATEFORMAT));
        list.append(QTime::currentTime().toString(CFD_DATA_TIMEFORMAT));
        list.append(ticker);
        list << "---" << "---" << "---" << "??";
    }
    else //пересчитать текущие отклонения цены с учетом новых данных
    {
        QList<double> list_to_bot;

        list.append(m_currentData.last().date.toString(CFD_DATA_DATEFORMAT));
        list.append(m_currentData.last().time.toString(CFD_DATA_TIMEFORMAT));
        list.append(ticker);
        list.append(changingPriceByPeriod(0));
        list_to_bot.append(list.last().toDouble(&ok)); if (!ok) list_to_bot[0] = 0;
        list.append(changingPriceByPeriod(1));
        list_to_bot.append(list.last().toDouble(&ok)); if (!ok) list_to_bot[1] = 0;
        list.append(changingPriceByPeriod(4));
        list_to_bot.append(list.last().toDouble(&ok)); if (!ok) list_to_bot[2] = 0;
        list.append(QString::number(m_currentData.last().price, 'f', 2));

        emit signalInfoToBot(ticker, list_to_bot);

    }
    emit signalUpdateCFDTable(list); //отправить последние результаты на страницу CFDPage
}
QString CFDCalcObj::changingPriceByPeriod(uint hours) const
{
    QString s("---");
    if (m_currentData.count() < 2) return s;

    QDateTime last_dt(m_currentData.last().dt());
    int i = m_currentData.count() - 2;
    while (i >= 0)
    {
        qint64 d_sec = m_currentData.at(i).dt().secsTo(last_dt);
        if (d_sec > hours*3600) break;
        i--;
    }

    if (i >= 0)
    {
        double last_price = m_currentData.last().price;
        double period_price = m_currentData.at(i).price;
        if (last_price < 0.1 || last_price < 0.1) return "err";
        double d = 100*(last_price - period_price)/period_price;
        s.clear();
        if (d < -0.1) {}
        else if (d > 0.1) s = "+";
        else d = 0;
        s = QString("%1%2%").arg(s).arg(QString::number(d, 'f', 1));
    }
    return s;
}
bool CFDCalcObj::needRecalc(const double &price) const
{
    if (m_currentData.isEmpty()) return true;
    if (m_currentData.last().invalid()) return true;

    if (qAbs(m_currentData.last().price - price) < 0.05)
    {
        if (m_currentData.last().dt().secsTo(QDateTime::currentDateTime()) < (3600*24)) return false;
    }

    return true;
}
void CFDCalcObj::addToFile(const QString &ticker, const double &price, bool &ok)
{
    ok = false;
    CFDFileRecord rec(QDateTime::currentDateTime(), price);
    if (rec.invalid()) return;

    m_currentData.append(rec);
    appendTickerFile(ticker, rec);
    ok = true;
}
void CFDCalcObj::appendTickerFile(const QString &ticker, const CFDFileRecord &rec)
{
    if (m_currentData.isEmpty()) return;
    if (ticker.trimmed().isEmpty()) return;
    QString fname = filePathByTicker(ticker);

    QString f_data(rec.toFileLine());
    QString err = LFile::appendFile(fname, f_data);
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

    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.isEmpty()) continue;

        CFDFileRecord rec;
        rec.parseFileLine(s.trimmed());
        if (!rec.invalid()) m_currentData.append(rec);
    }

    qDebug()<<QString("CFDCalcObj::loadTickerFile  ticker=[%1]  records count %2").arg(ticker).arg(m_currentData.count());
}
QString CFDCalcObj::filePathByTicker(const QString &ticker) const
{
    QString path = QString("%1%2%3").arg(QApplication::applicationDirPath()).arg(QDir::separator()).arg(CFD_DATA_FOLDER);
    path = QString("%1%2%3.%4").arg(path).arg(QDir::separator()).arg(ticker).arg(CFD_DATA_FILETYPE);
    return path;
}

//CFDFileRecord
void CFDFileRecord::parseFileLine(const QString &s)
{
    reset();
    QString s1 = LStatic::strBetweenStr(s, "(", ")").trimmed();
    if (s1.isEmpty()) return;

    int pos = s1.indexOf(LStatic::spaceSymbol());
    if (pos < 5) return;

    date = QDate::fromString(s1.left(pos), CFD_DATA_DATEFORMAT);
    s1 = LStatic::strTrimLeft(s1, pos);
    time = QTime::fromString(s1.trimmed(), CFD_DATA_TIMEFORMAT);

    bool ok;
    pos = s.indexOf("(");
    price = s.left(pos).trimmed().toDouble(&ok);
    if (!ok || price <= 0) reset();
}
QString CFDFileRecord::toFileLine() const
{
    QString s = QString::number(price, 'f', 2);
    s = QString("%1(%2 %3)").arg(s).arg(date.toString(CFD_DATA_DATEFORMAT)).arg(time.toString(CFD_DATA_TIMEFORMAT));
    return QString("%1 \n").arg(s);
}
void CFDFileRecord::reset()
{
    date = QDate();
    time = QTime();
    price = -1;
}
bool CFDFileRecord::invalid() const
{
    if (!date.isValid() || date.isNull()) return true;
    if (!time.isValid() || time.isNull() || price <= 0) return true;
    return false;
}

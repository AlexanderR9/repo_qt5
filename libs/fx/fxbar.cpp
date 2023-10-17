#include "fxbar.h"
#include "lstring.h"

#include <QDebug>

#define FX_DATE_FORMAT      QString("yyyy.MM.dd")
#define FX_TIME_FORMAT      QString("hh:mm")


// FXBar
FXBar::FXBar()
{
    m_digist = 4;
    reset();
}
FXBar::FXBar(quint8 d)
    :m_digist(d)
{
    reset();
}
void FXBar::reset()
{
    m_open = m_close = m_high = m_low = -1;
    m_volume = 0;
    m_time = QDateTime();
}
void FXBar::fromFileLine(const QString &fline, QString sep_values)
{
    reset();
    QString s = fline.trimmed();
    if (s.isEmpty()) return;

    QStringList list = LString::trimSplitList(s, sep_values);
    if (list.count() != 7)
    {
        qWarning()<<QString("FXBar::fromFileLine  WARNING: invalid list values count(%1) != 7,   fline=[%1]").arg(list.count()).arg(fline);
        return;
    }

    //qDebug()<<QString("FXBar::fromFileLine:  ")<<fline;

    int k = 0;
    bool ok;
    m_time.setDate(QDate::fromString(list.at(k), FX_DATE_FORMAT)); k++;
    m_time.setTime(QTime::fromString(list.at(k), FX_TIME_FORMAT)); k++;

    m_open = list.at(k).toDouble(&ok); k++;
    if (!ok || m_open <= 0) m_open = -1;
    m_high = list.at(k).toDouble(&ok); k++;
    if (!ok || m_high <= 0) m_high = -1;
    m_low = list.at(k).toDouble(&ok); k++;
    if (!ok || m_low <= 0) m_low = -1;
    m_close = list.at(k).toDouble(&ok); k++;
    if (!ok || m_close <= 0) m_close = -1;

    m_volume = list.at(k).toUInt(&ok); k++;
    if (!ok) m_volume = 0;

    //qDebug()<<toStr();
}
bool FXBar::invalid() const
{
    if (m_open <= 0 || m_close <= 0 || m_high <= 0 || m_low <= 0) return true;
    if (m_open < m_low || m_open > m_high || m_close < m_low || m_close > m_high) return true;
    if (m_time.isNull() || !m_time.isValid()) return true;
    if (m_time.date().year() < 2000)  return true;
    return false;
}
QString FXBar::toStr() const
{
    QString s("FXBar data:");
    s = QString("%1 (%2 %3)").arg(s).arg(m_time.date().toString(FX_DATE_FORMAT)).arg(m_time.time().toString(FX_TIME_FORMAT));
    s = QString("%1   open/close=%2/%3").arg(s).arg(QString::number(m_open, 'f', m_digist)).arg(QString::number(m_close, 'f', m_digist));
    s = QString("%1   high/low=%2/%3").arg(s).arg(QString::number(m_high, 'f', m_digist)).arg(QString::number(m_low, 'f', m_digist));
    return s;
}
QString FXBar::strTime() const
{
    return QString("%1 %2").arg(m_time.date().toString(FX_DATE_FORMAT)).arg(m_time.time().toString(FX_TIME_FORMAT));
}



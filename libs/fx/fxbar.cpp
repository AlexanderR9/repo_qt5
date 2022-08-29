#include "fxbar.h"
#include "lstatic.h"
#include "lfile.h"
#include "fxenums.h"

#include <QDebug>

#define FX_DATE_FORMAT      QString("yyyy.MM.dd")
#define FX_TIME_FORMAT      QString("hh:mm")
#define FX_FILE_FORMAT      QString(".csv")


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

    QStringList list = LStatic::trimSplitList(s, sep_values);
    if (list.count() != 7)
    {
        qWarning()<<QString("FXBar::fromFileLine  WARNING: invalid list values count(%1) != 7,   fline=[%1]").arg(list.count()).arg(fline);
        return;
    }

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
}
bool FXBar::invalid() const
{
    if (m_open <= 0 || m_close <= 0 || m_high <= 0 || m_low <= 0) return true;
    if (m_high < m_low || m_open > m_low || m_close > m_low) return true;
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






//FXBarContainer
FXBarContainer::FXBarContainer(const QString &fname, QObject *parent)
    :LSimpleObject(parent),
    m_fileName(fname.trimmed())
{
    reset();
}
void FXBarContainer::reset()
{
    m_data.clear();
    m_timeframe = -1;
    m_couple.clear();
    m_digist = 2;
}
void FXBarContainer::tryLoadData(QString sep_values)
{
    reset();
    checkFileName();
    if (invalid()) return;

    emit signalMsg(QString("Try load datafile: %1 ......").arg(m_fileName));
    QStringList list;
    QString err = LFile::readFileSL(m_fileName, list);
    if (!err.isEmpty()) {emit signalError(err); return;}

    int invalid_bars = 0;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.isEmpty()) continue;
        FXBar bar(m_digist);
        bar.fromFileLine(s, sep_values);
        if (bar.invalid()) invalid_bars++;
        else m_data.append(bar);
    }

    emit signalMsg(toStr());
}
void FXBarContainer::checkFileName()
{
    if (m_fileName.isEmpty())
    {
        emit signalError("FXBarContainer: datafile name is empty");
        return;
    }

    QString short_name = LFile::shortDirName(m_fileName);
    if (short_name.right(FX_FILE_FORMAT.length()) != FX_FILE_FORMAT)
    {
        emit signalError(QString("FXBarContainer: datafile(%1) is not [%1] type").arg(m_fileName).arg(FX_FILE_FORMAT));
        return;
    }
    if (!LFile::fileExists(m_fileName))
    {
        emit signalError(QString("FXBarContainer: datafile(%1) not found").arg(m_fileName));
        return;
    }

    QString s = LStatic::strTrimRight(short_name, FX_FILE_FORMAT.length());
    QStringList list = LStatic::trimSplitList(s, QString("_"));
    if (list.count() != 3)
    {
        emit signalError(QString("FXBarContainer: datafile(%1) invalid format, must be: couplename_timeframe_digist.csv").arg(short_name));
        return;
    }

    bool ok;
    m_couple = list.first().trimmed();
    m_timeframe = list.at(1).toInt(&ok);
    if (!ok || !FXEnumStaticObj::timeFrames().contains(m_timeframe))
    {
        m_timeframe = -1;
        emit signalError(QString("FXBarContainer: datafile(%1) invalid timeframe %1").arg(short_name).arg(list.at(1)));
        return;
    }
    m_digist = list.at(2).toUInt();
    if (!ok || m_digist > 8)
    {
        m_digist = 99;
        emit signalError(QString("FXBarContainer: datafile(%1) invalid digist %1").arg(short_name).arg(list.at(2)));
        return;
    }
}
bool FXBarContainer::invalid() const
{
    if (m_couple.isEmpty() || m_timeframe <= 0 || m_digist > 8) return true;
    return m_data.isEmpty();
}
QString FXBarContainer::toStr() const
{
    QString s("FXBarContainer:");
    s = QString("%1 couple=%2  timeframe=%3  digist=%4").arg(s).arg(m_couple).arg(m_timeframe).arg(m_digist);
    s = QString("%1    DATA_SIZE %2").arg(s).arg(m_data.count());
    return s;
}



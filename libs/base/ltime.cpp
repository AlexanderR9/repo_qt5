#include "ltime.h"
#include <time.h>

#include <QDataStream>
#include <QDebug>
#include <QTimeZone>



//LTime
QString LTime::strCurrentTime(QString mask)
{
    return QTime::currentTime().toString(mask);
}
QString LTime::strCurrentDateTime(QString mask)
{
    return QDateTime::currentDateTime().toString(mask);
}
QString LTime::strCurrentDate(QString mask)
{
    return QDate::currentDate().toString(mask);
}
QString LTime::strTime(const QTime &t, QString mask)
{
    return t.toString(mask);
}
QString LTime::strDate(const QDate &d, QString mask)
{
    return d.toString(mask);
}
QString LTime::strDateTime(const QDateTime &dt, QString mask)
{
    return dt.toString(mask);
}
void LTime::getTimeSpecCPP(timespec &tm, Qt::TimeSpec ts, qint64 def_nsec)
{
#ifdef OS_CENTOS
    clock_gettime (CLOCK_REALTIME, &tm);
#else
    timespec_get(&tm, TIME_UTC);
#endif

    if (ts == Qt::LocalTime) tm.tv_sec += (3600 * utcOffset());
    if (def_nsec >= 0) tm.tv_nsec = def_nsec;
}
int LTime::utcOffset()
{
    QDateTime td_loc(QDateTime::currentDateTime());
    QTimeZone z(td_loc.timeZone());
    return (z.offsetFromUtc(td_loc)/3600);
}



//w32_time
void w32_time::setTime(const QDateTime &dt)
{
    quint64 tmp = quint64(dt.toTime_t())*quint64(1000); //конвертация dt в количество мсек, прошедших с (1970-01-01 00:00:00)
    tmp += LTime::beforeUnixMSecs();
    tmp *= 10000ULL; // конвертация милисекунд в 100-наносекундные интервалы;

    dwHigh = (tmp >> 32); //извлечение старших 4-х байтов из значения tmp
    dwLow = ((tmp << 32) >> 32); //извлечение младших 4-х байтов из значения tmp
}
void w32_time::setCurrentTime()
{
    QDateTime dt = QDateTime::currentDateTime();
    setTime(dt);
}
void w32_time::setCurrentTimeUtc()
{
    QDateTime dt = QDateTime::currentDateTimeUtc();
    setTime(dt);
}
QString w32_time::toStr() const
{
    return QString("W32_TIME: dwLow=%1  dwHigh=%2").arg(dwLow).arg(dwHigh);
}
QDateTime w32_time::toQDateTime(Qt::TimeSpec ts)
{
    quint64 tmp = (quint64(dwHigh)*0x100000000ULL + quint64(dwLow))/10000ULL; // конвертация структуры в одно значение quint64, (мсек)
    if (tmp < LTime::beforeUnixMSecs())
    {
        qWarning()<<QString("w32_time::toQDateTime WARNING tmp(%1) < beforeUnixMSecs").arg(tmp); //предупреждение: это время меньше (1970-01-01 00:00:00)
        return QDateTime();
    }
    else tmp -= LTime::beforeUnixMSecs(); //количество мсек начиная с начала эпохи unix

    QDateTime result;
    result.setTimeSpec(ts);
    result.setTime_t(uint(tmp/1000ULL)); //Устанавливает дату и время, параметр - количество секунд, прошедших с (1970-01-01 00:00:00)
    return result.addMSecs(tmp%1000ULL); //добавляет остаток мсек не кратных одной секунде
}


//w32_system_time
void w32_system_time::setTime(const QDateTime &dt) //конвертирует QDateTime в w32_time
{
    wYear = dt.date().year();
    wMonth = dt.date().month();
    wDayOfWeek = dt.date().dayOfWeek();
    wDay = dt.date().day();
    wHour = dt.time().hour();
    wMinute = dt.time().minute();
    wSecond = dt.time().second();
    wMilliseconds = dt.time().msec();
}
void w32_system_time::setCurrentTime()
{
    QDateTime dt = QDateTime::currentDateTime();
    setTime(dt);
}
void w32_system_time::toStream(QDataStream &stream)
{
    stream << wYear << wMonth << wDayOfWeek << wDay << wHour << wMinute << wSecond << wMilliseconds;
}
void w32_system_time::reset()
{
    wYear=2022; wMonth=9; wDayOfWeek=4; wDay=1; //date
    wHour=15; wMinute=35; wSecond=wMilliseconds=0; //time
}
quint32 w32_system_time::size() const
{
    return (sizeof(wYear) + sizeof(wMonth) + sizeof(wDayOfWeek) + sizeof(wDay) +
            sizeof(wHour) + sizeof(wMinute) + sizeof(wSecond) + sizeof(wMilliseconds));
}
QDateTime w32_system_time::toQDateTime(Qt::TimeSpec ts)
{
    QDate qt_date(wYear, wMonth, wDay);
    QTime qt_time(wHour, wMinute, wSecond, wMilliseconds);
    return QDateTime(qt_date, qt_time, ts);
}




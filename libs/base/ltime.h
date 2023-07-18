#ifndef LTIME_H
#define LTIME_H

#include <QDateTime>
#include <QStringList>


class QDataStream;

//базовая структура для хранения времени в 64 битном виде
struct w32_time_base
{
    w32_time_base() {reset();}

    quint32 dwLow;
    quint32 dwHigh;

    inline void reset() {dwLow=100; dwHigh=100;} // сброс значений всех полей структуры в дефолтное состояние
    inline quint32 size() const {return (sizeof(dwLow) + sizeof(dwHigh));} //вернет свой размер в байтах

    virtual void toStream(QDataStream&);//записать структуру в поток
    virtual void fromStream(QDataStream&);//считать структуру из потока

    virtual void setTime(const QDateTime&) = 0;
    virtual QString toStr() const = 0; //вернет значения полей структуры в виде строки

    void setCurrentTime() {setTime(QDateTime::currentDateTime());} //устанавливает текущие дату и время
    void setCurrentTimeUtc() {setTime(QDateTime::currentDateTimeUtc());} //устанавливает текущие дату и время (UTC)

};


//Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
//11644473600 second before unix epoch (so = (unixtime + 11644473600) * 10000
struct w32_time : public w32_time_base
{
    w32_time() :w32_time_base() {}

    void setTime(const QDateTime&); //конвертирует QDateTime в w32_time
    QString toStr() const; //вернет значения полей структуры в виде строки
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC) const; //преобразование структуры в значение QDateTime

};


//содержит 2 значения:
//в старшем количество секунд с начала эпохи unix (1970-01-01 00:00:00)
//в младшем количество микросекунд последней(текущей) секунды
struct w32_time_us : public w32_time_base
{
    w32_time_us() :w32_time_base() {}

    void setTime(const QDateTime&); //конвертирует QDateTime в w32_time
    QString toStr() const; //вернет значения полей структуры в виде строки
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC) const; //преобразование структуры в значение QDateTime

};



//system datetime c++
struct w32_system_time
{
    w32_system_time() {reset();}

    qint16 wYear;
    qint16 wMonth;
    qint16 wDayOfWeek;
    qint16 wDay;
    qint16 wHour;
    qint16 wMinute;
    qint16 wSecond;
    qint16 wMilliseconds;

    void reset();// сброс значений всех полей структуры в дефолтное состояние
    void toStream(QDataStream&); //записать структуру в поток
    void setTime(const QDateTime&); //конвертирует QDateTime в w32_system_time
    void setCurrentTime(); //задает текущие дату и время в w32_system_time
    quint32 size() const; //вернет свой размер в байтах
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC) const; //преобразование структуры в значение QDateTime

};


struct timespec;


//static funcs time
class LTime
{
public:
    //вернет количество мсек прошедших до начала эпохи unix
    static quint64 beforeUnixMSecs() {return 11644473600000ULL;}

    //вернет текущее время в виде строки в заданном формате
    static QString strCurrentTime(QString mask = "hh:mm:ss_zzz");

    //вернет текущую дату в виде строки в заданном формате
    static QString strCurrentDate(QString mask = "dd.MM.yyyy");

    //вернет текущее время и дату в виде строки в заданном формате
    static QString strCurrentDateTime(QString mask = "dd.MM.yyyy hh:mm:ss");

    //вернет указанное время в виде строки в заданном формате
    static QString strTime(const QTime&, QString mask = "hh:mm:ss_zzz");

    //вернет указанную дату в виде строки в заданном формате
    static QString strDate(const QDate&, QString mask = "dd.MM.yyyy");

    //вернет указанное время и дату в виде строки в заданном формате
    static QString strDateTime(const QDateTime&, QString mask = "dd.MM.yyyy hh:mm:ss");

    //функция запишет в переменную tm(сишная структура) текущее значение времени.
    //параметр ts указывает какой часовой пояс нужен (utc либо местное время).
    //параметр def_nsec: если >= 0, то в поле tv_nsec запишется принудительно это значение
    static void getTimeSpecCPP(timespec &tm, Qt::TimeSpec ts = Qt::UTC, qint64 def_nsec = -1);

    //вернет параметр timespec в виде строки в заданном формате
    static QString strTimeSpec(const timespec &tm, Qt::TimeSpec ts = Qt::UTC, QString mask = "dd.MM.yyyy hh:mm:ss.zzz");

    //врернет смещение локального времени относительно UTC (в часах)
    static int utcOffset();

};




#endif //LTIME_H




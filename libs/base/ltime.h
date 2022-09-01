#ifndef LTIME_H
#define LTIME_H

#include <QDateTime>
#include <QStringList>


class QDataStream;

//Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
//11644473600 second before unix epoch (so = (unixtime + 11644473600) * 10000
struct w32_time //for c++
{
    w32_time() {reset();}

    quint32 dwLow;
    quint32 dwHigh;

    void reset() {dwLow=100; dwHigh=100;} // сброс значений всех полей структуры в дефолтное состояние
    quint32 size() const {return (sizeof(dwLow) + sizeof(dwHigh));} //вернет свой размер в байтах
    void toStream(QDataStream &stream) {stream << dwLow << dwHigh;} //записть струкртуры в поток
    void setTime(const QDateTime&); //конвертирует QDateTime в w32_time
    QString toStr() const; //вернет значения полей структуры в виде строки
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC); //преобразование структуры в значение QDateTime
    void setCurrentTime(); //устанавливает текущие дату и время
    void setCurrentTimeUtc(); //устанавливает текущие дату и время (UTC)

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
    void toStream(QDataStream&); //записть струкртуры в поток
    void setTime(const QDateTime&); //конвертирует QDateTime в w32_system_time
    void setCurrentTime(); //задает текущие дату и время в w32_system_time
    quint32 size() const; //вернет свой размер в байтах
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC); //преобразование структуры в значение QDateTime

};


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

};




#endif //LTIME_H




#ifndef MBTCPSTATISTIC_H
#define MBTCPSTATISTIC_H

#include <QTime>

class QModbusRequest;
class QModbusResponse;

//MBTcpStatistic
//вспомогательная структура для накопление статистики по пакетам и ошибок
struct MBTcpStatistic
{
    MBTcpStatistic() {reset();}

    int received; //количетсво полученных сообщений
    int sended; //количество отправленных запросов
    int received_size; //размер последнего полученного пакета (полного), byte
    int sended_size; //размер последнего отправленного пакета (полного), byte
    QTime t_received; //время последнего полученного пакета
    QTime t_sended; // время последнего отправленного пакета
    int errs;   //количетво сетевых ошибок
    int exceptions; //количетво возникших исключений modbus


    void reset();
    void nextReq(const QModbusRequest&);
    void nextResp(const QModbusResponse&);

    inline QString receivedTime() const {return (t_received.isValid() ? t_received.toString(t_mask()) : "---");}
    inline QString sendedTime() const {return (t_sended.isValid() ? t_sended.toString(t_mask()) : "---");}
    inline void nextErr() {errs++;}
    inline void nextException() {exceptions++;}

    static QString t_mask() {return QString("hh:mm:ss.zzz");}

};


#endif // MBTCPSTATISTIC_H

#ifndef MBPARAMS_H
#define MBPARAMS_H

#include <QSerialPort>
#include <QString>


//ModbusPacketParams
struct ModbusPacketParams
{
    ModbusPacketParams() {reset();}

    quint8 address; //адрес сервера (протокол modbus)


    //только для master: код команды и стартовый номер регистра (начинается с 0)
    quint8 cmd;
    quint16 start_pos;

    //для master: количество регистров в запросе на чтение/запись
    //для slave: инициализация буфера сервера, количество регистров в буфере, т.е. для 16 битных регистров размер буфера будет reg_count*2
    quint16 n_regs;

    //только для master: количество запросов при неуспешных или недождавшихся ответах
    quint8 retries;

    //QString emul_config; //конфиг файл для эмуляции системы
    int device_type; // 0-master, 1-slave

    bool show_update_reg_events; //выводить в протокол информацию об изменении значений регистров (only slave)

    void reset()
    {
        address = 0;
        cmd = 0x03;
        start_pos = 0;
        n_regs = 2;
        retries = 1;
        show_update_reg_events = false;
    }
    void setData(const ModbusPacketParams &other)
    {
        address = other.address;
        cmd = other.cmd;
        start_pos = other.start_pos;
        n_regs = other.n_regs;
        retries = other.retries;
        show_update_reg_events = other.show_update_reg_events;
    }
    QString toStr() const
    {
        QString s("PACK_PARAMS_VALUES: ");
        s = QString("%1  address=%2  cmd=%3  pos=%4  number=%5  retries=%6").arg(s).
                arg(address).arg(cmd).arg(start_pos).arg(n_regs).arg(retries);
        return s;
    }

};



#endif // MBPARAMS_H



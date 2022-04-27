#ifndef COMPARAMS_STRUCT_H
#define COMPARAMS_STRUCT_H

#include <QSerialPort>
#include <QString>

//ComParams
struct ComParams
{
    ComParams() {reset();}

    QString port_name;
    int baud_rate;
    int data_bits;
    int stop_bits;
    int parity;
    int device_type; // 0-master, 1-slave

    QString emul_config;

    void reset()
    {
        port_name = "/dev/ttyUSB3";
        baud_rate = QSerialPort::Baud19200;
        data_bits = QSerialPort::Data8;
        stop_bits = QSerialPort::OneStop;
        parity = QSerialPort::EvenParity;
        device_type = 0;
        emul_config.clear();
    }
};

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

    void reset()
    {
        address = 0;
        cmd = 0x03;
        start_pos = 0;
        n_regs = 2;
        retries = 1;
    }
    void setData(const ModbusPacketParams &other)
    {
        address = other.address;
        cmd = other.cmd;
        start_pos = other.start_pos;
        n_regs = other.n_regs;
        retries = other.retries;
    }
    QString toStr() const
    {
        QString s("PACK_PARAMS_VALUES: ");
        s = QString("%1  address=%2  cmd=%3  pos=%4  number=%5  retries=%6").arg(s).
                arg(address).arg(cmd).arg(start_pos).arg(n_regs).arg(retries);
        return s;
    }


};



#endif // COMPARAMS_STRUCT_H



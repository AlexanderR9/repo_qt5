#ifndef LCOMPARAMS_STRUCT_H
#define LCOMPARAMS_STRUCT_H

#include <QSerialPort>
#include <QByteArray>

//LComParams
struct LComParams
{
    LComParams() {reset();}

    QString port_name;
    int baud_rate;
    int data_bits;
    int stop_bits;
    int flow_control;
    int parity;
    int direction;

    void reset()
    {
        port_name = "/dev/ttyUSB3";
        baud_rate = QSerialPort::Baud115200;
        data_bits = QSerialPort::Data8;
        stop_bits = QSerialPort::OneStop;
        flow_control = QSerialPort::NoFlowControl;
        parity = QSerialPort::NoParity;
        direction = QSerialPort::AllDirections;
    }
};



#endif // COMPARAMS_STRUCT_H

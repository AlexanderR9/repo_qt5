#ifndef LCOMPARAMS_H
#define LCOMPARAMS_H

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

    QString toStr() const
    {
        return QString("LComParams: port_name[%1]  baud_rate=%2 stop_bits=%3 flow_control=%4 parity=%5  direction=%6").
                arg(port_name).arg(baud_rate).arg(stop_bits).arg(flow_control).arg(parity).arg(direction);
    }

};



#endif // LCOMPARAMS_H

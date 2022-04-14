#ifndef MBSERVER_H
#define MBSERVER_H

#include "mbslaveserverbase.h"

#include <QDebug>

class QSerialPort;
class QModbusPdu;
struct ComParams;
class MBAdu;
class MBDeviceEmulator;

/*
struct MBEmulSignal
{
    enum MBEmulSignalType {estBit_1 = 1, estBit_16 = 16};

    MBEmulSignal() {reset();}
    MBEmulSignal(int type, quint16 emul_pos) :signal_type(type), emul_register_pos(emul_pos) {value = id = 0;}

    int signal_type;
    quint16 value;
    quint16 emul_register_pos;  //позиция регистра, которая должна быть в имитируемом устройстве

    //уникальный
    //10-ми значное число
    //1-я цифра 1 или 2, означает тип сигнала estBit_1 или estBit_16 ??????
    //следующие 3 знака означают адрес устройства
    //следующие 4 знака означают позицию регистра, которая должна быть в имитируемом устройстве
    //следующие 2 знака означают номер бита в 16-битном регистре (для типа сигнала estBit_1)
    quint32 id;

    bool isBit() const {return (signal_type == estBit_1);}
    void reset() {signal_type = estBit_16; value = emul_register_pos = id = 0;}
    quint32 baseID() const {return quint16(qRound(double(id)/double(100))*100);}
    void calcID(quint8 addr)
    {
        id = 1000000000;
        //if (signal_type == estBit_16) id *= 2;
        id += (1000000 * addr);
        id += 100*emul_register_pos;
        //qDebug()<<QString("new signal, id = %1, addr=%2,  pos=%3").arg(id).arg(addr).arg(emul_register_pos);
    }
    void updateValue(double v, double factor, int adder = 0)
    {
        if (isBit()) value = (v > 0 ? 1 : 0);
        else value = quint16((v + adder)*factor);
    }
};
struct MBEmulDevice
{
    MBEmulDevice() {reset();}
    MBEmulDevice(quint8 addr) :address(addr) {emul_signals.clear();}

    quint8 address;

    QList<MBEmulSignal> emul_signals;

    void reset() {address = 200; emul_signals.clear();}
    void addSignal(int type, quint16 pos)
    {
        emul_signals.append(MBEmulSignal(type, pos));
        emul_signals.last().calcID(address);
    }
    void recalcRegisterPos(QMap<quint32, quint16> &hash_pos)
    {
        for (int i=0; i<emul_signals.count(); i++)
        {
            quint16 reg_pos = hash_pos.count();
            quint32 id = emul_signals.at(i).id;
            if (hash_pos.contains(id))
            {
                reg_pos = hash_pos.value(id);
                if (emul_signals.at(i).isBit())
                {
                    while (hash_pos.contains(id))
                    {
                        id++;
                    }
                    emul_signals[i].id = id;
                }
            }
            //qDebug()<<QString("       rack_addr=%0   id=%1   pos=%2").arg(address).arg(emul_signals.at(i).id).arg(reg_pos);
            hash_pos.insert(emul_signals.at(i).id, reg_pos);
        }
    }
    void updateSignal(int i, double v, double factor, int adder)
    {
        if (i < 0 || i >= emul_signals.count()) return;
        emul_signals[i].updateValue(v, factor, adder);
    }
};
struct MBEmulComplex
{
    MBEmulComplex() {reset();}

    QMap<quint8, MBEmulDevice> emul_devices;

    int signalsCount() const {return hash_pos.count();}
    void reset() {emul_devices.clear(); hash_pos.clear();}
    void addDevice(quint8 address) {emul_devices.insert(address, MBEmulDevice(address));}
    void addDeviceSignal(quint8 address, int type, quint16 pos)
    {
        if (emul_devices.contains(address))
            emul_devices[address].addSignal(type, pos);
    }
    void recalcRegisterPos() //после инициализации контейнера необходимо определить позиции регистров для соответствующих сигналов
    {
        hash_pos.clear();
        QMap<quint8, MBEmulDevice>::iterator it = emul_devices.begin();
        while (it != emul_devices.constEnd())
        {
            it.value().recalcRegisterPos(hash_pos);
            it++;
        }
    }
    QMap<quint32, quint16> hash_pos; //таблица соответствия всех реальных позиций регистров по идентификаторам сигналов
    quint32 idFromRequest(quint8 addr, quint16 reg_pos) const
    {
        quint32 id = 1000000000;
        id += 1000000*addr;
        id += 100*reg_pos;
        return id;
    }
    void hashToStr()
    {
        qDebug() << QString("############ HASH POS ############");
        QList<quint32> keys = hash_pos.keys();
        for (int i=0; i<keys.count(); i++)
            qDebug() << QString("   id=%1   pos=%2").arg(keys.at(i)).arg(hash_pos.value(keys.at(i)));
    }
    void out()
    {
        foreach (MBEmulDevice d, emul_devices)
        {
            qDebug()<<QString("device addr: %1").arg(d.address);
            for (int i=0; i<d.emul_signals.count(); i++)
                qDebug()<<QString("     signal[%1]: id=%2  type=%3  emul_pos=%4").arg(i).arg(d.emul_signals.at(i).id).arg(d.emul_signals.at(i).signal_type).arg(d.emul_signals.at(i).emul_register_pos);
        }
    }
};

*/

//MBServer
class MBServer : public MBSlaveServerBase
{
    Q_OBJECT
public:
    MBServer(QObject *parent = NULL);
    virtual ~MBServer() {}

    void setPortParams(const ComParams&); //установка параметров COM порта
    QString cmdCounterToStr() const; //info
    QString regPosToStr() const; //info

protected:
    void reset();
    void timerEvent(QTimerEvent*); //test
    QModbusResponse exeptionRequest(const QModbusPdu&) const; //обработать исключение запроса

    QMap<quint8, int>   m_cmdCounter; //счетчик команд
    int     m_maxReadingReg;
    int     m_maxWritingReg;
    int     m_invalidPass;

    void parseCurrentBuffer();
    void tryParseAdu(const MBAdu&);

    //for emulation
    void initRegistersMap();
    void transformPDU(QModbusPdu&, quint8);

    MBDeviceEmulator *emul_complex;
    void initEmulComplex(); //инициализировать таблицу устройств и сигналов для имитации системы
    void rewriteEmulData(); //перезаписать значения регистров всех сигналов системы (вызывается с некоторым интервалом)

protected slots:
    void slotSetRegisterValue(int, quint16, quint16); //записать значение в регистр


};


#endif // MBSERVER_H



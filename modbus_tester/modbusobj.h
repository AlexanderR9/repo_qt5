#ifndef MODBUSOBJ_H
#define MODBUSOBJ_H

#include "lsimpleobj.h"
#include "comparams_struct.h"


#include <QByteArray>
#include <QModbusDevice>
#include <QModbusDataUnit>


class QModbusRtuSerialMaster;
class QModbusRtuSerialSlave;
class MBServer;
struct ComParams;


//ModBusObj
class ModBusObj : public LSimpleObject
{
    Q_OBJECT
public:
    enum ModBusObjMode  {mboMaster = 355, mboSlave, mboUnknown = -1};

    ModBusObj(QObject *parent = NULL);
    virtual ~ModBusObj() {}

    void setPortParams(const ComParams&);
    void setPacketParams(const ModbusPacketParams &pp);
    inline const ModbusPacketParams& packetParams() const {return m_packParams;}

    bool isConnected() const;
    void tryConnect(bool&);
    void tryDisconnect();
    QString strMode() const;
    QString strDeviceState() const;
    int deviceState() const;
    QString strDeviceError() const;
    void disconnectSignals();
    void connectSignals();
    void sendData();
    QString portName() const; //return com tty...


    virtual QString name() const {return QString("modbus_obj");}
    inline void setMasterMode() {m_mode = mboMaster;}
    inline void setSlaveMode() {m_mode = mboSlave;}
    inline bool isMaster() const {return (m_mode == mboMaster);}
    inline bool isSlave() const {return (m_mode == mboSlave);}


protected:
    QModbusRtuSerialMaster      *m_master;
    //QModbusRtuSerialSlave       *m_slave;
    MBServer       *m_slave;

    int m_mode;
    ModbusPacketParams m_packParams;

    void sendRequest();
    void sendReply();
    bool readCmd() const;
    bool writeCmd() const;
    void prepareRequestData(QModbusDataUnit&);
    void initDataUnitSlave(); //инициализация набора регистров для режима slave
    void timerEvent(QTimerEvent*);

protected slots:
    void slotErrorOccurred(QModbusDevice::Error);
    void slotStateChanged(QModbusDevice::State);
    void slotDataWritten(QModbusDataUnit::RegisterType, int, int);


};



#endif // MODBUSOBJ_H

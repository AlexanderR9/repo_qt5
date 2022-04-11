#include "modbusobj.h"
#include "mbserver.h"

#include <QDebug>
#include <QModbusRtuSerialMaster>
#include <QModbusRtuSerialSlave>
#include <QModbusReply>
#include <QModbusDataUnit>



/////////////// ModBusObj ///////////////////
ModBusObj::ModBusObj(QObject *parent)
    :LSimpleObject(parent),
    m_master(NULL),
    m_slave(NULL),
    m_mode(-1)
{
    m_master = new QModbusRtuSerialMaster(this);
    m_slave = new MBServer(this);
    //initDataUnitSlave();
    //setSlaveMode();

    m_master->setTimeout(2000);
    m_master->setNumberOfRetries(1);

    //startTimer(3000);
}
void ModBusObj::timerEvent(QTimerEvent *event)
{
    //случайное изменение значений некоторых регистров в режиме slave (для примера)
    if (isSlave() && isConnected())
    {
        quint16 pos = qrand()%10;
        quint16 reg_value = qrand()%5 + 1;
        switch (reg_value)
        {
            case 1: {reg_value = 0x11; break;}
            case 2: {reg_value = 0xbb; break;}
            case 3: {reg_value = 0xcc; break;}
            case 4: {reg_value = 0xdd; break;}
            case 5: {reg_value = 0xee; break;}
        }
        m_slave->setData(QModbusDataUnit::HoldingRegisters, pos, reg_value);
        qDebug()<<QString("ModBusObj::timerEvent: change slave register %1, value %2").arg(pos).arg(reg_value);
    }
}
/*
void ModBusObj::initDataUnitSlave()
{
    quint16 start_pos_reg = 0; //стартовая позиция регистров
    quint16 reg_count = m_packParams.n_regs; //количество регистров в буфере, т.е. для 16 битных регистров размер буфера будет reg_count*2
    QModbusDataUnit data_unit(QModbusDataUnit::HoldingRegisters, start_pos_reg, reg_count);

    //принудительная установка значений для некоторых регистров (просто для примера)
    data_unit.setValue(0, 0xffff);
    data_unit.setValue(1, 0xffff);
    data_unit.setValue(2, 0xa1);
    data_unit.setValue(3, 0xa2);
    data_unit.setValue(4, 0xa3);
    data_unit.setValue(5, 0xa4);
    data_unit.setValue(6, 0xa5a6);
    ////////////////////////////////////////////////////////////

    QModbusDataUnitMap map;
    map.insert(data_unit.registerType(), data_unit);
    m_slave->setMap(map);
}
*/
bool ModBusObj::isConnected() const
{
    if (isMaster()) return (m_master->state() == QModbusDevice::ConnectedState);
    if (isSlave()) return (m_slave->state() == QModbusDevice::ConnectedState);
    return false;
}
QString ModBusObj::strDeviceError() const
{
    switch (m_mode)
    {
        case mboMaster: return m_master->errorString();
        case mboSlave: return m_slave->errorString();
        default: break;
    }
    return QString("???");
}
QString ModBusObj::portName() const
{
    switch (m_mode)
    {
        case mboMaster: return m_master->connectionParameter(QModbusDevice::SerialPortNameParameter).toString();
        case mboSlave: return m_slave->connectionParameter(QModbusDevice::SerialPortNameParameter).toString();
        default: break;
    }
    return QString("???");
}
QString ModBusObj::strDeviceState() const
{
    switch (deviceState())
    {
        case QModbusDevice::UnconnectedState: return QString("Unconnected");
        case QModbusDevice::ConnectingState: return QString("Connecting");
        case QModbusDevice::ConnectedState: return QString("Connected");
        case QModbusDevice::ClosingState: return QString("Closing");
        default: break;
    }
    return QString("Unknown state");
}
int ModBusObj::deviceState() const
{
    switch (m_mode)
    {
        case mboMaster: return m_master->state();
        case mboSlave: return m_slave->state();
        default: break;
    }
    return -1;
}
QString ModBusObj::strMode() const
{
    switch (m_mode)
    {
        case mboMaster: return QString("MASTER_MODE");
        case mboSlave: return QString("SLAVE_MODE");
        default: break;
    }
    return QString("UNKNOWN_MODE");
}
void ModBusObj::slotErrorOccurred(QModbusDevice::Error code)
{
    QString err = QString("%0: device emit error, code=%1").arg(strMode()).arg(code);
    emit signalError(err);
}
void ModBusObj::slotStateChanged(QModbusDevice::State state)
{
    QString msg = QString("%0: device state changed, state=%1 (%2)").arg(strMode()).arg(state).arg(strDeviceState());
    emit signalMsg(msg);
}
void ModBusObj::slotDataWritten(QModbusDataUnit::RegisterType reg, int addr, int size)
{
    quint16 v = 9999;
    bool ok = m_slave->data(reg, addr, &v);
    QString sv = (ok ? QString::number(v) : "fault");
    QString msg = QString("%0: device data writen, register_type=%1  position=%2  size=%3  value=%4").arg(strMode()).arg(reg).arg(addr).arg(size).arg(sv);
    emit signalMsg(msg);
}
void ModBusObj::disconnectSignals()
{
    disconnect(m_master, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(slotErrorOccurred(QModbusDevice::Error)));
    disconnect(m_master, SIGNAL(stateChanged(QModbusDevice::State)), this, SLOT(slotStateChanged(QModbusDevice::State)));

    disconnect(m_slave, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(slotErrorOccurred(QModbusDevice::Error)));
    disconnect(m_slave, SIGNAL(stateChanged(QModbusDevice::State)), this, SLOT(slotStateChanged(QModbusDevice::State)));
    disconnect(m_slave, SIGNAL(dataWritten(QModbusDataUnit::RegisterType, int, int)), this, SLOT(slotDataWritten(QModbusDataUnit::RegisterType, int, int)));
}
void ModBusObj::connectSignals()
{
    if (isMaster())
    {
        connect(m_master, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(slotErrorOccurred(QModbusDevice::Error)));
        connect(m_master, SIGNAL(stateChanged(QModbusDevice::State)), this, SLOT(slotStateChanged(QModbusDevice::State)));
    }
    if (isSlave())
    {
        connect(m_slave, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
        connect(m_slave, SIGNAL(stateChanged(QModbusDevice::State)), this, SLOT(slotStateChanged(QModbusDevice::State)));
        connect(m_slave, SIGNAL(dataWritten(QModbusDataUnit::RegisterType, int, int)), this, SLOT(slotDataWritten(QModbusDataUnit::RegisterType, int, int)));
    }
}
void ModBusObj::sendData()
{
    QString err;
    if (!isConnected())
    {
        err = QString("device not connected.");
        emit signalError(err);
        return;
    }

    if (isMaster()) sendRequest();
    else sendReply();
}
void ModBusObj::prepareRequestData(QModbusDataUnit &data_unit)
{
    data_unit.setStartAddress(m_packParams.start_pos); //установить стартовую позицию данных
    data_unit.setValueCount(m_packParams.n_regs); //установить количество регистров

    QString err;
    QVector<quint16> values;
    switch (m_packParams.cmd)
    {
        case 0x03: break;
        case 0x06:
        {
            data_unit.setValueCount(1); //установить количество регистров
            values.append(quint16(0xabcd));
            break;
        }
        case 0x10:
        {
            //установить значения регистров для записи
            for (int i=0; i<m_packParams.n_regs; i++)
                values.append(quint16(1000+i));
            break;
        }
        default:
        {
            err = QString("invalid request command: %1").arg(m_packParams.cmd);
            emit signalError(err);
            return;
        }
    }

    if (!values.isEmpty()) data_unit.setValues(values);
}
void ModBusObj::sendRequest()
{
    QString msg = QString("Try send request, cmd=%1 .........").arg(m_packParams.cmd);
    emit signalMsg(msg);

    QString err;
    QModbusReply *reply = NULL;
    QModbusDataUnit data_unit(QModbusDataUnit::HoldingRegisters);
    prepareRequestData(data_unit);

    if (readCmd()) reply = m_master->sendReadRequest(data_unit, m_packParams.address);
    else if (writeCmd()) reply = m_master->sendWriteRequest(data_unit, m_packParams.address);

    if (!reply)
    {
        err = QString("reply is NULL");
        emit signalError(err);
        return;
    }
}
bool ModBusObj::readCmd() const
{
    return (m_packParams.cmd == 0x03);
}
bool ModBusObj::writeCmd() const
{
    return (m_packParams.cmd == 0x06 || m_packParams.cmd == 0x10);
}
void ModBusObj::sendReply()
{
    //в режиме slave устройство не может посылать запросы, может только отвечать на них
    QString msg = QString("this mode slave, can't send request!!!");
    emit signalMsg(msg);
}
void ModBusObj::tryConnect(bool &ok)
{
    QString msg = QString("%0: Try connect, port_name=[%1] .........").arg(strMode()).arg(portName());
    emit signalMsg(msg);

    ok = false;
    QString err;
    if (isConnected())
    {
        err = QString("device allready opened.");
        emit signalError(err);
        return;
    }

    disconnectSignals();
    connectSignals();

    if (isMaster()) ok = m_master->connectDevice();
    else if (isSlave())
    {
        //initDataUnitSlave();
        qDebug("try slave connect ...");
        ok = m_slave->connectDevice();
    }
    else
    {
        err = QString(QString("invalid device mode: %1.").arg(m_mode));
        emit signalError(err);
        return;
    }

    if (!ok)
    {
        err = QString("connect fault: %1.").arg(strDeviceError());
        emit signalError(err);
        return;
    }

    emit signalMsg("Ok!");
}
void ModBusObj::tryDisconnect()
{
    QString msg = QString("Try disconnect [%1] .........").arg(strMode());
    emit signalMsg(msg);

    if (!isConnected())
    {
        emit signalError("device allready disconected");
        return;
    }

    if (isMaster()) m_master->disconnectDevice();
    else if (isSlave()) m_slave->disconnectDevice();

    emit signalMsg("done!");
}
void ModBusObj::setPortParams(const ComParams &params)
{
    if (isConnected()) return;

    m_mode = ((params.device_type == 0) ? mboMaster : mboSlave);

    m_master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, params.port_name);
    m_master->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, params.data_bits);
    m_master->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, params.stop_bits);
    m_master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, params.baud_rate);
    m_master->setConnectionParameter(QModbusDevice::SerialParityParameter, params.parity);

    m_slave->setPortParams(params);
}
void ModBusObj::setPacketParams(const ModbusPacketParams &pp)
{
    m_packParams.setData(pp);
    m_master->setNumberOfRetries(m_packParams.retries);
    m_slave->setServerAddress(m_packParams.address);
    emit signalMsg("Packet parameters was updated!");
    emit signalMsg(m_packParams.toStr());
}


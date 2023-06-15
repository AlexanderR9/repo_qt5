#include "mbtcpserverbase.h"
//#include "mbadu.h"
//#include "comparams.h"

#include <QSerialPort>
#include <QModbusResponse>
#include <QModbusRequest>
#include <QDebug>


////////////// LMBTCPServerBase //////////////////
LMBTCPServerBase::LMBTCPServerBase(QObject *parent)
    :QModbusTcpServer(parent)
{
    this->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "127.0.0.1"); //static tcp modbus host
    this->setConnectionParameter(QModbusDevice::NetworkPortParameter, 50502); //default tcp port


    connect(this, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(slotError(QModbusDevice::Error)));
    connect(this, SIGNAL(stateChanged(QModbusDevice::State)), this, SLOT(slotStateChanged(QModbusDevice::State)));
    connect(this, SIGNAL(dataWritten(QModbusDataUnit::RegisterType, int, int)), this, SLOT(slotDataWritten(QModbusDataUnit::RegisterType, int, int)));

}
void LMBTCPServerBase::slotError(QModbusDevice::Error err)
{
    qDebug()<<QString("LMBTCPServerBase::slotError  err=%1").arg(err);
}
void LMBTCPServerBase::slotStateChanged(QModbusDevice::State state)
{
    qDebug()<<QString("LMBTCPServerBase::slotStateChanged  state=%1").arg(state);
}
void LMBTCPServerBase::slotDataWritten(QModbusDataUnit::RegisterType reg_type, int address, int size)
{
    qDebug()<<QString("LMBTCPServerBase::slotDataWritten  reg_type=%1,  address=%2,  size=%3").arg(reg_type).arg(address).arg(size);
}
void LMBTCPServerBase::setDeviceTcpPort(quint32 port)
{
    this->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
}
void LMBTCPServerBase::openConnection()
{
    qDebug()<<QString("LMBTCPServerBase::openConnection()   address=%1    host=%2").arg(this->serverAddress()).
              arg(connectionParameter(QModbusDevice::NetworkAddressParameter).toString()).arg(connectionParameter(QModbusDevice::NetworkPortParameter).toInt());
    if (!isConnected())
    {
        this->open();
    }
}
void LMBTCPServerBase::closeConnection()
{
    if (!isDisconnected())
    {
        this->close();
    }
}
bool LMBTCPServerBase::isConnected() const
{
    return (state() == QModbusDevice::ConnectedState);
}
bool LMBTCPServerBase::isDisconnected() const
{
    return (state() == QModbusDevice::UnconnectedState);
}
bool LMBTCPServerBase::isClosing() const
{
    return (state() == QModbusDevice::ClosingState);
}
QString LMBTCPServerBase::strDeviceState() const
{
    switch (this->state())
    {
        case QModbusDevice::UnconnectedState: return QString("Closed");
        case QModbusDevice::ConnectingState: return QString("Connecting");
        case QModbusDevice::ConnectedState: return QString("Opened");
        case QModbusDevice::ClosingState: return QString("Closing");
        default: break;
    }
    return QString("Unknown state");
}

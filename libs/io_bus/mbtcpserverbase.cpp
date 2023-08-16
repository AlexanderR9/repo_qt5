#include "mbtcpserverbase.h"
//#include "mbadu.h"
//#include "comparams.h"

#include <QSerialPort>
#include <QModbusResponse>
#include <QModbusRequest>
#include <QModbusDataUnit>
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
    Q_UNUSED(reg_type);
    Q_UNUSED(address);
    Q_UNUSED(size);
    //qDebug()<<QString("LMBTCPServerBase::slotDataWritten  reg_type=%1,  address=%2,  size=%3").arg(reg_type).arg(address).arg(size);
}
void LMBTCPServerBase::setDeviceTcpPort(quint32 port)
{
    this->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
}
QByteArray LMBTCPServerBase::getRegistersData(int reg_type, quint16 start_pos, quint16 reg_count) const
{
    QByteArray ba;
    QModbusDataUnit *mbdu = new QModbusDataUnit(QModbusDataUnit::RegisterType(reg_type));
    mbdu->setStartAddress(start_pos);
    mbdu->setValueCount(reg_count);
    if (this->readData(mbdu))
    {
        if (mbdu) //was readed data Ok!
        {
            QDataStream stream(&ba, QIODevice::WriteOnly);
            stream.setByteOrder(QDataStream::BigEndian);
            for (int i=0; i<mbdu->valueCount(); i++)
                stream << mbdu->value(i);
        }
    }
    else qWarning()<<QString("LMBTCPServerBase: WARNING - error reading registers data, start=%1, n_regs=%2").arg(start_pos).arg(reg_count);

    if (mbdu) {delete mbdu; mbdu = NULL;}
    return ba;
}
void LMBTCPServerBase::openConnection()
{
    qDebug()<<QString("LMBTCPServerBase::openConnection()   address=%1/%2    host=%3").arg(this->serverAddress()).
              arg(connectionParameter(QModbusDevice::NetworkAddressParameter).toString()).
              arg(connectionParameter(QModbusDevice::NetworkPortParameter).toInt());

    if (!isConnected()) this->open();
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

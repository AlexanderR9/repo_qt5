#include "mbtcpslavebase.h"
//#include "mbadu.h"
//#include "comparams.h"

#include <QSerialPort>
#include <QModbusResponse>
#include <QModbusRequest>
#include <QModbusDataUnit>
#include <QDebug>


////////////// LMBTcpSlaveBase //////////////////
LMBTcpSlaveBase::LMBTcpSlaveBase(QObject *parent)
    :QModbusTcpServer(parent)
{
    //this->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "127.0.0.1"); //static tcp modbus host
    this->setConnectionParameter(QModbusDevice::NetworkPortParameter, 50502); //default tcp port

    connect(this, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(slotError(QModbusDevice::Error)));
    connect(this, SIGNAL(stateChanged(QModbusDevice::State)), this, SLOT(slotStateChanged(QModbusDevice::State)));
    connect(this, SIGNAL(dataWritten(QModbusDataUnit::RegisterType, int, int)), this, SLOT(slotDataWritten(QModbusDataUnit::RegisterType, int, int)));
}
void LMBTcpSlaveBase::slotError(QModbusDevice::Error err)
{
    qDebug()<<QString("LMBTCPServerBase::slotError  err=%1").arg(err);
}
void LMBTcpSlaveBase::slotStateChanged(QModbusDevice::State state)
{
    qDebug()<<QString("LMBTCPServerBase::slotStateChanged  state=%1").arg(state);
}
void LMBTcpSlaveBase::slotDataWritten(QModbusDataUnit::RegisterType reg_type, int address, int size)
{
    Q_UNUSED(reg_type);
    Q_UNUSED(address);
    Q_UNUSED(size);
    //qDebug()<<QString("LMBTCPServerBase::slotDataWritten  reg_type=%1,  address=%2,  size=%3").arg(reg_type).arg(address).arg(size);
}
void LMBTcpSlaveBase::setDeviceTcpPort(quint16 port)
{
    this->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
}
QString LMBTcpSlaveBase::ipAddress() const
{
    return connectionParameter(QModbusDevice::NetworkAddressParameter).toString().trimmed();
}
quint16 LMBTcpSlaveBase::port() const
{
    return connectionParameter(QModbusDevice::NetworkPortParameter).toInt();
}
QByteArray LMBTcpSlaveBase::getRegistersData(int reg_type, quint16 start_pos, quint16 reg_count) const
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
void LMBTcpSlaveBase::openConnection()
{
    qDebug()<<QString("LMBTcpSlaveBase::openConnection()   dev_address=%1  host=[%2]  port=%3").
              arg(this->serverAddress()).arg(ipAddress()).arg(port());

    if (!isConnected()) this->open();
}
void LMBTcpSlaveBase::closeConnection()
{
    if (!isDisconnected())
    {
        this->close();
    }
}
bool LMBTcpSlaveBase::isConnected() const
{
    return (state() == QModbusDevice::ConnectedState);
}
bool LMBTcpSlaveBase::isDisconnected() const
{
    return (state() == QModbusDevice::UnconnectedState);
}
bool LMBTcpSlaveBase::isClosing() const
{
    return (state() == QModbusDevice::ClosingState);
}
QString LMBTcpSlaveBase::strDeviceState() const
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
QModbusResponse LMBTcpSlaveBase::processRequest(const QModbusPdu &request)
{
    qDebug("LMBTCPServerBase::processRequest");
    return QModbusTcpServer::processRequest(request);
}




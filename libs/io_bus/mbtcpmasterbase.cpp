#include "mbtcpmasterbase.h"

#include <QModbusDevice>
#include <QDebug>


////////////// LMBTcpMasterBase //////////////////
LMBTcpMasterBase::LMBTcpMasterBase(QObject *parent)
    :QModbusTcpClient(parent)
{
    setNumberOfRetries(1);
}
void LMBTcpMasterBase::openConnection()
{
    qDebug()<<QString("LMBTcpMasterBase::openConnection()   ip_address: [%1]  port=%2").arg(ipAddress()).arg(port());

    if (!isConnected()) this->open();
}
void LMBTcpMasterBase::closeConnection()
{
    if (!isDisconnected())
    {
        this->close();
    }
}
bool LMBTcpMasterBase::isConnected() const
{
    return (state() == QModbusDevice::ConnectedState);
}
bool LMBTcpMasterBase::isDisconnected() const
{
    return (state() == QModbusDevice::UnconnectedState);
}
bool LMBTcpMasterBase::isClosing() const
{
    return (state() == QModbusDevice::ClosingState);
}
QString LMBTcpMasterBase::strDeviceState() const
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
QString LMBTcpMasterBase::ipAddress() const
{
    return connectionParameter(QModbusDevice::NetworkAddressParameter).toString().trimmed();
}
quint16 LMBTcpMasterBase::port() const
{
    return connectionParameter(QModbusDevice::NetworkPortParameter).toInt();
}
void LMBTcpMasterBase::setNetworkParams(QString host, quint16 tcp_port)
{
    setConnectionParameter(QModbusDevice::NetworkAddressParameter, host);
    setConnectionParameter(QModbusDevice::NetworkPortParameter, tcp_port);
}
bool LMBTcpMasterBase::processResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    qDebug()<<QString("LMBTcpMasterBase::processResponse");

    return QModbusTcpClient::processResponse(response, data);
}
bool LMBTcpMasterBase::processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    qDebug()<<QString("LMBTcpMasterBase::processPrivateResponse");

    return QModbusTcpClient::processPrivateResponse(response, data);
}
void LMBTcpMasterBase::checkReply(const QModbusReply *reply)
{
    if (!reply) {qWarning("LMBTcpMasterBase::checkReply WARNING reply is NULL"); return;}

    QModbusResponse resp(reply->rawResult());
    QModbusDataUnit data(reply->result());
    processResponse(resp, &data);

}



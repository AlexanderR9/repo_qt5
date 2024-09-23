#include "mbtcpmasterbase.h"

#include <QModbusDevice>
#include <QModbusDataUnit>
#include <QDebug>



////////////// LMBTcpMasterBase //////////////////
LMBTcpMasterBase::LMBTcpMasterBase(QObject *parent)
    :QModbusTcpClient(parent),
      m_devAddress(0)
{
    setObjectName("lmbtcp_mster_base");
    setNumberOfRetries(1);
    resetLastResponse();
}
void LMBTcpMasterBase::resetLastResponse()
{
    m_lastResponse.setFunctionCode(QModbusPdu::Invalid);
    m_lastResponse.setData(QByteArray());
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
quint16 LMBTcpMasterBase::transactionID() const
{
    //to do
    return 0;
}
bool LMBTcpMasterBase::processResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    m_lastResponse = response;

    qDebug()<<QString("LMBTcpMasterBase::processResponse, func_code=%1  resp_data_size=%2").
              arg(m_lastResponse.functionCode()).arg(m_lastResponse.dataSize());

    if (data)
        qDebug()<<QString("reply QModbusDataUnit: reg_type=%1  valueCount=%2").arg(data->registerType()).arg(data->valueCount());

    return QModbusTcpClient::processResponse(response, data);
}
bool LMBTcpMasterBase::processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    qDebug()<<QString("LMBTcpMasterBase::processPrivateResponse");

    return QModbusTcpClient::processPrivateResponse(response, data);
}
void LMBTcpMasterBase::slotCheckReply()
{
    QModbusReply *reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) {qWarning("LMBTcpMasterBase::checkReply WARNING reply is NULL"); return;}

    if (reply->error() != QModbusDevice::NoError)
    {
        qWarning()<<QString("LMBTcpMasterBase::slotCheckReply() WARNING reply->error() = [%1]").arg(reply->errorString());
        setError(reply->errorString(), reply->error());
        m_lastResponse = reply->rawResult();
        //emit errorOccurred(reply->error());
        emit signalRequestFinished(false);
    }
    else
    {
        QModbusResponse resp(reply->rawResult());
        QModbusDataUnit data(reply->result());
        bool ok = processResponse(resp, &data);
        emit signalRequestFinished(ok);
    }

    reply->deleteLater();
}
void LMBTcpMasterBase::sendRequest(const QModbusRequest &req)
{
    resetLastResponse();
    setError(QString(), QModbusDevice::NoError);
    QModbusReply *reply = sendRawRequest(req, m_devAddress);
    if (!reply)
    {
        qWarning("LMBTcpMasterBase::sendRequest  WARNING: reply is NULL");
        setError("Reply is NULL", QModbusDevice::ProtocolError);
        emit errorOccurred(ProtocolError);
        return;
    }

    connect(reply, SIGNAL(finished()), this, SLOT(slotCheckReply()));
}



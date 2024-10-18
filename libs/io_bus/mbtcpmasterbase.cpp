#include "mbtcpmasterbase.h"

#include <QModbusDevice>
#include <QModbusDataUnit>
#include <QDebug>
#include <QTimer>

#define TRACKING_CONNECTION_TIMER       500


////////////// LMBTcpMasterBase //////////////////
LMBTcpMasterBase::LMBTcpMasterBase(QObject *parent)
    :QModbusTcpClient(parent),
      m_devAddress(0),
      is_acive(false),
      m_reconnectTimeout(-1),
      m_trackTimer(NULL),
      m_timerCounter(0)
{
    setObjectName("lmbtcp_mster_base");
    setNumberOfRetries(1);
    resetLastResponse();

    m_trackTimer = new QTimer(this);
    connect(m_trackTimer, SIGNAL(timeout()), this, SLOT(slotConnectionTracking()));
    m_trackTimer->setInterval(TRACKING_CONNECTION_TIMER);
    m_trackTimer->stop();

}
void LMBTcpMasterBase::slotConnectionTracking()
{
    m_timerCounter++;
    //qDebug()<<QString("LMBTcpMasterBase::slotConnectionTracking()  state: %1").arg(strDeviceState());
    //qDebug()<<QString("remainingTime %1").arg(m_timerCounter);
    if (m_timerCounter < 2) return;
    if (isConnected()) {m_timerCounter = 0; return;}

    checkReconnect();
}
void LMBTcpMasterBase::checkReconnect()
{
    if (m_reconnectTimeout < 1) return;

    if (m_timerCounter*TRACKING_CONNECTION_TIMER == (m_reconnectTimeout*1000))
    {
        //qDebug("setState(QModbusDevice::ConnectingState);");
        this->setState(QModbusDevice::ConnectingState);
    }
    else if (m_timerCounter*TRACKING_CONNECTION_TIMER > (m_reconnectTimeout*1000))
    {
        qDebug("reconnect_timeot, try reconnect!!!!");
        m_timerCounter = 0;
        this->open();
    }
}
void LMBTcpMasterBase::resetLastResponse()
{
    m_lastResponse.setFunctionCode(QModbusPdu::Invalid);
    m_lastResponse.setData(QByteArray());
}
void LMBTcpMasterBase::openConnection()
{
    qDebug()<<QString("LMBTcpMasterBase::openConnection()   ip_address: [%1]  port=%2").arg(ipAddress()).arg(port());

    is_acive = true;
    m_timerCounter = 0;
    m_trackTimer->start();

    if (!isConnected()) this->open();
}
void LMBTcpMasterBase::closeConnection()
{
    m_trackTimer->stop();
    if (!isDisconnected())
    {
        this->close();
    }
    is_acive = false;
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
    //TCP connection state
    switch (this->state())
    {
        case QModbusDevice::UnconnectedState: return QString("Unconnected");
        case QModbusDevice::ConnectingState: return QString("Connecting");
        case QModbusDevice::ConnectedState: return QString("Connected");
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
    if (!isActive())
    {
        emit errorOccurred(QModbusDevice::UnknownError);
        return;
    }

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



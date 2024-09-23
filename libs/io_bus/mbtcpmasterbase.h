#ifndef LMBTCPMASTERBASE_H
#define LMBTCPMASTERBASE_H


#include <QModbusTcpClient>
#include <QModbusResponse>

//LMBTcpMasterBase
//класс являющийся надстройкой над ModbusTcpServer,
//работает в режиме MASTER при взаимодействиии ModbusTCP, т.е. является клиентом
class LMBTcpMasterBase : public QModbusTcpClient
{
    Q_OBJECT
    //Q_DECLARE_PRIVATE(QModbusTcpClient)

public:
    LMBTcpMasterBase(QObject *parent = NULL);
    virtual ~LMBTcpMasterBase() {}

    virtual void openConnection();
    virtual void closeConnection();

    virtual QString strDeviceState() const;

    virtual bool isConnected() const;           //device state - ConnectedState
    virtual bool isDisconnected() const;        //device state - UnconnectedState
    virtual bool isClosing() const;             //device state - ClosingState

    //network parameters
    virtual QString ipAddress() const;
    virtual quint16 port() const;
    virtual void setNetworkParams(QString host, quint16 tcp_port);
    virtual void sendRequest(const QModbusRequest&);

    virtual quint16 transactionID() const;
    virtual const QModbusResponse& lastResponse() const {return m_lastResponse;}

    inline quint8 devAddress() const {return m_devAddress;}
    inline void setDevAddress(quint8 a) {m_devAddress = a;}


protected:
    QModbusResponse m_lastResponse;
    quint8 m_devAddress; //modbus device address

    virtual bool processResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual bool processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual void resetLastResponse();

protected slots:
    void slotCheckReply(); //request finished, need check reply

signals:
    void signalRequestFinished(bool);

};



#endif // LMBTCPMASTERBASE_H



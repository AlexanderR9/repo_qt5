#ifndef LMBTCPMASTERBASE_H
#define LMBTCPMASTERBASE_H


#include <QModbusTcpClient>


//LMBTcpMasterBase
//класс являющийся надстройкой над ModbusTcpServer,
//работает в режиме MASTER при взаимодействиии ModbusTCP, т.е. является клиентом
class LMBTcpMasterBase : public QModbusTcpClient
{
    Q_OBJECT
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

    virtual void checkReply(const QModbusReply*);


protected:

    bool processResponse(const QModbusResponse &response, QModbusDataUnit *data);
    bool processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data);

};



#endif // LMBTCPMASTERBASE_H



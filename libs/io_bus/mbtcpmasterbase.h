#ifndef LMBTCPMASTERBASE_H
#define LMBTCPMASTERBASE_H


#include <QModbusTcpClient>
#include <QModbusResponse>

class QTimer;

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

    virtual QString strDeviceState() const; //TCP connection state

    //TCP connection funcs
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
    inline bool isActive() const {return is_acive;} //Modbus object state
    inline void setReconnectTimeout(int t) {if (t != 0) m_reconnectTimeout = t;}

protected:
    QModbusResponse m_lastResponse;
    quint8 m_devAddress; //modbus device address
    bool is_acive; //признак того, что объект активировали (он пытается подключиться или уже обмен идет)

    //используется для отслеживания необходимости переподключения по TCP к SLAVE
    //если произошел разрыв связи или подключения вовсе небыло, задается в секундах,
    //-1 значит не отслеживать эти ситуации и не пытаться переподключиться (это родное поведение QModbusTcpClient)
    int m_reconnectTimeout;

    QTimer *m_trackTimer; //запускается при активации объекта, нужен для отслеживания текущего состояния соединения TCP
    quint32 m_timerCounter; //счетчик тиков m_trackTimer, при запуске m_trackTimer сбрасывается в 0

    virtual void checkReconnect(); //проверить состояние на предмет необходимости переподключения

    virtual bool processResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual bool processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual void resetLastResponse();

protected slots:
    void slotCheckReply(); //request finished, need check reply
    void slotConnectionTracking();

signals:
    void signalRequestFinished(bool); //выполняется когда приходит подготовленный ответ(как успешный, так и нет).


};



#endif // LMBTCPMASTERBASE_H



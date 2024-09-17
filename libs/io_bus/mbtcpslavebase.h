#ifndef LMBTCP_SERVERBASE_H
#define LMBTCP_SERVERBASE_H


#include <QModbusTcpServer>


//LMBTCPServerBase
//класс являющийся надстройкой над ModbusTcpServer,
//работает в режиме SLAVE при взаимодействиии ModbusTCP, т.е. отвечает на запросы
class LMBTcpSlaveBase : public QModbusTcpServer
{
    Q_OBJECT
public:
    LMBTcpSlaveBase(QObject *parent = NULL);
    virtual ~LMBTcpSlaveBase() {}

    virtual void openConnection();
    virtual void closeConnection();

    virtual QString strDeviceState() const;

    virtual bool isConnected() const;           //device state - ConnectedState
    virtual bool isDisconnected() const;        //device state - UnconnectedState
    virtual bool isClosing() const;             //device state - ClosingState


    //network parameters
    virtual QString ipAddress() const;
    virtual quint16 port() const;
    virtual void setDeviceTcpPort(quint16);
//    virtual void setNetworkParams(QString host, quint16 tcp_port);


    //получить данные регистров указанного типа, на заданных позициях
    virtual QByteArray getRegistersData(int, quint16 start_pos, quint16 reg_count) const;

protected:
    virtual QModbusResponse processRequest(const QModbusPdu &request) override;

protected slots:
    //virtual void slotReadyRead(); //пришли данные в m_port, обработать, возможно запрос от мастера
    //virtual void slotAboutToClose(); //выполняется при закрытии устройства(m_port) по каким-либо причинам
    virtual void slotError(QModbusDevice::Error); //выполняется при возникновении ошибки в работе device
    virtual void slotStateChanged(QModbusDevice::State);
    virtual void slotDataWritten(QModbusDataUnit::RegisterType, int, int);

signals:
    void signalError(const QString&); //для отправки сообщения в протокол (внешнему объекту)
    void signalMsg(const QString&);    //для отправки сообщения в протокол (внешнему объекту)

};




#endif // LMBTCP_SERVERBASE_H



#ifndef LMBTCP_SERVERBASE_H
#define LMBTCP_SERVERBASE_H


#include <QModbusTcpServer>


//LMBTCPServerBase
class LMBTCPServerBase : public QModbusTcpServer
{
    Q_OBJECT
public:
    LMBTCPServerBase(QObject *parent = NULL);
    virtual ~LMBTCPServerBase() {}

    virtual void openConnection();
    virtual void closeConnection();

    virtual QString strDeviceState() const;

    virtual bool isConnected() const;           //device state - ConnectedState
    virtual bool isDisconnected() const;        //device state - UnconnectedState
    virtual bool isClosing() const;             //device state - ClosingState

    void setDeviceTcpPort(quint32);

    //получить данные регистров указанного типа, на заданных позициях
    virtual QByteArray getRegistersData(int, quint16 start_pos, quint16 reg_count) const;


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



#ifndef MBSERVER_H
#define MBSERVER_H


#include <QModbusServer>
#include <QElapsedTimer>


class QSerialPort;
class QModbusPdu;
struct ComParams;
class MBAdu;

//MBServer
class MBServer : public QModbusServer
{
    Q_OBJECT
public:
    MBServer(QObject *parent = NULL);
    virtual ~MBServer() {}

    void setPortParams(const ComParams&); //установка параметров COM порта

    inline bool processesBroadcast() const {return is_broadcast;}
    inline bool isConnected() const {return (state() == QModbusDevice::ConnectedState);}
    QString cmdCounterToStr() const; //info
    QString regPosToStr() const; //info

protected:
    virtual bool open();
    virtual void close();
    void getResponseBA(QByteArray&, const QModbusPdu&); //преобразовать ответ (QModbusResponse) в QByteArray для возможной отправки в COM
    qint64 curDTPoint() const;
    double curDTPoint_double() const;
    void reset();

    void timerEvent(QTimerEvent*); //test

    QModbusResponse processRequest(const QModbusPdu &request) override; //подготовить ответ (QModbusResponse) на полученный запрос (request) по правилам протокола modbus
    QModbusResponse dateTimeResponse(const QModbusPdu &request) const;

    QSerialPort        *m_port;
    QByteArray          m_requestBuffer;
    bool                is_broadcast;
    QMap<quint8, int>   m_cmdCounter; //счетчик команд

    int     m_maxReadingReg;
    int     m_maxWritingReg;
    int     m_invalidPass;

    void parseCurrentBuffer();
    void tryParseAdu(const MBAdu&);
    void trySendResponse(const QModbusResponse&);

    //for emulation
    void initBuffers();
    void transformPDU(QModbusPdu&, quint8);

protected slots:
    void slotReadyRead(); //пришли данные, обработать, возможно запрос от мастера
    void slotAboutToClose(); //выполняется при закрытии устройства(m_port) по каким-либо причинам
    void slotError(); //выполняется при возникновении ошибки в работе m_port

signals:
    void signalError(const QString&); //для отправки сообщения в протокол

};


#endif // MBSERVER_H



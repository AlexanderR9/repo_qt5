#ifndef MBSERVER_H
#define MBSERVER_H


#include <QModbusServer>
#include <QElapsedTimer>


class QSerialPort;
class QModbusPdu;
struct ComParams;

//MBServer
class MBServer : public QModbusServer
{
    Q_OBJECT
public:
    MBServer(QObject *parent = NULL);
    virtual ~MBServer() {}

    void setPortParams(const ComParams&);
    virtual bool processesBroadcast() const {return is_broadcast;}
    inline bool isConnected() const {return (state() == QModbusDevice::ConnectedState);}
    QString cmdCounterToStr() const;
    QString regPosToStr() const;

protected:
    virtual bool open();
    virtual void close();
    void getResponseBA(QByteArray&, const QModbusPdu&);
    qint64 curDTPoint() const;
    double curDTPoint_double() const;
    //void recalcElapsedOffset();
    void reset();

    QModbusResponse processRequest(const QModbusPdu &request) override;
    QModbusResponse dateTimeResponse(const QModbusPdu &request) const;



    QSerialPort     *m_port;
    QByteArray       m_requestBuffer;
    bool             is_broadcast;
    //QElapsedTimer    m_elapsedTimer;
    //quint64          m_elapsedOffset; //поправочное смещение относительно стартовой точки времени
    QMap<quint8, int> m_cmdCounter; //счетчик команд
    int     m_maxReadingReg;
    int     m_maxWritingReg;
    int     m_invalidPass;


protected slots:
    void slotReadyRead();
    void slotAboutToClose();
    void slotError();

signals:
    void signalError(const QString&);

};


#endif // MBSERVER_H

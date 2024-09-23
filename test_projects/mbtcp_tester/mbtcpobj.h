#ifndef MBTCP_OBJ_H
#define MBTCP_OBJ_H

#include "lsimpleobj.h"

#include <QByteArray>
#include <QTime>


class LMBTcpMasterBase;
class LMBTcpSlaveBase;
class QModbusRequest;
class QModbusResponse;
class QModbusReply;


//MBTcpStat
//накопление статистики по пакетам и ошибок
struct MBTcpStat
{
    MBTcpStat() {reset();}

    int received;
    int sended;
    int received_size;
    int sended_size;
    QTime t_received;
    QTime t_sended;
    int errs;


    void reset() {received = sended = errs = 0; received_size = sended_size = -1;}
    static QString t_mask() {return QString("hh:mm:ss.zzz");}
    QString receivedTime() const {return (t_received.isValid() ? t_received.toString(t_mask()) : "---");}
    QString sendedTime() const {return (t_sended.isValid() ? t_sended.toString(t_mask()) : "---");}

    void nextReq(const QModbusRequest&);
    void nextResp(const QModbusResponse&);
    void nextErr() {errs++;}


};


//MBTcpObj
//объект для установки соединения и обмена запрос-ответ (зависит от режима)
class MBTcpObj : public LSimpleObject
{
    Q_OBJECT
public:
    MBTcpObj(QObject *parent = NULL);
    virtual ~MBTcpObj() {reset();}

    void reinit(int mode); //0-master, 1-slave

    virtual QString name() const {return QString("mbtcp_obj");}

    inline bool isMaster() const {return (m_master != NULL);}
    inline bool isSlave() const {return (m_slave != NULL);}
    inline bool deactivated() const {return (!isMaster() && !isSlave());}
    inline quint8 devAddress() const {return m_devAddress;}
    inline void setDevAddress(quint8 a) {m_devAddress = a;}


    void start();
    void stop();
    QString curMode() const;
    bool isStarted() const;
    void setNetworkParams(QString host, quint16 tcp_port);
    void sendReq(); //for master only


protected:
    MBTcpStat m_stat;
    quint8 m_devAddress; //modbus device address


    //может быть инициализирован только один из двух объектов
    //если оба NULL, то утилита не работает
    LMBTcpMasterBase    *m_master;
    LMBTcpSlaveBase     *m_slave;

    void reset();
    void initSlave();
    void initMaster();
    void updateReqTable(const QModbusRequest&);
    void updateRespTable(const QModbusResponse&);

protected slots:
    void slotTimer(); //for update state
    void slotRequestFinished(bool); //for master only
    void slotModbusError();

signals:
    void signalUpdateState(const QStringList&);
    void signalFillReq(QModbusRequest&, quint8&, QString&);
    void signalUpdateReqRespTable(const QStringList&, const QStringList&);
    void signalUpdateRegTable(const QModbusResponse&);


};


#endif // MBTCP_OBJ_H

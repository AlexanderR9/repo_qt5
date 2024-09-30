#include "mbtcpobj.h"
#include "mbtcpmasterbase.h"
#include "mbtcpslavebase.h"
#include "exchangestatewidget.h"
#include "mbadu.h"
#include "lmath.h"

#include <QDebug>
#include <QTimerEvent>
#include <QTimer>
#include <QModbusResponse>


/////////////// MBTcpObj ///////////////////
MBTcpObj::MBTcpObj(QObject *parent)
    :LSimpleObject(parent),
    m_master(NULL),
    m_slave(NULL),
    m_devAddress(0)
{
    setObjectName("mbtcp_object");
    reset();

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(slotTimer()));
    t->start(1200);

}
void MBTcpObj::sendReq()
{
    if (!isMaster())
    {
        emit signalError(QString("current mode: %1").arg(curMode()));
        return;
    }

    QModbusRequest req;
    QString err;
    emit signalFillReq(req, m_devAddress, err);
    if (!err.isEmpty()) {emit signalError(err); return;}

    emit signalMsg("send request ...");
    m_stat.nextReq(req);
    m_master->setDevAddress(m_devAddress);
    updateReqTable(req);
    m_master->sendRequest(req);

}
void MBTcpObj::slotRequestFinished(bool result)
{
    emit signalMsg(QString("finished, result=%1").arg(result));
    qDebug()<<QString("MBTcpObj::slotRequestFinished  result=%1").arg(result?"OK":"FAULT");
    if (!result)
    {

        qDebug("!result");
        m_stat.nextErr();
    }
    else
    {
        qDebug("result_ok");
        m_stat.nextResp(m_master->lastResponse());
    }

    //qDebug("need updateRespTable");

    updateRespTable(m_master->lastResponse());
}
void MBTcpObj::slotModbusError()
{
    if (m_master->error() == 0) return;
    qDebug()<<QString("MBTcpObj::slotModbusError  err=%1(%2)").arg(m_master->errorString()).arg(m_master->error());
    emit signalError(QString("MASTER_ERR: err=%1(%2)").arg(m_master->errorString()).arg(m_master->error()));

    //m_stat.nextErr();
}
void MBTcpObj::updateReqTable(const QModbusRequest &req)
{
    QStringList req_list;
    QStringList resp_list;
    for (int i=0; i<5; i++) resp_list<<"?";

    if (req.isValid())
    {
        req_list << "---" << "0" << QString::number(QModbusRequest::calculateDataSize(req)) << QString::number(m_devAddress) << LMath::uint8ToBAStr(quint8(req.functionCode()), true);
        QByteArray ba(req.data());
        if (!ba.isEmpty())
        {
            for (int i=0; i<ba.size(); i++)
            {
                //qDebug()<<QString("ba[%1] = %2 / %3").arg(i).arg(ba.at(i)).arg(int(ba.at(i)));
                //req_list << QString::number(quint8(ba.at(i)), 16);
                req_list << LMath::charToBAStr(ba.at(i), true);
            }
        }
    }
    else req_list << "invalid";

    emit signalUpdateReqRespTable(req_list, resp_list);
}
void MBTcpObj::updateRespTable(const QModbusResponse &resp)
{
    qDebug("MBTcpObj::updateRespTable");
    QStringList req_list;
    QStringList resp_list;

    if (resp.isException()) resp_list << "Exception";
    else
    {
        if (resp.isValid()) resp_list << "---";
        else resp_list << m_master->errorString();
    }


    resp_list << "0" << QString::number(QModbusResponse::calculateDataSize(resp));
    resp_list << "?" << LMath::uint8ToBAStr(quint8(resp.functionCode()), true);
    QByteArray ba(resp.data());
    if (!ba.isEmpty())
    {
        for (int i=0; i<ba.size(); i++)
            resp_list << LMath::charToBAStr(ba.at(i), true);
    }
    emit signalUpdateReqRespTable(req_list, resp_list);
    emit signalUpdateRegTable(resp);
}
void MBTcpObj::slotTimer()
{
    qDebug("MBTcpObj::slotTimer()");
    QStringList state_list;
    if (deactivated())
    {
        state_list << "DEACTIVATED";
        for (int i=0; i<(ExchangeStateWidget::stateRow()-1); i++) state_list << "?";
    }
    else
    {
        if (isMaster())
        {
            if (m_master->isConnected()) state_list << "CONNECTED";
            else  state_list << "OPENED";
            state_list << m_master->ipAddress() << QString::number(m_master->port());
        }
        else
        {
            if (m_slave->isConnected()) state_list << "CONNECTED";
            else  state_list << "OPENED";
            state_list << m_slave->ipAddress() << QString::number(m_slave->port());
        }
        state_list << m_stat.receivedTime() << QString::number(m_stat.received_size) << QString::number(m_stat.received);
        state_list << m_stat.sendedTime() << QString::number(m_stat.sended_size) << QString::number(m_stat.sended);
        state_list << QString::number(m_stat.errs);
    }
    emit signalUpdateState(state_list);
}
void MBTcpObj::reset()
{
    if (m_master)
    {
        m_master->closeConnection();
        m_master->deleteLater();
        m_master = NULL;
    }
    if (m_slave)
    {
        m_slave->closeConnection();
        m_slave->deleteLater();
        m_slave = NULL;
    }
}
void MBTcpObj::reinit(int mode)
{
    reset();
    switch (mode)
    {
        case 0: {initMaster(); break;}
        case 1: {initSlave(); break;}
        default:
        {
            emit signalError(QString("MBTcpObj: invalid mode %1").arg(mode));
            break;
        }
    }
}
void MBTcpObj::setNetworkParams(QString host, quint16 tcp_port)
{
    if (isMaster()) m_master->setNetworkParams(host, tcp_port);
    else if (isSlave()) m_slave->setDeviceTcpPort(tcp_port);
}
void MBTcpObj::initMaster()
{
    m_master = new LMBTcpMasterBase(this);
    connect(m_master, SIGNAL(errorOccurred(QModbusDevice::Error)), this, SLOT(slotModbusError()));
    connect(m_master, SIGNAL(signalRequestFinished(bool)), this, SLOT(slotRequestFinished(bool)));
}
void MBTcpObj::initSlave()
{
    m_slave = new LMBTcpSlaveBase(this);
}
void MBTcpObj::start()
{
    if (isMaster())  m_master->openConnection();
    else if (isSlave())  m_slave->openConnection();
    else
    {
        emit signalError(QString("MBTcpObj: can't starting, MB objects is NULL"));
        return;
    }

    emit signalMsg(QString("MBTcpObj started, mode[%1]").arg(curMode()));
}
void MBTcpObj::stop()
{
    reset();
    emit signalMsg(QString("MBTcpObj stoped!!!"));

}
QString MBTcpObj::curMode() const
{
    if (isMaster())  return "MASTER";
    else if (isSlave())  return "SLAVE";
    else return "NULL";
}
bool MBTcpObj::isStarted() const
{
    return (!deactivated());
}


//MBTcpStat
void MBTcpStat::nextReq(const QModbusRequest &req)
{
    t_sended = QTime::currentTime();
    sended++;
    sended_size = LMBTcpAdu::mbapSize() + req.size();
}
void MBTcpStat::nextResp(const QModbusResponse &resp)
{
    t_received = QTime::currentTime();
    received++;
    received_size = LMBTcpAdu::mbapSize() + resp.size();
}





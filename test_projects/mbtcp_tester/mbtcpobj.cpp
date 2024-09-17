#include "mbtcpobj.h"
#include "mbtcpmasterbase.h"
#include "mbtcpslavebase.h"
#include "exchangestatewidget.h"

#include <QDebug>
#include <QTimerEvent>
#include <QTimer>



/////////////// MBTcpObj ///////////////////
MBTcpObj::MBTcpObj(QObject *parent)
    :LSimpleObject(parent),
    m_master(NULL),
    m_slave(NULL)
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
    quint8 dev_addr;
    emit signalFillReq(req, dev_addr, err);
    if (!err.isEmpty()) {emit signalError(err); return;}

    emit signalMsg("send request ...");
    QModbusReply *reply = m_master->sendRawRequest(req, dev_addr);
    if (reply)
    {
        qDebug()<<QString("reply_err: %1").arg(reply->errorString());
        qDebug()<<QString("reply_finished: %1").arg(reply->isFinished() ? "YES" : "NO");
        qDebug()<<QString("reply_type: %1").arg(reply->type());
        m_master->checkReply(reply);
    }
    else emit signalError("reply is NULL");
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







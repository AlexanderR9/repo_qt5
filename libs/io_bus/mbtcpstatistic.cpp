#include "mbtcpstatistic.h"
#include "mbadu.h"

#include <QModbusResponse>
#include <QModbusRequest>


//MBTcpStatistic
void MBTcpStatistic::nextReq(const QModbusRequest &req)
{
    t_sended = QTime::currentTime();
    sended++;
    sended_size = LMBTcpAdu::mbapSize() + req.size();
}
void MBTcpStatistic::nextResp(const QModbusResponse &resp)
{
    t_received = QTime::currentTime();
    received++;
    received_size = LMBTcpAdu::mbapSize() + resp.size();
    if (resp.isException()) exceptions++;
}
void MBTcpStatistic::reset()
{
    received = sended = errs = exceptions = 0;
    received_size = sended_size = -1;
}




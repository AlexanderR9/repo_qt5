#ifndef MBTCP_OBJ_H
#define MBTCP_OBJ_H

#include "lsimpleobj.h"

#include <QByteArray>


class QModbusTcpClient;
class LMBTCPServerBase;
struct ComParams;


//MBTcpObj
class MBTcpObj : public LSimpleObject
{
    Q_OBJECT
public:
    MBTcpObj(QObject *parent = NULL);
    virtual ~MBTcpObj() {}

    /*
    void setPortParams(const ComParams&);
    QString portName() const;
    QString strDirection() const;
    inline const QByteArray& receivedBuffer() const {return m_readedBuffer;}
    virtual QString name() const {return QString("com_obj");}

    void tryOpen(bool&);
    void tryClose();
    void tryWrite(const QByteArray&);
    bool portOpened() const;
    */

protected:
    QModbusTcpClient    *m_master;
    LMBTCPServerBase    *m_slave;

    /*
    QSerialPort     *m_port;
    int              m_direction;
    QByteArray       m_readedBuffer;

    int openModeByDirection() const;
    void timerEvent(QTimerEvent*);
    int readyBytes() const;

protected slots:
    void slotReadyRead();
    void slotError();
    */

};


#endif // MBTCP_OBJ_H

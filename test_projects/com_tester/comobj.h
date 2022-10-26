#ifndef COMOBJ_H
#define COMOBJ_H

#include "lsimpleobj.h"

#include <QByteArray>


class QSerialPort;
struct ComParams;


//ComObj
class ComObj : public LSimpleObject
{
    Q_OBJECT
public:
    ComObj(QObject *parent = NULL);
    virtual ~ComObj() {}

    void setPortParams(const ComParams&);
    QString portName() const;
    QString strDirection() const;
    inline const QByteArray& receivedBuffer() const {return m_readedBuffer;}
    virtual QString name() const {return QString("com_obj");}

    void tryOpen(bool&);
    void tryClose();
    void tryWrite(const QByteArray&);
    bool portOpened() const;

protected:
    QSerialPort     *m_port;
    int              m_direction;
    QByteArray       m_readedBuffer;

    int openModeByDirection() const;
    void timerEvent(QTimerEvent*);
    int readyBytes() const;

protected slots:
    void slotReadyRead();
    void slotError();

};


#endif // COMOBJ_H

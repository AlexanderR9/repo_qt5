#ifndef DIANA_OBJECT_H
#define DIANA_OBJECT_H

#include "lsimpleobj.h"

class MQWorker;
class MQ;
class QByteArray;


//MBConfigLoader
class DianaObject : public LSimpleObject
{
    Q_OBJECT
public:
    DianaObject(const QString&, QObject *parent = NULL);
    virtual ~DianaObject() {}

    QString name() const {return objectName().toUpper();}
    void addQueue(const QString&);
    const MQ* inputQueue() const;
    const MQ* outputQueue() const;
    void updateMQState();
    void sendMsgToQueue(const QByteArray&); //записать пакет в очередь (input)


protected:
    MQWorker    *mq_manager;

signals:
    void signalSendMsgOk(const QString&);
    void signalReceiveMsgOk(const QString&);
    void signalSendMsgErr(const QString&);
    void signalReceiveMsgErr(const QString&);


};



#endif //DIANA_OBJECT_H



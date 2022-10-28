#ifndef DIANA_OBJECT_H
#define DIANA_OBJECT_H

#include "lsimpleobj.h"

class MQWorker;
class MQ;


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


protected:
    MQWorker    *mq_manager;

};



#endif //DIANA_OBJECT_H



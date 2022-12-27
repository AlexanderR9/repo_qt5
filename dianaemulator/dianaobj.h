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
    void updateMQState(bool);
    void sendMsgToQueue(const QByteArray&, bool); //записать пакет в очередь (input)
    void tryReadMsgFromQueue(QByteArray&, bool); //попытаться считать сообщение из очереди (output)
    void destroyAllQueues(); //удалить все реальные MQ очереди
    void recreatePosixQueues();
    void clearQueues();

    static QString inputType() {return QString("input");}
    static QString outputType() {return QString("output");}

protected:
    MQWorker    *mq_manager;

signals:
    void signalSendMsgOk(const QString&);
    void signalSendMsgErr(const QString&);
    void signalGetPacketSize(const QString&, quint32&);

private:
    int queueInputIndexOf() const;
    int queueOutputIndexOf() const;


};



#endif //DIANA_OBJECT_H



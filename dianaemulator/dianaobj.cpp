#include "dianaobj.h"
#include "lstatic.h"
#include "mqworker.h"
#include "mq.h"

#include <QDomNode>
#include <QFile>


/////////////// DianaObject ///////////////////
DianaObject::DianaObject(const QString &diana_name, QObject *parent)
    :LSimpleObject(parent),
      mq_manager(NULL)
{
    setObjectName(diana_name.trimmed());

    mq_manager = new MQWorker(this);
    connect(mq_manager, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(mq_manager, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

}
void DianaObject::addQueue(const QString &mq_name)
{
    mq_manager->createQueueObj(mq_name);
    emit signalMsg(QString("%1: MQ object [%2] was created").arg(name()).arg(mq_manager->lastQueue()->name()));
}
const MQ* DianaObject::inputQueue() const
{
    for (int i=0; i<mq_manager->count(); i++)
        if (mq_manager->queueAt(i)->name().contains("input"))
            return mq_manager->queueAt(i);
    return NULL;
}
const MQ* DianaObject::outputQueue() const
{
    for (int i=0; i<mq_manager->count(); i++)
        if (mq_manager->queueAt(i)->name().contains("output"))
            return mq_manager->queueAt(i);
    return NULL;
}
void DianaObject::updateMQState()
{
    if (inputQueue()->isDeinit())
        mq_manager->openQueue(0, QIODevice::ReadOnly);

    if (outputQueue()->isDeinit())
        mq_manager->openQueue(1, QIODevice::WriteOnly);

    mq_manager->updateState();
}
void DianaObject::sendMsgToQueue(const QByteArray &ba)
{
    const MQ *mq = inputQueue();
    if (mq)
    {
        bool ok;
        mq_manager->sendMsg(0, ba, ok);

        if (ok) emit signalSendMsgOk(name().toLower());
        else emit signalSendMsgErr(name().toLower());
    }
}











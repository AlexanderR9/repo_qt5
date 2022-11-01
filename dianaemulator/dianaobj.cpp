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
    int pos = queueInputIndexOf();
    return ((pos<0) ? NULL : mq_manager->queueAt(pos));
}
const MQ* DianaObject::outputQueue() const
{
    int pos = queueOutputIndexOf();
    return ((pos<0) ? NULL : mq_manager->queueAt(pos));
}
int DianaObject::queueInputIndexOf() const
{
    for (int i=0; i<mq_manager->count(); i++)
        if (mq_manager->queueAt(i)->name().contains(DianaObject::inputType()))
            return i;
    return -1;
}
int DianaObject::queueOutputIndexOf() const
{
    for (int i=0; i<mq_manager->count(); i++)
        if (mq_manager->queueAt(i)->name().contains(DianaObject::outputType()))
            return i;
    return -1;
}
void DianaObject::updateMQState()
{
    int pos = queueInputIndexOf();
    if (pos >= 0)
    {
        if (inputQueue()->isDeinit())
            mq_manager->openQueue(pos, QIODevice::WriteOnly);
    }


    pos = queueOutputIndexOf();
    if (pos >= 0)
    {
        if (outputQueue()->isDeinit())
            mq_manager->openQueue(pos, QIODevice::ReadOnly);
    }

    mq_manager->updateState();
}
void DianaObject::sendMsgToQueue(const QByteArray &ba)
{
    int pos = queueInputIndexOf();
    if (pos < 0) return;

    bool ok;
    mq_manager->sendMsg(pos, ba, ok);
    if (ok) emit signalSendMsgOk(name().toLower());
    else emit signalSendMsgErr(name().toLower());

}
void DianaObject::tryReadMsgFromQueue(QByteArray &ba)
{
    ba.clear();
    int pos = queueOutputIndexOf();
    if (pos >= 0)
    {
        if (outputQueue()->hasMsg())
        {
            mq_manager->readMsg(pos, ba);
        }
    }
}




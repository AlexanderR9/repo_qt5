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
void DianaObject::clearQueues()
{
    if (mq_manager->isEmpty())
    {
        emit signalError(QString("%1: MQ list is empty").arg(name()));
        return;
    }

    bool ok;
    int n = mq_manager->count();
    for (int i=0; i<n; i++)
    {
        emit signalMsg(QString("%1: try clear queue - %2").arg(name()).arg(mq_manager->queueAt(i)->name()));
        mq_manager->clearMsgs(i, ok);
    }
}
void DianaObject::recreatePosixQueues()
{
    if (mq_manager->isEmpty())
    {
        emit signalError(QString("%1: MQ list is empty").arg(name()));
        return;
    }
    int n = mq_manager->count();
    for (int i=0; i<n; i++)
    {
        if (!mq_manager->queueAt(i)->isNotFound())
        {
            emit signalMsg(QString("%1: try remove queue - %2 .....").arg(name()).arg(mq_manager->queueAt(i)->name()));
            mq_manager->removePosixFile(i);
            emit signalMsg("done.");
        }

        quint32 msg_size = 0;
        emit signalGetPacketSize(mq_manager->queueAt(i)->name(), msg_size);
        emit signalMsg(QString("%1: try create queue - %2, msg_size=%3 .....").arg(name()).arg(mq_manager->queueAt(i)->name()).arg(msg_size));
        mq_manager->createPosixFile(i, msg_size);
        emit signalMsg("done.");
    }
}
void DianaObject::destroyAllQueues()
{
    if (mq_manager->isEmpty())
    {
        emit signalError(QString("%1: MQ list is empty").arg(name()));
        return;
    }
    int n = mq_manager->count();
    for (int i=0; i<n; i++)
    {
        emit signalMsg(QString("%1: try remove queue - %2 .....").arg(name()).arg(mq_manager->queueAt(i)->name()));
        mq_manager->removePosixFile(i);
        emit signalMsg("done.");
    }
}
void DianaObject::updateMQState(bool is_client)
{
    int pos = queueInputIndexOf();
    if (pos >= 0)
    {
        if (inputQueue()->isDeinit())
            mq_manager->openQueue(pos, is_client ? QIODevice::WriteOnly : QIODevice::ReadOnly);
    }


    pos = queueOutputIndexOf();
    if (pos >= 0)
    {
        if (outputQueue()->isDeinit())
            mq_manager->openQueue(pos, is_client ? QIODevice::ReadOnly : QIODevice::WriteOnly);
    }

    mq_manager->updateState();
}
void DianaObject::sendMsgToQueue(const QByteArray &ba, bool is_client)
{
    int pos = (is_client ? queueInputIndexOf() : queueOutputIndexOf());
    if (pos < 0) return;

    bool ok;
    mq_manager->sendMsg(pos, ba, ok);
    if (ok) emit signalSendMsgOk(name().toLower());
    else emit signalSendMsgErr(name().toLower());
}
void DianaObject::tryReadMsgFromQueue(QByteArray &ba, bool is_client)
{
    ba.clear();
    int pos = (!is_client ? queueInputIndexOf() : queueOutputIndexOf());
    if (pos < 0) return;

    if (is_client && outputQueue()->hasMsg()) mq_manager->readMsg(pos, ba);
    else if (!is_client && inputQueue()->hasMsg()) mq_manager->readMsg(pos, ba);
}




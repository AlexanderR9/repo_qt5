 #include "mqworker.h"
 #include "mq.h"


 #include <QDebug>
 #include <QIODevice>

//MQWorker
MQWorker::MQWorker(QObject *parent)
	:LSimpleObject(parent)
{

}
void MQWorker::reset()
{
    qDeleteAll(m_queues);
    m_queues.clear();
}
void MQWorker::createQueueObj(const QString &mq_name)
{
	MQ *mq = new MQ(mq_name, this);
	m_queues.append(mq);
	emit signalMsg(QString("created MQ object, name: %1").arg(mq->name()));

    connect(mq, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(mq, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
}
const MQ* MQWorker::lastQueue() const
{
	if (m_queues.isEmpty()) return NULL;
	return m_queues.last();
}
const MQ* MQWorker::firstQueue() const
{
	if (m_queues.isEmpty()) return NULL;
	return m_queues.first();
}
const MQ* MQWorker::queueAt(int i) const
{
	if (i < 0 || i >= count()) return NULL;
	return m_queues.at(i);
}
void MQWorker::destroyQueueObj(const QString &mq_name)
{
	MQ *mq = getMQByName(mq_name);
	if (!mq)
	{
		emit signalError(QString("queue [%1] not found").arg(mq_name));
		return;
	}

	int pos = m_queues.indexOf(mq);
	m_queues.removeAt(pos);
	delete mq;
	emit signalMsg(QString("MQ object destroyed, name: %1").arg(mq_name));
}
void MQWorker::openQueue(int i, int mode)
{
	if (i < 0 || i >= count())
	{
		emit signalError(QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}

    bool ok;
    m_queues.at(i)->tryOpen(mode, ok);
    if (ok)
    {
    	qDebug()<<QString("Queue [%1] opened OK!").arg(m_queues.at(i)->name());
    	emit signalMsg(QString("queue [%1] was opened, handle=%2!").arg(m_queues.at(i)->name()).arg(m_queues.at(i)->handle()));
    }
}
void MQWorker::closeQueue(int i)
{
	if (i < 0 || i >= count())
	{
		emit signalError(QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}

    bool ok;
    m_queues.at(i)->tryClose(ok);
    if (ok)
    {
    	qDebug()<<QString("Queue [%1] closed OK!").arg(m_queues.at(i)->name());
    	emit signalMsg(QString("queue [%1] was closed!").arg(m_queues.at(i)->name()));
    }
}
void MQWorker::newQueue(const QString &mq_name, int mode, bool &ok)
{
	createQueueObj(mq_name);

    m_queues.last()->tryCreate(mode, ok);
    if (ok)
    {
    	qDebug()<<QString("Queue [%1] created OK!").arg(m_queues.last()->name());
    	emit signalMsg(QString("queue [%1] was created!").arg(m_queues.last()->name()));
    }
    else
    {
    	MQ *mq = m_queues.takeLast();
    	if (mq) delete mq;
    	emit signalMsg(QString("last MQ object destroyed, queues size %1").arg(m_queues.count()));
    }
}
void MQWorker::removeQueue(int i)
{
	if (i < 0 || i >= count())
	{
		emit signalError(QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}

    bool ok;
    m_queues.at(i)->tryDestroy(ok);
    if (ok)
    {
    	QString mq_name = m_queues.at(i)->name();
    	qDebug()<<QString("Queue [%1] destroyed OK!").arg(mq_name);
    	destroyQueueObj(mq_name);
    }
}
void MQWorker::sendMsg(int i, const QByteArray &ba, bool &ok)
{
	if (i < 0 || i >= count())
	{
		emit signalError(QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}

	emit signalMsg(QString("try send pack (%1 bytes) to queue: [%2]").arg(ba.size()).arg(m_queues.at(i)->name()));

    m_queues.at(i)->trySendMsg(ba, ok);
    if (ok)
    {
    	qDebug()<<QString("message sended OK!");
    	emit signalMsg("message was sended");
    }
}
void MQWorker::readMsg(int i, QByteArray &ba)
{
	if (i < 0 || i >= count())
	{
		emit signalError(QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}

	m_queues.at(i)->tryReadMsg(ba);
	if (!ba.isEmpty())
	{
    	qDebug()<<QString("message readed OK!");
    	emit signalMsg(QString("readed %1 bytes").arg(ba.count()));
	}
}
void MQWorker::updateState()
{
	foreach (MQ *value, m_queues)
	{
		if (value)
			value->updateAttrs();
	}
}
bool MQWorker::queueContains(const QString &mq_name) const
{
	if (mq_name.isEmpty()) return false;
	return (getMQByName(QString("/%1").arg(mq_name)) != NULL);
}


//private
MQ* MQWorker::getMQByName(const QString &mq_name) const
{
	MQ *mq = NULL;
	foreach (MQ *value, m_queues)
	{
		if (value->name() == mq_name)
		{
			mq = value;
			break;
		}
	}
	return mq;
}


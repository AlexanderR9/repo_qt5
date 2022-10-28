 #include "mq.h"
 #include <fcntl.h>        
 #include <sys/stat.h> 
 #include <mqueue.h>
 #include <sys/types.h>
 #include <unistd.h>
 #include <cerrno>
 #include <cstring>
 #include "lstatic.h"
  
  
 #include <QDebug>  
 #include <QColor>  
 #include <QIODevice>  
 #include <QByteArray>

#define MSG_PRIOR	1

MQ::MQ(const QString &name, QObject *parent)
	:LSimpleObject(parent),
	 mq_name(QString("/%1").arg(name)),
	 m_size(-1),
	 m_handle(-99),
	 m_state(mqsDeinit),
	 m_mode(0),
	 m_attrs(NULL)
{


}
QString MQ::strStatus() const
{
    if (m_handle > 0) return QString("handle = %1").arg(m_handle);
    if (m_handle == -99) return "???";
    return QString("err = %1").arg(m_handle);
}
QString MQ::strState() const
{
    switch (m_state)
    {
		case mqsOpened: 	return "Opened";
		case mqsCreated: 	return "Created";
		case mqsClosed: 	return "Closed";
		case mqsInvalid: 	return "Invalid";
		default: break;
    }
    return "Not init";	
}
QString MQ::strMode() const
{
    switch (m_mode)
    {
		case QIODevice::NotOpen: return "None";
		case QIODevice::ReadOnly: return "Read";
		case QIODevice::WriteOnly: return "Write";
		case QIODevice::ReadWrite: return "Read/Write";
		default: break;
    }
    return "---";	
}
QString MQ::strAttrs() const
{
	if (!m_attrs) return QString("unknown");

	return QString("flag=%1 msg_num=%2/%3 msg_size=%4").arg(m_attrs->mq_flags).
				arg(m_attrs->mq_curmsgs).arg(m_attrs->mq_maxmsg).arg(m_attrs->mq_msgsize);
}
QColor MQ::colorStatus() const
{
    switch (m_state)
    {
		case mqsOpened: return Qt::blue;
		case mqsClosed: return Qt::black;
		case mqsInvalid: return Qt::red;
		default: break;
    }
    return Qt::gray;
}
void MQ::tryClose(bool &ok)
{
    ok = false;
    QString msg;
    if (!isOpened())
    {
    	msg = QString("MQ[%1]: not was opened").arg(name());
    	emit signalError(msg);
    	qWarning()<<QString("MQ::tryClose - WARNING queue [%1] is not opened").arg(name());
    	return;
    }

    mqd_t result = mq_close(m_handle);
    if (result == 0)
    {
    	m_state = mqsClosed;
    	m_mode = 0;
    	m_handle = -99;
    	ok = true;
    }
    else
    {
    	m_state = mqsInvalid;
    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::tryClose WARNING err=%1  str(%2)  result_code=%4").arg(errno).arg(LStatic::fromCodec(err)).arg(result);

    	msg = QString("MQ[%1]: closing error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    }

	updateAttrs();
}
void MQ::tryCreate(int mode, bool &ok)
{
	ok = false;

    struct mq_attr attr = {0, 0, 0, 0, 0};
    attr.mq_flags = 0; 		// value: 0 or O_NONBLOCK
    attr.mq_maxmsg = 8; 	// max number of messages allowed on queue
    attr.mq_curmsgs = 0; 	// messages count currently in queue
    attr.mq_msgsize = 250; 	// max size of one message

    m_mode = mode;
    int flag = (mqModeByMode() | O_CREAT);
    mqd_t result = mq_open(charName(), flag, 0777, &attr);
    m_handle = result;
    if (result <= 0)
    {
    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::tryCreate - WARNING result=%1  str(%2)").arg(result).arg(LStatic::fromCodec(err));
    	m_state = mqsInvalid;

    	QString msg = QString("MQ[%1]: creating error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    }
    else
    {
    	ok = true;
    	m_state = mqsOpened;
    }

	updateAttrs();

}
void MQ::tryDestroy(bool &ok)
{
	ok = false;
    int result = mq_unlink(charName());
    if (result == 0)
    {
    	ok = true;
    	m_state = mqsDeinit;
    	m_mode = 0;
    	m_handle = -99;
    }
    else
    {
    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::tryOpen - WARNING result=%1  str(%2)").arg(result).arg(LStatic::fromCodec(err));
    	m_state = mqsInvalid;

    	QString msg = QString("MQ[%1]: unlink error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    }

	updateAttrs();
}
void MQ::tryOpen(int mode, bool &ok)
{
    ok = false;
    QString msg;
    if (isOpened())
    {
    	msg = QString("MQ[%1]: allready opened").arg(name());
    	emit signalError(msg);

    	qWarning()<<QString("MQ::tryOpen - WARNING queue [%1] allready opened").arg(name());
    	return;
    }

    m_mode = mode;
    mqd_t result = mq_open(charName(), mqModeByMode());
    m_handle = result;
    if (result <= 0)
    {
    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::tryOpen - WARNING result=%1  str(%2)").arg(result).arg(LStatic::fromCodec(err));
    	m_state = mqsInvalid;

    	msg = QString("MQ[%1]: opening error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    }
    else
    {
    	ok = true;
    	m_state = mqsOpened;
    }

	updateAttrs();
}
void MQ::trySendMsg(const QByteArray &ba, bool &ok)
{
    ok = false;
    QString msg;
    if (!isOpened())
    {
    	msg = QString("MQ[%1]: not was opened").arg(name());
    	emit signalError(msg);
    	qWarning()<<QString("MQ::trySendMsg - WARNING queue [%1] is not opened").arg(name());
    	return;
    }
    if (m_mode != QIODevice::WriteOnly && m_mode != QIODevice::ReadWrite)
    {
    	msg = QString("MQ[%1]: open mode(%2) not for writing").arg(name()).arg(m_mode);
    	emit signalError(msg);
    	return;
    }

    updateAttrs();
    if (m_attrs->mq_curmsgs >= m_attrs->mq_maxmsg)
    {
    	msg = QString("MQ[%1]: stack of queue overflow").arg(name());
    	emit signalError(msg);
    	return;
    }


    size_t len = ba.count();
    const char *data = ba.data();
    int result = mq_send(m_handle, data, len, MSG_PRIOR);
    if (result == 0)
    {
    	ok = true;
    }
    else
    {
    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::trySendMsg WARNING err=%1  str(%2)  result_code=%4").arg(errno).arg(LStatic::fromCodec(err)).arg(result);
    	msg = QString("MQ[%1]: sending msg error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    }

    updateAttrs();
}
void MQ::tryReadMsg(QByteArray &ba)
{
	ba.clear();

    QString msg;
    if (!isOpened())
    {
    	msg = QString("MQ[%1]: not was opened").arg(name());
    	emit signalError(msg);
    	qWarning()<<QString("MQ::tryReadMsg - WARNING queue [%1] is not opened").arg(name());
    	return;
    }
    if (m_mode != QIODevice::ReadOnly && m_mode != QIODevice::ReadWrite)
    {
    	msg = QString("MQ[%1]: open mode(%2) not for reading").arg(name()).arg(m_mode);
    	emit signalError(msg);
    	return;
    }

    updateAttrs();
    if (m_attrs->mq_curmsgs == 0)
    {
    	msg = QString("MQ[%1]: stack of queue is empty").arg(name());
    	emit signalError(msg);
    	return;
    }


    uint prior = 0;
    size_t len = m_attrs->mq_msgsize;
    ba.fill(0, len);
    int result = mq_receive(m_handle, ba.data(), len, &prior);
    if (result < 0)
    {
    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::tryReadMsg WARNING err=%1  str(%2)  result_code=%4").arg(errno).arg(LStatic::fromCodec(err)).arg(result);
    	msg = QString("MQ[%1]: receive msg error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    	ba.clear();
    }
    else
    {
        if (ba.size() > result) ba.truncate(result);
    	msg = QString("MQ[%1]: received msg, size=%2, prior=%3").arg(name()).arg(result).arg(prior);
    	emit signalMsg(msg);
    }

    updateAttrs();
}



//private funcs
const char* MQ::charName() const
{
    QByteArray ba = name().toLocal8Bit();
    return ba.data();
}
int MQ::mqModeByMode() const
{
    switch (m_mode)
    {
		case QIODevice::WriteOnly: return O_WRONLY;
		case QIODevice::ReadWrite: return O_RDWR;
		default: break;
    }
    return O_RDONLY;
}
void MQ::updateAttrs()
{
	m_size = -1;
    if (!isOpened())
    {
    	qWarning()<<QString("MQ::mqAttrs - WARNING queue [%1] is not opened").arg(name());
    	if (m_attrs) {delete m_attrs; m_attrs = NULL;}
    	return;
    }

    m_attrs = new mq_attr();
    if (mq_getattr(m_handle , m_attrs) != 0)
    {
    	delete m_attrs;
    	m_attrs = NULL;

    	QString err(std::strerror(errno));
    	qWarning()<<QString("MQ::mqAttrs - WARNING errno=%1  str(%2)").arg(errno).arg(LStatic::fromCodec(err));
    	QString msg = QString("MQ[%1]: get mq_attrs error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    	emit signalError(msg);
    }

    m_size = m_attrs->mq_curmsgs * m_attrs->mq_msgsize;
}


  

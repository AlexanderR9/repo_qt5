#include "mq.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include "lstatic.h"
#include "lfile.h"
#include "mqworker.h"
  
  
#include <QDebug>
#include <QColor>
#include <QDir>
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
        case mqsNotFound:   return "Not found";
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
        case mqsNotFound:
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
void MQ::tryCreate(int mode, quint32 msg_size, bool &ok)
{
	ok = false;
    if (existPosixFile()) //file mqueue POSIX allready exist
    {
        emit signalError(QString(QString("POSIX file [%1] allready exist").arg(name())));
        qWarning()<<QString("MQ::tryCreate - WARNING queue [%1] allready exist").arg(name());
        return;
    }
    if (msg_size < 1)
    {
        emit signalError(QString("MQ[%1]: creating error, msg_size=%2").arg(name()).arg(msg_size));
        return;
    }

    struct mq_attr attr = {0, 0, 0, 0, 0};
    attr.mq_flags = 0; 		// value: 0 or O_NONBLOCK
    attr.mq_maxmsg = 10; 	// max number of messages allowed on queue
    attr.mq_curmsgs = 0; 	// messages count currently in queue
    attr.mq_msgsize = msg_size; 	// max size of one message

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
void MQ::resetState()
{
    m_state = mqsDeinit;
    m_mode = 0;
    m_handle = -99;
    updateAttrs();
}
void MQ::tryDestroy(bool &ok)
{
    if (!existPosixFile())
    {
        ok = true;
        resetState();
        return;
    }

    int result = mq_unlink(charName());
    ok = (result == 0);
    if (ok) {resetState(); return;}

    QString err(std::strerror(errno));
    qWarning()<<QString("MQ::tryOpen - WARNING result=%1  str(%2)").arg(result).arg(LStatic::fromCodec(err));
    m_state = mqsInvalid;

    QString msg = QString("MQ[%1]: unlink error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(LStatic::fromCodec(err));
    emit signalError(msg);
	updateAttrs();
}
void MQ::tryOpen(int mode, bool &ok)
{
    checkQueueFile(false);
    if (isNotFound()) return;

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
void MQ::tryClearAllMsgs(bool &ok)
{
    ok = true;
    QString msg;
    if (!isOpened())
    {
        msg = QString("MQ[%1]: not was opened").arg(name());
        emit signalError(msg);
        ok = false;
        qWarning()<<QString("MQ::tryClearAllMsgs - WARNING queue [%1] is not opened").arg(name());
        return;
    }

    updateAttrs();
    if (m_attrs->mq_curmsgs == 0)
    {
        msg = QString("MQ[%1]: stack of queue is empty, not necessary for clearing").arg(name());
        emit signalMsg(msg);
        return;
    }

    //reading msgs
    QByteArray ba;
    int n_msg = m_attrs->mq_curmsgs;
    for (int i=0; i<n_msg; i++)
    {
        tryReadMsg(ba);
        if (ba.isEmpty())
        {
            ok = false;
            break;
        }
    }
}
void MQ::updateAttrs()
{
    m_size = -1;
    checkQueueFile();
    if (invalid()) return;

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
bool MQ::hasMsg() const
{
    if (invalid()) return false;
    if (!isOpened() || !m_attrs) return false;
    return (m_attrs->mq_curmsgs > 0);
}
void MQ::checkQueueFile(bool check_invalid)
{
    if (check_invalid && invalid()) return;

    if (!existPosixFile())
    {
        bool ok;
        if (isOpened()) tryClose(ok);
        qWarning()<<QString("MQ::checkQueueFile - WARNING queue [%1] not found").arg(name());
        if (m_attrs) {delete m_attrs; m_attrs = NULL;}
        m_state = mqsNotFound;
        m_handle = -1;
    }
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
bool MQ::existPosixFile() const
{
    QString fname = LStatic::strTrimLeft(name(), 1);
    fname = QString("%1%2%3").arg(MQWorker::mqLinuxDir()).arg(QDir::separator()).arg(fname);
    return LFile::fileExists(fname);
}


  

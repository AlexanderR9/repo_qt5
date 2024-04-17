#include "lthread.h"

#include <QTimerEvent>
#include <QDebug>

#define DEFAULT_TIMER_INTERVAL      350


/////////////// LThreadObj ///////////////////
LThreadObj::LThreadObj(QObject *parent)
    :QThread(parent),
    //m_timer(NULL),
    m_timeout(30000),
    m_runningTime(QTime()),
    m_counter(-1),
    m_lastElapsed(-1),
    m_priority(QThread::LowPriority),
    //m_chekingInterval(-1),
    m_chekingTimerID(-1),
    m_resultCode(trUnknown)
{
    setObjectName("lthread_obj");

    //m_timer = new QTimer(this);
    //m_timer->stop();
    //connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    connect(this, SIGNAL(finished()), this, SLOT(slotThreadFinished()));

}
QString LThreadObj::strResult(int code)
{
    switch (code)
    {
        case trOk:                  return QString("FINISHED_OK");
        case trTimeoutBreaked:      return QString("BREAKED_BY_TIMEOUT");
        case trInvalidParams:       return QString("INVALID_INPUT_PARAMETERS");
        case trUnknown:             return QString("UNKNOWN");
        case trCanNotStartTimer:    return QString("CAN_NOT_START_WAITING_TIMER");
        default: break;
    }
    return "???";
}
void LThreadObj::timerEvent(QTimerEvent *e)
{
    //qDebug() << QString("LThreadObj::timerEvent,  Timer ID: %1").arg(e->timerId());

    if (!isRunning())
    {
        qWarning()<<QString("LThreadObj: WARNING thread does't running but timer running");
        stopThread();
    }
    else if (elapsed() > int(m_timeout))
    {
        qWarning()<<QString("LThreadObj: WARNING thread over timeout");
        m_err="breaked";
        m_resultCode = trTimeoutBreaked;
        stopThread();
    }
}
void LThreadObj::slotThreadFinished()
{
    qDebug("LThread::slotThreadFinished()");
    stopThread();
    m_lastElapsed = m_runningTime.elapsed();
    if (!hasErr()) m_resultCode = trOk;
    else emit signalError(m_err);

    //m_timer->stop();
    emit signalThreadFinished(m_resultCode);
}
void LThreadObj::run()
{
    if (hasErr()) return;

    prepare();
    checkInputParams();
    if (hasErr())
    {
        emit signalError("LThreadObj: invalid input parameters");
        m_resultCode = trInvalidParams;
        return;
    }

    //run task
    doWork();
}
void LThreadObj::startWaitingTimer(int cheking_interval)
{
    if (m_chekingTimerID > 0)
    {
        qWarning()<<QString("LThreadObj::startWaitingTimer() WARNING cheking timer already started, id=%1").arg(m_chekingTimerID);
        killTimer(m_chekingTimerID);
    }

    if (cheking_interval < 50) cheking_interval = DEFAULT_TIMER_INTERVAL;
    m_chekingTimerID = this->startTimer(cheking_interval);
    //qDebug()<<QString("try start checking timer, ID: %1").arg(m_chekingTimerID);
    if (m_chekingTimerID <= 0)
    {
        m_err = "LThreadObj: can't starting checking timer";
        m_resultCode = trCanNotStartTimer;
    }
}
void LThreadObj::startThread(int cheking_interval)
{
    if (isRunning())
    {
        qWarning()<<QString("LThreadObj: WARNING - already running, elapsed %1").arg(lastElapsed());
        emit signalMsg("LThreadObj executing now ...");
        return;
    }

    m_err.clear();
    startWaitingTimer(cheking_interval);

    //starting thread
    this->start(QThread::Priority(m_priority));
}
void LThreadObj::prepare()
{
    if (m_counter < 0) m_counter = 1;
    else m_counter++;

    m_runningTime.start();
    m_lastElapsed = 0;
    m_resultCode = trUnknown;
}
int LThreadObj::elapsed() const
{
    if (!isRunning()) return -1;
    return m_runningTime.elapsed();
}
int LThreadObj::lastElapsed() const
{
    return m_lastElapsed;
}
void LThreadObj::stopThread()
{
    //qDebug("LThreadObj::stopThread()");
    if (m_chekingTimerID > 0)
    {
        killTimer(m_chekingTimerID);
        m_chekingTimerID = -1;
    }
    if (isRunning())
    {
        quit();
        wait();
    }
}


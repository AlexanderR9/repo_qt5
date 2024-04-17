#include "lthread.h"

#include <QTimer>
#include <QDebug>

#define DEFAULT_TIMER_INTERVAL      350


/////////////// LThreadObj ///////////////////
LThreadObj::LThreadObj(QObject *parent)
    :QThread(parent),
    m_timer(NULL),
    m_timeout(30000),
    m_runningTime(QTime()),
    m_counter(-1),
    m_lastElapsed(-1)
{
    setObjectName("lthread_obj");

    m_timer = new QTimer(this);
    m_timer->stop();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    connect(this, SIGNAL(finished()), this, SLOT(slotThreadFinished()));

}
void LThreadObj::slotThreadFinished()
{
    qDebug("LThread::slotThreadFinished()");
    m_lastElapsed = m_runningTime.elapsed();

    if (hasErr())
    {
        emit signalThreadFinished(trTimeoutBreaked);
        return;
    }

    m_timer->stop();
    emit signalThreadFinished(trOk);
}
void LThreadObj::slotTimer()
{
    if (!isRunning())
    {
        qWarning()<<QString("LThreadObj: WARNING thread does't running but timer running");
        m_timer->stop();
        return;
    }

    if (elapsed() > int(m_timeout))
    {
        qWarning()<<QString("LThreadObj: WARNING thread over timeout");
        m_err="breaked";
        m_timer->stop();

        stopThread();
    }
}
void LThreadObj::run()
{
    qDebug("START LThreadObj");
    prepare();
    m_timer->start();

    doWork();
}
void LThreadObj::doWork()
{
    qDebug("LThread::doWork");
}
void LThreadObj::startThread(int cheking_interval)
{
    if (isRunning())
    {
        qWarning()<<QString("LThreadObj: WARNING - already running, elapsed %1").arg(lastElapsed());
        return;
    }

    if (cheking_interval < 50) cheking_interval = DEFAULT_TIMER_INTERVAL;
    m_timer->setInterval(cheking_interval);

    //starting thread
    this->start();
}
void LThreadObj::prepare()
{
    if (m_counter < 0) m_counter = 1;
    else m_counter++;

    m_err.clear();
    m_runningTime.start();
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
    if (isRunning())
    {
        quit();
        wait();
    }
}


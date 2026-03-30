#include "zipobj_base.h"
#include "processobj.h"


#include <QTimer>
#include <QDebug>

#define PROC_TIMER_INTERVAL     750



/////////////// LZipObj_base ///////////////////
LZipObj_base::LZipObj_base(QObject *parent)
    :LSimpleObject(parent),
      m_zipType(ztQt),
      m_state(zosSettingsInvalid),
      m_proc(NULL),
      m_timer(NULL),
      m_workingFolder(QString())
{
    setObjectName("zip_obj_base");

    initProcessor();

    m_timer = new QTimer(this);
    m_timer->setInterval(PROC_TIMER_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotZipTimer()));

    m_tickCounter = 0;
    m_timer->start();

}
void LZipObj_base::initProcessor()
{
    m_proc = new LProcessObj(this);
    connect(m_proc, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_proc, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_proc, SIGNAL(signalFinished()), this, SLOT(slotProcFinished()));

}
bool LZipObj_base::processBuzy() const
{
    if (!m_proc) return false;
    return m_proc->isRunning();
}
void LZipObj_base::slotZipTimer()
{
    m_tickCounter++;
    //qDebug()<<QString("LZipObj_base::slotZipTimer()  m_tickCounter=%1").arg(m_tickCounter);


}
bool LZipObj_base::isArchiveFile(const QString &fname)
{
    if (fname.trimmed().length() < 4) return false;
    return (fname.trimmed().right(3) == QString(".gz"));
}


#include "pcleaner.h"
#include "processobj.h"
#include "lstring.h"
#include "ltime.h"
#include "lfile.h"

#include <QTimer>
#include <QDebug>
#include <QTime>
#include <QDir>


#define KILL_INTERVAL   8000
#define PS_ROW_SIZE     11
#define STIME_MASK      QString("hh:mm")
#define LOG_FILE        QString("log.txt")
#define USER_PASSWD     QString("******")

// PCleaner
PCleaner::PCleaner(int& argc, char** argv)
    :QCoreApplication(argc, argv),
     m_timer(NULL),
     m_proc(NULL),
     t_counter(0),
     m_needKill(-1),
     m_ourUser(QString())
{
    qDebug("PCleaner::PCleaner() started");

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTick()));
    m_timer->start(KILL_INTERVAL);

    m_proc = new LProcessObj(this);
    connect(m_proc, SIGNAL(signalFinished()), this, SLOT(slotProcFinished()));

}
void PCleaner::slotProcFinished()
{
    qDebug()<<QString("FINISHED: process state %1, result=%2").arg(m_proc->strProcessState()).arg(m_proc->isOk()?"OK":"FAULT");
    QString buff(m_proc->buffer().trimmed());
    if (m_proc->isOk())
    {
        QStringList data(m_proc->bufferList());
        if (m_proc->fullCommand().left(2) == "ps")
        {
            parseOut(data);
        }
        else if (m_proc->fullCommand().contains("kill"))
        {
            qDebug()<<QString("BUFFER: %1").arg(buff);
            if (buff.contains("process_err") && !buff.contains("[sudo]"))
            {
                addLog(QString("WARNING - %1").arg(buff));
            }
            else
            {
                addLog(QString("process[%1] was killed!!!").arg(m_needKill));
                m_needKill = -1;
            }
        }
    }
    else
    {
        qDebug()<<QString("ERR: %1").arg(m_proc->err());
        qDebug()<<QString("BUFFER: %1").arg(buff);
        addLog(QString("FAULT FINISHED CMD[%1],  ERR: %2").arg(m_proc->fullCommand()).arg(m_proc->err()));
    }
}
void PCleaner::parseOut(const QStringList &data)
{
    foreach (const QString &v, data)
    {
        QString line(LString::removeLongSpaces(v));
        QStringList row(LString::trimSplitList(line, LString::spaceSymbol()));
        if (row.count() < PS_ROW_SIZE) continue;
        if (row.count() > PS_ROW_SIZE)
        {
            int n=0;
            QString p_name(row.at(PS_ROW_SIZE-1));
            for (int i=PS_ROW_SIZE; i<row.count(); i++)
            {
                p_name = QString("%1%2%3").arg(p_name).arg(LString::spaceSymbol()).arg(row.at(i));
                n++;
            }
            for (int i=0; i<n; i++) row.removeLast();
            row.replace(PS_ROW_SIZE-1, p_name);
        }

        checkKill(row);
    }
}
void PCleaner::checkKill(const QStringList &proc_row)
{
    if (hasKillProcess()) return;

    PInfo p;
    p.fromLine(proc_row);
    if (p.isUsrStartPoint()) return;
    if (!p.isSsl()) return;
    if (!p.isRoot()) return;

    if (p.name.length() < 15 && !p.name.contains(LString::spaceSymbol()))
    {
        qDebug() << p.toStr();
        m_needKill = p.pid;
        //if (p.isRoot()) m_proc->setSudo(true);
        addLog(QString("find virus process - %1").arg(p.toStr()));
    }
}
void PCleaner::slotTimerTick()
{
    if (m_proc->isRunning())
    {
        addLog("WARNING: process_obj is buzy");
        return;
    }


    if (m_ourUser.isEmpty())
    {
        m_ourUser = qgetenv("USER");
        if (m_ourUser.isEmpty()) m_ourUser = qgetenv("USERNAME");
        qDebug() << QString("OUR USER: ") << m_ourUser;
        addLog(QString("finded our user - %1").arg(m_ourUser));
        return;
    }


    t_counter++;
    QStringList args;
    if (hasKillProcess())
    {
        if (m_ourUser == "root")
        {
            m_proc->setCommand("kill");
            args << QString::number(m_needKill);
        }
        else
        {
            //VARIANT 1
            m_proc->setCommand("echo");
            args << USER_PASSWD << "|" << "sudo" << "-S" << "kill" << QString::number(m_needKill);
        }

        //VARIANT 2
        //m_proc->setSudo(true);
        //m_proc->setCommand("kill");
        //args << QString::number(m_needKill);
        addLog(QString("try kill process - %1").arg(m_needKill));
    }
    else
    {
        m_proc->setSudo(false);
        m_proc->setCommand("ps");
        args << "aux";
    }

    m_proc->setArgs(args);
    qDebug()<<QString(" -------------- start cmd [%1] ------------------").arg(m_proc->fullCommand());
    m_proc->startCommand();
}
void PCleaner::addLog(QString line)
{
    QString f_name = QString("%1%2%3").arg(LFile::appPath()).arg(QDir::separator()).arg(LOG_FILE);
    line = QString("tick[%1]  %2 ...... %3").arg(t_counter).arg(LTime::strCurrentDateTime()).arg(line);
    line.append('\n');
    QString err = LFile::appendFile(f_name, line);
    if (!err.isEmpty()) qWarning()<<QString("PCleaner::addLog WARNING - %1").arg(err);
}


//PInfo
void PInfo::fromLine(const QStringList &row)
{
    reset();
    if (row.count() != PS_ROW_SIZE)
    {
        qWarning()<<QString("PInfo::fromLine WARNING - row size (%1) != %2").arg(row.count()).arg(PS_ROW_SIZE);
        return;
    }

    name = row.last();
    user = row.first();
    pid = row.at(1).toInt();
    stat = row.at(7);
    load_proc = row.at(2).toFloat();
    load_mem = row.at(3).toFloat();
    start_time = QTime::fromString(row.at(8), STIME_MASK);
}
QString PInfo::toStr() const
{
    QString s("PROCESS: ");
    if (invalid()) return QString("%1: INVALID").arg(s);

    s = QString("%1 %2, PID=%3, TIME=%4(%5), STAT(%6)").arg(s).arg(user).arg(pid).arg(start_time.toString(STIME_MASK)).arg(runnigSecs()).arg(stat);
    s = QString("%1 LOAD(%2/%3) NAME(%4   len=%6)").arg(s).arg(QString::number(load_proc, 'f', 1)).
            arg(QString::number(load_mem, 'f', 1)).arg(name).arg(name.length());
    if (isUsrStartPoint()) s = QString("%1  IS_USR_PROC").arg(s);


    return s;
}
int PInfo::runnigSecs() const
{
    if (invalid()) return -1;
    return start_time.secsTo(QTime::currentTime());
}

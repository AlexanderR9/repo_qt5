#include "processobj.h"

#include <QProcess>
#include <QTimer>
#include <QDebug>

#define PROCESS_TIMER_INTERVAL      300


/////////////// LProcessObj ///////////////////
LProcessObj::LProcessObj(QObject *parent)
    :LSimpleObject(parent),
    m_process(NULL),
    m_command(QString()),
    is_running(false),
    m_needSudo(false),
    m_timer(NULL)
{
    m_args.clear();
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(slotProcessError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int)));
    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slotProcessStateChanged()));

    m_timer = new QTimer(this);
    m_timer->setInterval(PROCESS_TIMER_INTERVAL);
    m_timer->stop();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void LProcessObj::slotProcessError()
{
    switch (m_process->error())
    {
        case QProcess::FailedToStart: {m_err = "The process failed to start."; break;}
        case QProcess::Crashed: {m_err = "The process crashed after starting."; break;}
        case QProcess::Timedout: {m_err = "The last waitFor...() function timed out."; break;}
        case QProcess::WriteError: {m_err = "An error occurred when attempting to write to the process."; break;}
        case QProcess::ReadError: {m_err = "An error occurred when attempting to read from the process."; break;}
        default: {m_err = "The unknown error occurred."; break;}
    }

    if (m_debugLevel > 0)
        qWarning()<<QString("LProcessObj::slotProcessError() WARNING: %1").arg(m_err);

    emit signalError(m_err);
}
void LProcessObj::slotProcessFinished(int exit_code)
{
    if (m_debugLevel > 0)
    {
        qDebug()<<QString("LProcessObj::slotProcessFinished  exit_code=%1/%2  status=%3").arg(exit_code).arg(m_process->exitCode()).
                  arg(m_process->exitStatus() == 0 ? "normal" : "crashed");
    }

    if (m_process->exitStatus() == QProcess::CrashExit)
        m_err.append("  exit_status = QProcess::CrashExit");

    slotTimer();
}
QString LProcessObj::strProcessState() const
{
    switch (m_process->state())
    {
        case QProcess::NotRunning: return "stoped";
        case QProcess::Starting: return "starting";
        case QProcess::Running: return "running";
        default: break;
    }
    return "??";
}
void LProcessObj::slotProcessStateChanged()
{
    if (m_debugLevel > 0)
        qDebug()<<QString("LProcessObj::slotProcessStateChanged  current state: %1").arg(strProcessState());

}
void LProcessObj::breakCommand()
{
    if (!is_running)
    {
        emit signalError("process is not running");
        return;
    }

    m_process->kill();
}
void LProcessObj::startCommand()
{
    if (m_debugLevel > 0)
    {
        qDebug("");
        qDebug("------------- try start new command --------------");
    }

    m_err.clear();
    m_buff.clear();
    if (m_command.trimmed().isEmpty())
    {
        m_err = QString("Command name is empty");
        emit signalError(m_err);
        return;
    }

    if (m_needSudo) m_process->setProgram(QString("sudo %1").arg(m_command));
    else m_process->setProgram(m_command);
    m_process->setArguments(m_args);

    is_running = true;
    m_timer->start();
    m_process->start();

}
void LProcessObj::slotTimer()
{
    if (m_debugLevel > 0)
    {
        qDebug()<<QString("LProcessObj::slotTimer() bytesAvailable()=%1").arg(m_process->bytesAvailable());
    }

    QString s = m_process->readAllStandardOutput();
    s = s.trimmed();
    if (!s.isEmpty())
        m_buff.append(QString("%1 [process_out] \n").arg(s));

    s = m_process->readAllStandardError();
    s = s.trimmed();
    if (!s.isEmpty())
        m_buff.append(QString("%1 [process_err] \n").arg(s));

    if (m_debugLevel > 0)
    {
        qDebug()<<QString("LProcessObj::slotTimer() buffer size: %1").arg(m_buff.size());
    }

    if (m_process->state() == QProcess::NotRunning)
    {
        is_running = false;
        m_timer->stop();
        emit signalFinished();
    }
}
QStringList LProcessObj::bufferList() const
{
    return buffer().split("\n");
}
void LProcessObj::setArgs(const QStringList& args)
{
    m_args.clear();
    if (!args.isEmpty()) m_args.append(args);
}
QString LProcessObj::fullCommand() const
{
    QString s = m_command;
    if (m_needSudo) s = QString("sudo %1").arg(s);
    if (m_args.isEmpty()) return s;

    for (int i=0; i<m_args.count(); i++)
        s = QString("%1 %2").arg(s).arg(m_args.at(i));
    return s;
}


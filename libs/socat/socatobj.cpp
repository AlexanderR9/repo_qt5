#include "socatobj.h"
#include "lfile.h"
//#include "lstatic.h"

#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QDir>

#define UTILITY_NAME        QString("socat")
#define CHECKING_CMD        QString("command")

/////////////// LSocatObj ///////////////////
LSocatObj::LSocatObj(QObject *parent)
    :LProcessObj(parent),
      m_exist(false),
      m_mode(0),
      m_ownTTY(""),
      m_destinationTTY("")
{
    setObjectName("socat_process");
    connect(this, SIGNAL(signalFinished()), this, SLOT(slotParentFinished()));
    setSudo(false);
}
void LSocatObj::setTTY(QString tty1, QString tty2)
{
    m_ownTTY = tty1.trimmed();
    m_destinationTTY = tty2.trimmed();
}
void LSocatObj::checkUtility()
{
    if (isRunning()) {emit signalError(QString("%1 is buzy").arg(objectName())); return;}

    m_mode = smChecking;
    m_exist = false;
    setCommand("sh");
    QStringList args;
    args << "-c" << QString("%1 -v %2").arg(CHECKING_CMD).arg(UTILITY_NAME);
    setArgs(args);

    emit signalMsg(QString("start cmd: [%1]").arg(fullCommand()));
    startCommand();
}
void LSocatObj::checkCurrentProcess()
{
    if (isRunning()) {emit signalError(QString("%1 is buzy").arg(objectName())); return;}
    m_mode = smCurrentProcess;
    m_processID.clear();

    //ps -ax | grep socat
    setCommand("ps");
    QStringList args;
    args << "-ax" << "|" << "grep" << UTILITY_NAME;
    setArgs(args);

    emit signalMsg(QString("start cmd: [%1]").arg(fullCommand()));
    startCommand();
}
void LSocatObj::killProcess()
{
    if (isRunning()) {emit signalError(QString("%1 is buzy").arg(objectName())); return;}
    m_mode = smKillProcess;

    if (m_processID.isEmpty())
    {
        emit signalMsg("socat running process is empty");
        m_mode = smNone;
        return;
    }

    setCommand("kill");
    QStringList args;
    args << QString::number(m_processID.first());
    setArgs(args);

    emit signalMsg(QString("[%1] ..........").arg(fullCommand()));
    startCommand();
}
void LSocatObj::stopChannel()
{
    if (!isRunning()) {emit signalError(QString("%1 is not running now").arg(objectName())); return;}
    if (m_mode != smComPipe && m_mode != smComCom) {emit signalError(QString("current mode (%1) is not open channel").arg(m_mode)); return;}

    emit signalMsg(QString("try stop process %1 ...........").arg(m_process->processId()));
    m_process->terminate();
}
void LSocatObj::createComToCom()
{
    if (isRunning()) {emit signalError(QString("%1 is buzy").arg(objectName())); return;}
    m_mode = smComCom;

    checkTTY();
    if (!m_err.isEmpty())
    {
        emit signalError(err());
        m_mode = 0;
        return;
    }
    emit signalMsg("TTY names OK!");


    QString params1 = QString("pty,raw,echo=0,link=%1").arg(m_ownTTY);
    QString params2 = QString("pty,raw,echo=0,link=%1").arg(m_destinationTTY);
    setCommand(UTILITY_NAME);
    QStringList args;
    args << params1 << params2;
    setArgs(args);

    emit signalMsg(QString("start cmd: [%1]").arg(fullCommand()));
    startCommand();
}
void LSocatObj::createComToPipe()
{
    if (isRunning()) {emit signalError(QString("%1 is buzy").arg(objectName())); return;}
    m_mode = smComPipe;

    checkTTY();
    if (!m_err.isEmpty())
    {
        emit signalError(err());
        m_mode = 0;
        return;
    }
    emit signalMsg("TTY names OK!");

    QString params1 = QString("pty,raw,echo=0,link=%1").arg(m_ownTTY);
    QString params2 = QString("UNIX-CONNECT:%1").arg(m_destinationTTY);
    setCommand(UTILITY_NAME);
    QStringList args;
    args << "-d" << "-d" << params1 << params2;
    setArgs(args);

    emit signalMsg(QString("start cmd: [%1]").arg(fullCommand()));
    startCommand();
}
bool LSocatObj::isCkeckMode() const
{
    return (m_command == CHECKING_CMD || m_args.contains(CHECKING_CMD) || m_command == "sh");
}
void LSocatObj::slotParentFinished()
{
    switch(m_mode)
    {
        case smChecking:        {finishCheking(); break;}
        case smComCom:          {finishComCom(); break;}
        case smComPipe:         {finishComPipe(); break;}
        case smCurrentProcess:  {finishCurrentProcess(); break;}
        case smKillProcess:     {finishKillCommand(); break;}

        default:
        {
            emit signalError( QString("invalid %1 mode(%2)").arg(name()).arg(m_mode) );
            break;
        }
    }
}
void LSocatObj::finishComCom()
{
    emit signalMsg(QString("COM-COM process finished."));
    m_mode = smNone;
}
void LSocatObj::finishComPipe()
{
    emit signalMsg(QString("COM-PIPE process finished."));
    m_mode = smNone;
}
void LSocatObj::finishCurrentProcess()
{
    emit signalMsg(QString("Check socat-process finished."));
    if (isOk()) //команда выполнилась, смотрим результат
    {
        QStringList result(bufferList());
        int n = result.count();
        for (int i=n-1; i>=0; i--)
            if (result.at(i).trimmed().isEmpty()) result.removeAt(i);
        if (result.isEmpty())
        {
            emit signalMsg("running socat-process not found.");
            return;
        }

        n = 0;
        for (int i=0; i<result.count(); i++)
        {
            QString proc_line = result.at(i); //одна строка процесса в выхлопе: ps ax
            proc_line.remove("[process_out]");
            proc_line.remove("[process_err]");
            proc_line = proc_line.trimmed();

            if (proc_line.contains("grep")) continue;
            if (proc_line.contains(QString("bin%1bash").arg(QDir::separator()))) continue;


            int space_pos = proc_line.indexOf(" ");
            if (space_pos <= 0) emit signalError("split list by space is empty");
            else m_processID.append(proc_line.left(space_pos).trimmed().toUInt());

            n++;
            emit signalMsg(QString("    %1.  %2").arg(n).arg(proc_line));
        }

        if (n > 0) emit signalMsg(QString("Find %1 socat process.").arg(n));
    }
    m_mode = smNone;
}
void LSocatObj::finishKillCommand()
{
    if (!isOk()) return;
    emit signalMsg(QString("done."));
    m_processID.removeFirst();
    killProcess();
}
void LSocatObj::finishCheking()
{
    emit signalMsg(QString("cheking process finished."));
    if (isOk()) //команда выполнилась, смотрим результат
    {
        QStringList result(bufferList());
        int n = result.count();
        for (int i=n-1; i>=0; i--)
            if (result.at(i).trimmed().isEmpty()) result.removeAt(i);

        for (int i=0; i<result.count(); i++)
        {
            if (result.at(i).contains(UTILITY_NAME))
            {
                 m_exist = true;
                 emit signalMsg("Utility OK!");
                 break;
             }
        }
    }

    m_mode = smNone;
    if (!m_exist)
        emit signalError(QString("Utility [%1] not found in your OS!").arg(UTILITY_NAME));
}
void LSocatObj::checkTTY()
{
    m_err.clear();
    m_ownTTY = m_ownTTY.trimmed();
    m_destinationTTY = m_destinationTTY.trimmed();

    if (m_ownTTY.isEmpty())
    {
        m_err = QString("own_tty is empty");
        return;
    }
    if (m_destinationTTY.isEmpty())
    {
        m_err = QString("destination_tty is empty");
        return;
    }


    if (m_mode == smComPipe)
    {
        if (!LFile::fileExists(m_destinationTTY))
        {
            m_err = QString("destination_tty [%1] not found").arg(m_destinationTTY);
            return;
        }
        if (LFile::fileExists(m_ownTTY))
        {
            m_err = QString("own_tty [%1] already exists").arg(m_ownTTY);
            return;
        }
        if (LFile::shortFileName(m_ownTTY).trimmed().isEmpty())
        {
            m_err = QString("own_tty [%1] is invalid").arg(m_ownTTY);
            return;
        }
        if (!LFile::dirExists(LFile::rootPath(m_ownTTY)))
        {
            m_err = QString("own_tty dir [%1] not found").arg(LFile::rootPath(m_ownTTY));
            return;
        }
    }
    else if (m_mode == smComCom)
    {
        if (LFile::fileExists(m_ownTTY))
        {
            m_err = QString("own_tty [%1] already exists").arg(m_ownTTY);
            return;
        }
        if (LFile::shortFileName(m_ownTTY).trimmed().isEmpty())
        {
            m_err = QString("own_tty [%1] is invalid").arg(m_ownTTY);
            return;
        }
        if (!LFile::dirExists(LFile::rootPath(m_ownTTY)))
        {
            m_err = QString("own_tty dir [%1] not found").arg(LFile::rootPath(m_ownTTY));
            return;
        }
        //////////////////////
        if (LFile::fileExists(m_destinationTTY))
        {
            m_err = QString("m_destinationTTY [%1] already exists").arg(m_destinationTTY);
            return;
        }
        if (LFile::shortFileName(m_destinationTTY).trimmed().isEmpty())
        {
            m_err = QString("m_destinationTTY [%1] is invalid").arg(m_destinationTTY);
            return;
        }
        if (!LFile::dirExists(LFile::rootPath(m_destinationTTY)))
        {
            m_err = QString("m_destinationTTY dir [%1] not found").arg(LFile::rootPath(m_destinationTTY));
            return;
        }
    }
}



#include "socatobj.h"
#include "lfile.h"

//#include <QProcess>
#include <QTimer>
#include <QDebug>

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


    //ps -ax | grep socat
    setCommand("ps");
    QStringList args;
    args << "-ax" << "|" << "grep" << UTILITY_NAME;
    setArgs(args);

    emit signalMsg(QString("start cmd: [%1]").arg(fullCommand()));
    startCommand();
}
void LSocatObj::createComToCom()
{
    if (isRunning()) {emit signalError(QString("%1 is buzy").arg(objectName())); return;}
    m_mode = smComCom;
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
        case smChecking:    {finishCheking(); break;}
        case smComCom:      {finishComCom(); break;}
        case smComPipe:     {finishComPipe(); break;}
        case smCurrentProcess:     {finishCurrentProcess(); break;}
        default:
        {
            emit signalError( QString("invalid %1 mode(%2)").arg(name()).arg(m_mode) );
            break;
        }
    }
    emit signalMsg(QString("process finished"));
    m_mode = smNone;
}
void LSocatObj::finishComCom()
{
    emit signalMsg(QString("COM-COM process finished."));

}
void LSocatObj::finishComPipe()
{
    emit signalMsg(QString("COM-PIPE process finished."));

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

        for (int i=0; i<result.count(); i++)
        {
            emit signalMsg(QString("  - %1").arg(result.at(i)));
        }
    }

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

    if (!m_exist)
        emit signalError(QString("Utility [%1] not found in your OS!").arg(UTILITY_NAME));
}
void LSocatObj::checkTTY()
{
    m_err.clear();
    m_ownTTY = m_ownTTY.trimmed();
    m_destinationTTY = m_destinationTTY.trimmed();
    if (m_mode == smComPipe)
    {
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
}



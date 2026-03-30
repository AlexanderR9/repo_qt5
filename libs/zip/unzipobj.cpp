#include "unzipobj.h"
#include "processobj.h"
//#include "lstring.h"
#include "ltime.h"
#include "lfile.h"


//#include <QProcess>
//#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QDateTime>



/////////////// LUnzipObj ///////////////////
LUnzipObj::LUnzipObj(QObject *parent)
    :LZipObj_base(parent),
      m_curFile(QString())
{
    setObjectName("unzip_obj");
    reset();
}
void LUnzipObj::reset()
{
    m_curFile.clear();
    f_list.clear();
    m_state = zosIdle;
}
void LUnzipObj::prepareFileList(const QStringList &list)
{
    f_list.clear();
    if (list.isEmpty()) return;

    foreach (const QString &v, list)
    {
        QString fname = v.trimmed();
        if (fname.isEmpty()) continue;
        if (!LZipObj_base::isArchiveFile(fname)) continue;

        if (LFile::fileExists(QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(fname)))
            f_list.append(fname);
    }
}
void LUnzipObj::tryUnzip()
{
    if (f_list.isEmpty())
    {
        emit signalError("archive-files is empty");
        return;
    }
    if (processBuzy())
    {
        qWarning("ZIP process аlready running");
        emit signalError("process is buzy");
        return;
    }

    // m_proc params
    m_proc->setProcessDir(m_workingFolder);
    switch (m_zipType) // set zip command
    {
        case ztGZip: {m_proc->setCommand("gunzip"); break;}
        case ztTar: {m_proc->setCommand("tar"); break;}
        default:
        {
            emit signalError(QString("invalid zip type: %1").arg(m_zipType));
            return;
        }
    }

    m_state = zosProcessUnzipping;
    unzipNextFile();
}
void LUnzipObj::unzipNextFile()
{
    m_curFile.clear();
    if (f_list.isEmpty()) return;
    else m_curFile = f_list.takeFirst();

    QStringList args;
    switch (m_zipType)
    {
        case ztGZip: {args << "-k" << m_curFile; break;}
        case ztTar: {args << "-xzf" << m_curFile; break;}
    }
    m_proc->startCommand();
}
void LUnzipObj::slotProcFinished()
{
    qDebug()<<QString("%1 ..... unzip processor finished").arg(LTime::strCurrentTime());
    int code = ((m_proc->isOk()) ? 0 : -1);
    emit signalProcFinished(code);

    if (code < 0) // последняя команда m_proc не выполнилась
    {
        reset();
        emit signalError("invalid unziping executed");
        return;
    }

    if (m_state == zosProcessUnzipping)
    {
        if (f_list.isEmpty())
        {
            reset();
            emit signalUnzippingFinished();
        }
        else unzipNextFile();
    }
}
void LUnzipObj::slotZipTimer()
{
    LZipObj_base::slotZipTimer();
    if (processBuzy()) return;

}



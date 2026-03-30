#include "zipobj.h"
#include "lfile.h"
#include "ltime.h"
#include "processobj.h"
#include "lstring.h"


#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QMutexLocker>


/////////////// LZipObj ///////////////////
LZipObj::LZipObj(QObject *parent)
    :LZipObj_base(parent),
      m_compressLevel(8),
      m_tmpFile(QString()),
      f_tmp(NULL),
      m_tmpAppendInterval(90),
      m_lastTs(-1)
{
    setObjectName("zip_obj");

    reset();
}
void LZipObj::slotProcFinished()
{    
    qDebug()<<QString("%1 ..... processor finished").arg(LTime::strCurrentTime());
    bool was_zip = (m_state == zosProcessZipping);
    m_state = zosIdle;
    int code = ((m_proc->isOk()) ? 0 : -1);
    emit signalProcFinished(code);
    if (code < 0) reset();

    if (was_zip && (code == 0)) emit signalZippingFinished();

}
void LZipObj::setAppendTmpInterval(int t)
{
    if (t <= 0) m_tmpAppendInterval = -1;
    else if (t > 10) m_tmpAppendInterval = t;
}
bool LZipObj::tmpOpened() const
{
    if (f_tmp) return true;
    return false;
}
void LZipObj::openTmpFile()
{
    m_workingFolder = m_workingFolder.trimmed();
    m_tmpFile = m_tmpFile.trimmed();
    if (m_workingFolder.isEmpty() || !LFile::dirExists(m_workingFolder))
    {
        emit signalError("working folder is invalid");
        return;
    }
    if (m_tmpFile.isEmpty())
    {
        emit signalError("tmp filename is empty");
        return;
    }
    if (f_tmp)
    {
        emit signalError("tmp file already opened");
        return;
    }

    QString fname = QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(m_tmpFile);
    f_tmp = new QFile(fname);
    if (!f_tmp->open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QString err = QString("Cannot open tmp-file %1: %2").arg(m_tmpFile).arg(f_tmp->errorString());
        qWarning() << "WARNING: " << err;
        emit signalError(err);
        closeTmpFile(false);
    }
    else
    {
        qDebug()<<QString("TMP file success opened: %1").arg(m_tmpFile);
        emit signalMsg(QString("TMP file success opened: %1").arg(m_tmpFile));
    }
}
void LZipObj::closeTmpFile(bool add_cur_buff)
{
    if (!f_tmp) return;

    if (add_cur_buff)
    {
        tryAppendBufferToTmpFile();
        m_lastTs = -1;
    }

    if (f_tmp->isOpen()) f_tmp->close();
    delete f_tmp;
    f_tmp = NULL;

    emit signalMsg(QString("TMP file was closed: %1").arg(m_tmpFile));
}
void LZipObj::addBufferData(const QByteArray &ba)
{
    if (ba.isEmpty()) return;

    m_buffer.append(ba);
    if (m_tmpAppendInterval <= 0) // данные сразу добавляются в tmp файл, без накопления
        tryAppendBufferToTmpFile();
}
void LZipObj::checkTmpTimeout()
{
    // check TS
    qint64 ts = QDateTime::currentDateTime().toSecsSinceEpoch();
    if (m_lastTs <= 0) {m_lastTs = ts; return;}
    if ((ts - m_lastTs) < m_tmpAppendInterval) return;

    QMutexLocker locker(&m_mutex);
    tryAppendBufferToTmpFile();
    m_lastTs = ts;
}
void LZipObj::tryAppendBufferToTmpFile()
{
    if (!f_tmp || !f_tmp->isOpen())
    {
        emit signalError("tmp file was not opened");
        f_tmp = NULL;
    }
    else
    {
        int result = f_tmp->write(m_buffer.data(), m_buffer.size());
        if (result < 0) emit signalError("can't append data to tmp file");
        else f_tmp->flush();
    }
    m_buffer.clear();
}
void LZipObj::reset()
{
    m_buffer.clear();
    m_lastTs = -1;
    m_state = zosIdle;
}
void LZipObj::tryZipFile(QString fname)
{
    if (processBuzy())
    {
        qWarning("ZIP process аlready running");
        emit signalError("process is buzy");
        return;
    }


    qDebug()<<QString("LZipObj::tryZipFile [%1]").arg(fname);
    m_workingFolder = m_workingFolder.trimmed();
    fname = fname.trimmed();
    if (m_workingFolder.isEmpty() || !LFile::dirExists(m_workingFolder))
    {
        emit signalError("working folder is invalid");
        return;
    }
    if (fname.isEmpty())
    {
        emit signalError("archiving filename is empty");
        return;
    }
    if (LZipObj_base::isArchiveFile(fname))
    {
        emit signalError(QString("archiving file is already archive: %1 ").arg(fname));
        return;
    }
    if (!LFile::fileExists(QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(fname)))
    {
        emit signalError(QString("archiving file not found: %1 ").arg(fname));
        return;
    }

    startZipProcess(fname);
}
void LZipObj::startZipProcess(const QString &fname)
{
    qDebug()<<QString("%1 ..... processor started").arg(LTime::strCurrentTime());
    QStringList args;
    m_proc->setProcessDir(m_workingFolder);

    switch (m_zipType)
    {
        case ztGZip:
        {
            m_proc->setCommand("gzip");
            //args << "-c" << fname << ">" << zipExtentionByInputFile(fname);
            args << "-k" << fname;
            break;
        }
        case ztTar:
        {
            m_proc->setCommand("tar");
            args << "-czf" << zipExtentionByInputFile(fname) << fname;
            break;
        }
        default:
        {
            emit signalError(QString("invalid zip type: %1").arg(m_zipType));
            break;
        }
    }

    if (!args.isEmpty())
    {
        m_state = zosProcessZipping;
        m_proc->startCommand();
    }
}
void LZipObj::slotZipTimer()
{
    LZipObj_base::slotZipTimer();
    if (processBuzy()) return;
    if (m_tmpAppendInterval <= 0) return;

    checkTmpTimeout();
}



// private
QString LZipObj::zipExtentionByInputFile(const QString &fname) const
{
    if (fname.trimmed().isEmpty()) return QString();

    QString fname_zip(fname.trimmed());
    QString ext = LFile::fileExtension(fname_zip).trimmed();

    QString ext_zip;
    switch (m_zipType)
    {
        case ztGZip: {ext_zip = ".gz"; break;}
        case ztTar: {ext_zip = ".tar.gz"; break;}
        default: return QString();
    }

    if (!ext.isEmpty())
        fname_zip = LString::strTrimRight(fname_zip, ext.length()+1);

    fname_zip.append(ext_zip);
    return fname_zip;
}




#include "zipobj.h"
#include "lfile.h"
#include "ltime.h"
#include "processobj.h"
#include "lstring.h"


//#include <QProcess>
//#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QDateTime>



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

    //initProcessor();
    reset();

}

/*
void LZipObj::initProcessor()
{
    m_proc = new LProcessObj(this);
    connect(m_proc, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_proc, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_proc, SIGNAL(signalFinished()), this, SLOT(slotProcFinished()));

}
*/


void LZipObj::slotProcFinished()
{
    qDebug()<<QString("%1 ..... processor finished").arg(LTime::strCurrentTime());

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
        appendTmpFile();
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
    if (m_tmpAppendInterval <= 0)
    {
        appendTmpFile();
        return;
    }

    // check TS
    qint64 ts = QDateTime::currentDateTime().toSecsSinceEpoch();
    if (m_lastTs <= 0) {m_lastTs = ts; return;}
    if ((ts - m_lastTs) < m_tmpAppendInterval) return;

    appendTmpFile();
    m_lastTs = ts;
}
void LZipObj::appendTmpFile()
{
    if (!f_tmp || !f_tmp->isOpen())
    {
        emit signalError("tmp file was not opened");
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
}
void LZipObj::tryZipFile(QString fname)
{
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

    //fname = QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(fname);
    if (!LFile::fileExists(QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(fname)))
    {
        emit signalError(QString("archiving file not found: %1 ").arg(fname));
        return;
    }

    startZipProcess(fname);

    /*
    compress_result.clear();
    if (m_buffer.isEmpty())
    {
        emit signalError("LZipObj: buffer is empty");
        return;
    }

    qDebug()<<QString("%1  compress %2 bytes started (level %3) ......").arg(LTime::strCurrentTime()).arg(m_buffer.size()).arg(m_compressLevel);
    compress_result = qCompress(m_buffer, m_compressLevel);


    qDebug()<<QString("%1  compress finished, result %2 bytes!").arg(LTime::strCurrentTime()).arg(compress_result.size());
    */


}
void LZipObj::startZipProcess(const QString &fname)
{
    qDebug()<<QString("%1 ..... processor started").arg(LTime::strCurrentTime());
    if (!m_proc) {qWarning("LZipObj: WARNING - m_proc is NULL"); return;}
    if (m_proc->isRunning())
    {
        emit signalError("LZipObj: process object is buzy");
        qWarning("LZipObj: WARNING - m_proc is buzy");
        return;
    }

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
        m_proc->startCommand();;
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




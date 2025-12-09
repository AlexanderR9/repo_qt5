#include "zipobj.h"
#include "lfile.h"
#include "ltime.h"


//#include <QProcess>
//#include <QTimer>
#include <QDebug>
#include <QDir>



/////////////// LZipObj ///////////////////
LZipObj::LZipObj(QObject *parent)
    :LSimpleObject(parent),
      m_compressLevel(8),
      m_workingFolder(QString()),
      m_zipType(ztQt)
{
    setObjectName("zip_obj");

    reset();

}
void LZipObj::addBufferData(const QByteArray &ba)
{
    m_buffer.append(ba);
}
void LZipObj::reset()
{
    m_buffer.clear();
    m_bufferExtract.clear();
}
void LZipObj::tryCompress(QByteArray &compress_result)
{
    compress_result.clear();
    if (m_buffer.isEmpty())
    {
        emit signalError("LZipObj: buffer is empty");
        return;
    }

    qDebug()<<QString("%1  compress %2 bytes started (level %3) ......").arg(LTime::strCurrentTime()).arg(m_buffer.size()).arg(m_compressLevel);
    compress_result = qCompress(m_buffer, m_compressLevel);


    qDebug()<<QString("%1  compress finished, result %2 bytes!").arg(LTime::strCurrentTime()).arg(compress_result.size());
}
void LZipObj::writeBufferToFile(QString fname, bool &ok)
{
    qDebug()<<QString("%1 try write buffer to file ....").arg(LTime::strCurrentTime());
    fname = fname.trimmed();
    ok = false;

    // check state
    QString err;
    if (fname.isEmpty())
    {
        qWarning("LZipObj::writeBufferToFile WARNING fname is empty");
        emit signalError("LZipObj::writeBufferToFile WARNING fname is empty");
        return;
    }
    if (!LFile::dirExists(m_workingFolder))
    {
        err = QString("LZipObj::writeBufferToFile WARNING invalid working folder - [%1]").arg(m_workingFolder);
        qWarning()<<err;
        emit signalError(err);
        return;
    }
    if (m_buffer.isEmpty())
    {
        err = QString("LZipObj::writeBufferToFile WARNING buffer is empty");
        qWarning()<<err;
        emit signalError(err);
        return;
    }

    //write operation
    QString full_name = QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(fname);
    qDebug()<<QString("try write buffer (%1 bytes) to file - [%2]").arg(m_buffer.size()).arg(full_name);

    err = LFile::writeFileBA(full_name, m_buffer);
    if (!err.isEmpty())
    {
        err = QString("LZipObj::writeBufferToFile  %1").arg(err);
        qWarning()<<err;
        emit signalError(err);
        return;
    }

    ok = true;
    qDebug()<<QString("%1 write finished!").arg(LTime::strCurrentTime());
}
void LZipObj::appendBufferToFile(QString fname, bool &ok)
{
    qDebug()<<QString("%1 try append buffer to file ....").arg(LTime::strCurrentTime());
    fname = fname.trimmed();
    ok = false;

    // check state
    QString err;
    if (fname.isEmpty())
    {
        qWarning("LZipObj::appendBufferToFile WARNING fname is empty");
        emit signalError("LZipObj::appendBufferToFile WARNING fname is empty");
        return;
    }
    if (!LFile::dirExists(m_workingFolder))
    {
        err = QString("LZipObj::writeBufferToFile WARNING invalid working folder - [%1]").arg(m_workingFolder);
        qWarning()<<err;
        emit signalError(err);
        return;
    }
    if (m_buffer.isEmpty())
    {
        err = QString("LZipObj::appendBufferToFile WARNING buffer is empty");
        qWarning()<<err;
        emit signalError(err);
        return;
    }

    //append operation
    QString full_name = QString("%1%2%3").arg(m_workingFolder).arg(QDir::separator()).arg(fname);
    qDebug()<<QString("try append buffer (%1 bytes) to file - [%2]").arg(m_buffer.size()).arg(full_name);
    if (!LFile::fileExists(full_name)) // need write new file
    {
        writeBufferToFile(fname, ok);
        return;
    }

    err = LFile::appendFileBA(full_name, m_buffer);
    if (!err.isEmpty())
    {
        err = QString("LZipObj::appendBufferToFile  %1").arg(err);
        qWarning()<<err;
        emit signalError(err);
        return;
    }

    ok = true;
    qDebug()<<QString("%1 append finished!").arg(LTime::strCurrentTime());
}




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
      m_workingFolder(QString())
{
    setObjectName("zip_obj");
//    connect(this, SIGNAL(signalFinished()), this, SLOT(slotParentFinished()));


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




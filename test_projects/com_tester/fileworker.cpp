#include "fileworker.h"
#include "lfile.h"
#include "lstatic.h"


#include <QDir>
#include <QApplication>
#include <QDate>
#include <QTime>

#define SPACE_SYMBOL    QString(" ")
#define COMMENT_SYMBOL    QString("#")

/////////////// FileWorker ///////////////////
FileWorker::FileWorker(QObject *parent)
    :LSimpleObject(parent)
{


}
QString FileWorker::saveFileName() const
{
    QString path = QApplication::applicationDirPath();
    QDir dir(QString("%1%2%3").arg(path).arg(QDir::separator()).arg("received"));
    if (dir.exists()) path = dir.path();

    QString s_date = QDate::currentDate().toString("MMdd");
    return QString("%1%2buff_%3.dat").arg(path).arg(QDir::separator()).arg(s_date);
}
void FileWorker::setInputFile(const QString &fname)
{
    QString msg = QString("try update sending buffer from file: [%1]").arg(fname);
    emit signalMsg(msg);

    m_writeBuffer.clear();
    if (fname.trimmed().isEmpty())
    {
        emit signalError("input filename is empty");
        return;
    }

    QStringList list;
    QString err = LFile::readFileSL(fname, list);
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }

    parseInputFileData(list);

    msg = QString("new bufer size %1").arg(m_writeBuffer.count());
    emit signalMsg(msg);
    emit signalMsg("Ok!");

}
void FileWorker::parseInputFileData(const QStringList &list)
{
    if (list.isEmpty()) return;
    int n = list.count();
    for (int i=0; i<n; i++)
    {
        QString s = list.at(i).trimmed();
        if (s.isEmpty()) continue;
        if (s.left(1) == COMMENT_SYMBOL) continue;

        int pos = s.indexOf(COMMENT_SYMBOL);
        if (pos > 0) s = s.left(pos).trimmed();

        parseFileLine(s);
    }
}
void FileWorker::parseFileLine(const QString &s)
{
    bool ok;
    QStringList bytes = s.split(SPACE_SYMBOL);
    for (int i=0; i<bytes.count(); i++)
    {
        QString s_byte = bytes.at(i).trimmed().toLower();
        if (s_byte.length() != 2) continue;

        char v = s_byte.toInt(&ok, 16);
        if (ok) m_writeBuffer.append(v);
    }
}
void FileWorker::saveBuffer(const QByteArray &ba)
{
    QString fname(saveFileName().trimmed());
    if (fname.isEmpty())
    {
        emit signalError("saving filename is empty");
        return;
    }

    QString err;
    if (!LFile::fileExists(fname))
    {
        emit signalMsg(QString("try create saving file: %1 ......").arg(fname));
        err = LFile::fileCreate(fname);
        if (!err.isEmpty())
        {
            emit signalError(err);
            return;
        }
        else emit signalMsg("Ok!");
    }

    QString data = "\n\n";
    data.append(QString("-------------- %1 --------------- \n").arg(QTime::currentTime().toString("hh:mm:ss.zzz")));
    data.append(LStatic::baToStr(ba));
    data.append("\n");

    err = LFile::appendFile(fname, data);
    if (!err.isEmpty())
        emit signalError(QString("save buffer to file: %1").arg(err));

}





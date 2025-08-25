#include "lfile.h"

#include <QTextStream>
#include <QDir>
#include <QFileInfo>

QString LFile::readFileBA(QString fname, QByteArray &ba)
{
    ba.clear();
    QFile f(fname);
    if (!f.exists()) return QString("file [%1] not found!").arg(fname);
    if (!f.open(QIODevice::ReadOnly)) return QString("file [%1] not open for reading!").arg(fname);

    ba = f.readAll();
    f.close();
    return QString();
}
QString LFile::readFileStr(QString fname, QString &str)
{
    str.clear();
    QByteArray ba;
    QString err(readFileBA(fname, ba));
    if (!err.isEmpty()) return err;
    str.append(ba);
    return QString();
}


QString LFile::readFileSL(QString fname, QStringList &list, QString spliter)
{
    list.clear();
    QString str;
    QString err(readFileStr(fname, str));
    if (!err.isEmpty()) return err;
    list.append(str.split(spliter));
    return QString();
}

QString LFile::writeFileSL(QString fname, const QStringList &list)
{
    if (fname.trimmed().isEmpty()) return QString("filename is empty!");

    if (list.isEmpty()) return QString("list is empty!");
    QFile f(fname);
    if (!f.open(QIODevice::WriteOnly))
        return QString("file [%1] not open for writing!").arg(fname);

    QTextStream stream(&f);
    for (int i=0; i<list.count(); i++)
    stream<<list.at(i)<<"\n";
    f.close();
    return QString();
}
bool LFile::fileExists(QString fname)
{
    if (fname.trimmed().isEmpty()) return false;
    return QFileInfo::exists(fname);
}
QString LFile::filePath(QString fname)
{
    if (fname.trimmed().isEmpty()) return QString();
    if (!fileExists(fname)) return QString();
    return rootPath(fname);
}
QString LFile::appPath()
{
    return QDir::currentPath();
}
bool LFile::dirExists(QString dir_path)
{
    if (dir_path.trimmed().isEmpty()) return false;
    QDir dir(dir_path.trimmed());
    return dir.exists();
}
int LFile::fileSizeB(QString fname)
{
    if (!fileExists(fname)) return -1;
    QFileInfo fi(fname);
    return fi.size();
}
int LFile::fileSizeKB(QString fname)
{
    int sb = fileSizeB(fname);
    if (sb <= 0) return sb;
    return sb/1024;
}
int LFile::fileSizeMB(QString fname)
{
    int sb = fileSizeKB(fname);
    if (sb <= 0) return sb;
    return sb/1024;
}
QString LFile::shortFileName(QString full_name)
{
    QFileInfo fi(full_name);
    return fi.fileName();
}
QString LFile::fileExtension(QString full_name)
{
    QString fname = LFile::shortFileName(full_name).trimmed();
    int pos = fname.lastIndexOf(".");
    if (pos < 0 || pos == (fname.length()-1)) return QString();
    return fname.right(fname.length()-pos-1);
}
bool LFile::isXmlFile(QString full_name)
{
    if (full_name.trimmed().length() < 4) return false;
    return (LFile::fileExtension(full_name) == "xml");
}
bool LFile::isTxtFile(QString full_name)
{
    if (full_name.trimmed().length() < 4) return false;
    return (LFile::fileExtension(full_name) == "txt");
}
bool LFile::isJsFile(QString full_name)
{
    if (full_name.trimmed().length() < 4) return false;
    return (LFile::fileExtension(full_name) == "js");
}
bool LFile::isJsonFile(QString full_name)
{
    if (full_name.trimmed().length() < 5) return false;
    return (LFile::fileExtension(full_name) == "json");
}
bool LFile::isHtmlFile(QString full_name)
{
    if (full_name.trimmed().length() < 5) return false;
    return (LFile::fileExtension(full_name) == "html");
}
QString LFile::rootPath(QString full_name)
{
    if (full_name.trimmed().isEmpty()) return QString();
    QFileInfo fi(full_name);
    QString s = fi.absolutePath().trimmed();
    if (s.right(1) == QDir::separator()) s = s.left(s.length()-1);
    return s;
}
QString LFile::shortDirName(QString full_name)
{
    QDir dir(full_name);
    return dir.dirName();
}
QString LFile::fileCreate(QString fname)
{
    QString err;
    if (fname.trimmed().isEmpty())
    {
        err = QString("filename is empty!");
        return err;
    }

    if (LFile::fileExists(fname))
    {
        err = QString("file [%1] allready exists!");
        return err;
    }

    err = LFile::writeFile(fname, QString());
    return err;
}
QString LFile::writeFile(QString fname, const QString &data)
{
    QFile f(fname);
    if (!f.open(QIODevice::WriteOnly))
        return QString("file [%1] not open for writing!").arg(fname);

    QTextStream stream(&f);
    stream<<data;
    f.close();
    return QString();
}
QString LFile::writeFileBA(QString fname, const QByteArray &ba)
{
    if (fname.trimmed().isEmpty()) return QString("filename is empty!");
    //if (!LFile::fileExists(fname))  return QString("filename [%1] not found!").arg(fname);

    QFile f(fname);
    if (!f.open(QIODevice::WriteOnly))
        return QString("file [%1] not open for writing!").arg(fname);

    f.write(ba.data(), ba.size());
    f.close();
    return QString();
}
QString LFile::appendFile(QString fname, const QString &data)
{
    QFile f(fname);
    if (!f.open(QIODevice::Append))
        return QString("file [%1] not open for append data!").arg(fname);

    QTextStream stream(&f);
    stream<<data;
    f.close();
    return QString();
}
QString LFile::appendFileSL(QString fname, const QStringList &list)
{
    if (fname.trimmed().isEmpty()) return QString("filename is empty!");
    if (list.isEmpty()) return QString("data list is empty!");

    int n = list.count();
    if (fileExists(fname))
    {
        QFile f(fname);
        if (!f.open(QIODevice::Append))
            return QString("file [%1] not open for append data!").arg(fname);

        QTextStream stream(&f);
        for(int i=0; i<n; i++)
            stream<<list.at(i)<<"\n";
        f.close();
    }
    else
    {
        QString err = writeFileSL(fname, list);
        if (!err.isEmpty()) return err;
    }
    return QString();
}
QString LFile::dirFolders(QString dir_path, QStringList &list, QString filter_text)
{
    list.clear();

    QString err;
    if (dir_path.trimmed().isEmpty())
    {
        err = QString("Dir path is empty!");
        return err;
    }

    QDir dir(dir_path.trimmed());
    if (!dir.exists())
    {
        err = QString("Dir path [%1] not found!").arg(dir.path());
        return err;
    }

    QStringList dir_list = dir.entryList(QDir::AllDirs);
    for (int i=0; i<dir_list.count(); i++)
    {
        QString s = dir_list.at(i).trimmed();
        if (s.isEmpty() || s =="." || s =="..") continue;
        if (!filter_text.isEmpty() && s.contains(filter_text)) continue;
        list.append(QString("%1%2%3").arg(dir.path()).arg(QDir::separator()).arg(s));
    }
    return err;
}
QString LFile::dirFiles(QString dir_path, QStringList &list, QString ftype)
{
    list.clear();

    QString err;
    if (dir_path.trimmed().isEmpty())
    {
        err = QString("Data path is empty!");
        return err;
    }

    QDir dir(dir_path.trimmed());
    if (!dir.exists())
    {
        err = QString("Data path [%1] not found!").arg(dir.path());
        return err;
    }

    QStringList dir_list = dir.entryList(QDir::Files);
    for (int i=0; i<dir_list.count(); i++)
    {
        QString s = dir_list.at(i).trimmed();
        if (s.length() < 3) continue;
        if (!ftype.trimmed().isEmpty() && !s.contains(QString(".%1").arg(ftype.trimmed()))) continue;
        list.append(QString("%1%2%3").arg(dir.path()).arg(QDir::separator()).arg(s));
    }

    return err;
}




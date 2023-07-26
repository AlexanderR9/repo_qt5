#include "lstatic.h"

#include <QList>
#include <QTextCodec>
#include <QDebug>
#include <QTime>
#include <QDateTime>

//////////////// LStatic /////////////////////////
int LStatic::subCount(const QString &s, const QString sub_s)
{
    int n = 0;
    int start_pos = 0;
    for (;;)
    {
        int pos = s.indexOf(sub_s, start_pos);
        if (pos < 0) break;

        n++;
        start_pos = pos + 1;
        if (start_pos >= s.length()) break;
    }
    return n;
}
QList<int> LStatic::subStrIndexes(const QString &s, const QString sub_s)
{
    QList<int> list;
    int start_pos = 0;
    for (;;)
    {
        int pos = s.indexOf(sub_s, start_pos);
        if (pos < 0) break;

        list.append(pos);
        start_pos = pos + 1;
        if (start_pos >= s.length()) break;
    }
    return list;
}
int LStatic::strIndexOfByEnd(const QString &s, const QString sub_s)
{
    int last_pos = -1;
    int start_pos = 0;            
    for (;;)
    {
        int pos = s.indexOf(sub_s, start_pos);
        if (pos < 0) break;

        last_pos = pos;
        start_pos = pos + 1;
        if (start_pos >= s.length()) break;
    }
    return last_pos;
}
QString LStatic::fromColor(const QColor &color, QString split_symbol)
{
    return QString("(%1%2 %3%4 %5)").arg(color.red()).arg(split_symbol).arg(color.green()).arg(split_symbol).arg(color.blue());
}
QStringList LStatic::trimSplitList(const QString &data, QString split_symbol, bool remove_empty_line)
{
    QStringList list = data.split(split_symbol);
    if (!remove_empty_line) return list;

    int n = list.count();
    for (int i=n-1; i>=0; i--)
    {
        QString s = list.at(i).trimmed();
        if (s.isEmpty()) list.removeAt(i);
        else list.replace(i, s);
    }
    return list;
}
QString LStatic::removeLongSpaces(const QString &s, bool remove_tabs)
{
    QString space(" ");
    QString space2("  ");
    QString space160(QChar(160));
    QString result = s.trimmed();

    result.replace(space160, space);

    if (remove_tabs)
        result.replace("\t", space);

    while (result.contains(space2))
        result.replace(space2, space);

    return result;
}
QString LStatic::baToStr(const QByteArray &ba, int line_size, bool with_int_values)
{
    QString s;
    if (ba.isEmpty()) return QString("ByteArray is empty!");

    int cur_len = 0;
    for (int i=0; i<ba.count(); i++)
    {
        uchar uc = uchar(ba.at(i));
        QString s16 = QString::number(ba.at(i), 16);
        if (s16.isEmpty()) continue;
        if (s16.length() > 2)
        {
            s16 = s16.right(2);
            //qWarning()<<QString("WARNING: LStatic::baToStr   s16.length(%1) > 2, i=%2").arg(s16.length()).arg(i);
        }
        if (s16.length() == 1) s16 = s16.prepend("0");
        if (with_int_values)
            s16 = QString("%1(%2)").arg(s16).arg(uc);

        s.append(s16);
        s.append(QString("  "));
        cur_len++;

        if (line_size > 0 && cur_len == line_size)
        {
            s.append(QString("\n"));
            cur_len = 0;
        }
    }
    return s;
}
QColor LStatic::strToColor(const QString &str, QString split_symbol)
{
    qDebug()<<QString("str");
    QColor color(0, 0, 0);

    int pos1 = str.indexOf("(");
    int pos2 = str.indexOf(")");
    if (pos1 < 0 || pos1 >= pos2) return color;

    qDebug("2");
    QString s = str.mid(pos1+1, pos2-pos1-1).trimmed();
    QStringList list = s.split(split_symbol);
    if (list.count() < 3) return color;
    
    qDebug()<<s;
    qDebug("3");
    bool ok;
    int r = list.at(0).toInt(&ok);
    if (!ok || r < 0 || r > 255) return color;
    int g = list.at(1).toInt(&ok);
    if (!ok || g < 0 || g > 255) return color;
    int b = list.at(2).toInt(&ok);
    if (!ok || b < 0 || b > 255) return color;

    qDebug()<<QString("r=%1  g=%2  b=%3").arg(r).arg(g).arg(b);

    return QColor(r, g, b);
}
QString LStatic::strTrimLeft(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.right(s.length()-n);
}
QString LStatic::strAddLeft(const QString &s, int n,  QChar c)
{
    if (n <= 0) return s;
    QString cs = QString(n, c);
    return QString("%1%2").arg(cs).arg(s);
}
QString LStatic::strAlignLeft(const QString &s, int n,  QChar c)
{
    if (n <= s.length()) return s;
    int n_left = n - s.length();
    return strAddLeft(s, n_left, c);
}
QString LStatic::strTrimRight(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.left(s.length()-n);
}
QString LStatic::strAddRight(const QString &s, int n,  QChar c)
{
    if (n <= 0) return s;
    QString cs = QString(n, c);
    return QString("%1%2").arg(s).arg(cs);
}
QString LStatic::strAlignRight(const QString &s, int n,  QChar c)
{
    if (n <= s.length()) return s;
    int n_right = n - s.length();
    return strAddRight(s, n_right, c);
}
QString LStatic::strBetweenStr(const QString &s, QString s1, QString s2)
{
    if (s.isEmpty() || s1.isEmpty() || s2.isEmpty()) return QString();
    int pos1 = s.indexOf(s1);
    int pos2 = s.indexOf(s2);
    if (pos1 < 0 || pos2 < 0 || pos2 <= pos1) return QString();
    return s.mid(pos1+1, pos2-pos1-1);
}
QString LStatic::replaceByRange(const QString &s, const QString sub_s1, const QString sub_s2, quint16 pos1, quint16 pos2)
{
    if ((pos1 >= pos2) || (pos1 >= s.length()-1)) return s;
    if (pos2  > s.length()-1) pos2 = s.length()-1;

    QString left = s.left(pos1+1);
    QString right = s.right(s.length() - pos2);
    QString mid = s.mid(pos1+1, s.length() - left.length() - right.length());
    mid.replace(sub_s1, sub_s2);
    return (left+mid+right);
}
QString LStatic::fromCodec(const QString &s, QString codec_name)
{
    if (!codec_name.isEmpty())
    {
        QTextCodec *codec = QTextCodec::codecForName(codec_name.toLatin1());
        if (codec) return codec->toUnicode(s.toLatin1());
        else qWarning("err codec");
    }
    return s;
}
QString LStatic::systemInfo()
{
    QString s("\n");
#ifdef Q_OS_LINUX
    s += QString("-------------------- OS_LINUX ---------------------");
#elif Q_OS_WIN32
    s += QString("------------ OS_WIN32 ----------------");
#elif Q_OS_WIN64
    s += QString("------------ OS_WIN64 ----------------");
#elif Q_OS_SOLARIS
    s += QString("------------ OS_SOLARIS ----------------");
#elif Q_OS_MAC
    s += QString("------------ OS_MAC ----------------");
#elif Q_OS_ANDROID
    s += QString("------------ OS_ANDROID ----------------");
#else
    s += QString("------------ OS_UNKNOWN ----------------");
#endif
    s += QString("\n");
    s += QString("name = [%1] \n").arg(QSysInfo::prettyProductName());
    s += QString("build_abi = [%1] \n").arg(QSysInfo::buildAbi());
    s += QString("cpu_architecture = [%1] \n").arg(QSysInfo::currentCpuArchitecture());
    s += QString("build_cpu_architecture = [%1] \n").arg(QSysInfo::buildCpuArchitecture());
    s += QString("product: type/version = [%1 / %2] \n").arg(QSysInfo::productType()).arg(QSysInfo::productVersion());
    s += QString("kernel: type/version = [%1 / %2] \n").arg(QSysInfo::kernelType()).arg(QSysInfo::kernelVersion());
    s += QString("-------------------------------------------------------\n");
    return s;
}


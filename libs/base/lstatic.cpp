#include "lstatic.h"

#include <QList>
#include <QTextCodec>
#include <QDebug>
#include <QTime>
#include <QDateTime>

//////////////// LStatic /////////////////////////
QString LStatic::strCurrentTime(bool with_ms)
{
    QString mask = with_ms ? "hh:mm:ss_zzz" : "hh:mm:ss";
    return QTime::currentTime().toString(mask);
}
QString LStatic::strCurrentDateTime(QString mask)
{
    return QDateTime::currentDateTime().toString(mask);
}
QString LStatic::strCurrentDate(QString mask)
{
    return QDate::currentDate().toString(mask);
}
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
QString LStatic::strTrimRight(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.left(s.length()-n);
}
QString LStatic::strBetweenStr(const QString &s, QString s1, QString s2)
{
    if (s.isEmpty() || s1.isEmpty() || s2.isEmpty()) return QString();
    int pos1 = s.indexOf(s1);
    int pos2 = s.indexOf(s2);
    if (pos1 < 0 || pos2 < 0 || pos2 <= pos1) return QString();
    return s.mid(pos1+1, pos2-pos1-1);
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
void LStatic::setAttrNode(QDomElement &node, QString a1, QString v1, QString a2, QString v2, QString a3, QString v3, QString a4, QString v4, QString a5, QString v5)
{
    if (node.isNull()) return;
    if (a1.trimmed().isEmpty()) return;
    node.setAttribute(a1.trimmed(), v1.trimmed());
    if (a2.trimmed().isEmpty()) return;
    node.setAttribute(a2.trimmed(), v2.trimmed());
    if (a3.trimmed().isEmpty()) return;
    node.setAttribute(a3.trimmed(), v3.trimmed());
    if (a4.trimmed().isEmpty()) return;
    node.setAttribute(a4.trimmed(), v4.trimmed());
    if (a5.trimmed().isEmpty()) return;
    node.setAttribute(a5.trimmed(), v5.trimmed());
}
double LStatic::getDoubleAttrValue(const QString &attr_name, const QDomNode &node, double defValue)
{
    QString s_value = getStringAttrValue(attr_name, node, "err");
    if (s_value == "err") return defValue;

    bool ok;
    double a = s_value.toDouble(&ok);
    return (ok ? a : defValue);
}
int LStatic::getIntAttrValue(const QString &attr_name, const QDomNode &node, int defValue)
{
    if (attr_name.trimmed().isEmpty() || node.isNull()) return defValue;
    if (!node.toElement().hasAttribute(attr_name.trimmed()))  return defValue;
    QString s_value = node.toElement().attribute(attr_name.trimmed());

    bool ok;
    int value = s_value.toInt(&ok);
    if (!ok) return defValue;
    return value;
}
QString LStatic::getStringAttrValue(const QString &attr_name, const QDomNode &node, QString defValue)
{
    if (attr_name.trimmed().isEmpty() || node.isNull()) return defValue;
    if (!node.toElement().hasAttribute(attr_name.trimmed()))  return defValue;
    return node.toElement().attribute(attr_name.trimmed());
}
void LStatic::createDomHeader(QDomDocument &dom)
{
    dom.appendChild(dom.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
}




#include "lstring.h"

int LString::subCount(const QString &s, const QString sub_s)
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
int LString::strIndexOfByEnd(const QString &s, const QString sub_s)
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
QList<int> LString::subStrIndexes(const QString &s, const QString sub_s)
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
QString LString::strBetweenStr(const QString &s, QString s1, QString s2)
{
    if (s.isEmpty() || s1.isEmpty() || s2.isEmpty()) return QString();
    int pos1 = s.indexOf(s1);
    if (pos1 < 0) return QString();
    if (s1.length() > 1) pos1 += (s1.length()-1);

    int pos2 = s.indexOf(s2, pos1+1);
    if (pos1 < 0 || pos2 < 0 || pos2 <= pos1) return QString();
    return s.mid(pos1+1, pos2-pos1-1);
}
QString LString::replaceByRange(const QString &s, const QString sub_s1, const QString sub_s2, quint16 pos1, quint16 pos2)
{
    if ((pos1 >= pos2) || (pos1 >= s.length()-1)) return s;
    if (pos2  > s.length()-1) pos2 = s.length()-1;

    QString left = s.left(pos1+1);
    QString right = s.right(s.length() - pos2);
    QString mid = s.mid(pos1+1, s.length() - left.length() - right.length());
    mid.replace(sub_s1, sub_s2);
    return (left+mid+right);
}
QString LString::strTrimLeft(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.right(s.length()-n);
}
QString LString::strAddLeft(const QString &s, int n,  QChar c)
{
    if (n <= 0) return s;
    QString cs = QString(n, c);
    return QString("%1%2").arg(cs).arg(s);
}
QString LString::strAlignLeft(const QString &s, int n,  QChar c)
{
    if (n <= s.length()) return s;
    int n_left = n - s.length();
    return strAddLeft(s, n_left, c);
}
QString LString::strTrimRight(const QString &s, int n)
{
    if (n <= 0) return s;
    if (n >= s.length()) return QString();
    return s.left(s.length()-n);
}
QString LString::strAddRight(const QString &s, int n,  QChar c)
{
    if (n <= 0) return s;
    QString cs = QString(n, c);
    return QString("%1%2").arg(s).arg(cs);
}
QString LString::strAlignRight(const QString &s, int n,  QChar c)
{
    if (n <= s.length()) return s;
    int n_right = n - s.length();
    return strAddRight(s, n_right, c);
}
QString LString::uniteList(const QStringList &list, QString split_symbol)
{
    if (list.isEmpty()) return "EMPTY";

    QString s;
    int n = list.count();
    for (int i=0; i<n; i++)
    {
        s.append(list.at(i));
        if (i < (n-1)) s.append(split_symbol);
    }
    return s;
}
QStringList LString::trimSplitList(const QString &data, QString split_symbol, bool remove_empty_line)
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
QString LString::removeLongSpaces(const QString &s, bool remove_tabs)
{
    QString space2("  ");
    QString space160(QChar(160));
    QString result = s.trimmed();
    result.replace(space160, LString::spaceSymbol());

    if (remove_tabs)
        result.replace("\t", LString::spaceSymbol());

    while (result.contains(space2))
        result.replace(space2, LString::spaceSymbol());

    return result;
}
QString LString::removeSpaces(const QString &s)
{
    if (s.trimmed().isEmpty()) return QString();
    QString s2 = LString::removeLongSpaces(s);
    s2.replace(LString::spaceSymbol(), QString());
    return s2;
}
QString LString::removeSymbol(const QString &s, QChar c)
{
    if (s.trimmed().isEmpty()) return QString();
    QString result = s;
    result.replace(c, QString());
    return result;
}
void LString::removeEmptyStrings(QStringList &list, bool remove_spaces)
{
    if (list.isEmpty()) return;

    int n = list.count();
    for (int i=n-1; i>=0; i--)
    {
        QString s = list.at(i);
        if (s.isEmpty()) {list.removeAt(i); continue;}
        if (remove_spaces && s.trimmed().isEmpty()) list.removeAt(i);
    }
}
QStringList LString::toUnicode(const QString &s)
{
    QStringList result;
    if (s.isEmpty()) return result;

    int n = s.length();
    for (int i=0; i<n; i++)
    {
        QChar c_pos(s.at(i));
        QString line = QString("%1. [%2]  code=%3").arg(i+1).arg(c_pos).arg(c_pos.unicode());
        result.append(line);
    }
    return result;
}



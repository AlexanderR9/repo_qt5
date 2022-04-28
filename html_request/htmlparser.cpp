#include "htmlparser.h"


#include <QDebug>
#include <QTextDocument>
#include <QTextFrame>
#include <QElapsedTimer>

#define SERVICE_SYMBOL1     QString("\t")
#define SERVICE_SYMBOL2     QString("\r")
#define SERVICE_SYMBOL3     QString("\n")
#define SPACE_SYMBOL        QString(" ")


MyHTMLParser::MyHTMLParser()
    :QObject(0)
{
    //connect(&w_page, SIGNAL(loadStarted()), this, SIGNAL(signalStart()));
    //connect(&w_page, SIGNAL(loadFinished(bool)), this, SIGNAL(signalFinished()));

}
void MyHTMLParser::tryParseHtmlText(const QString &data)
{
    m_err.clear();
    header_data.clear();
    body_data.clear();

    if (data.trimmed().isEmpty())
    {
        m_err = QString("html data is empty!");
        return;
    }

    QString mod_data = removeComment(data);
    mod_data = replaceServiceSymbol(mod_data, SERVICE_SYMBOL1);
    mod_data = replaceServiceSymbol(mod_data, SERVICE_SYMBOL2);
    mod_data = replaceServiceSymbol(mod_data, SERVICE_SYMBOL3);
    mod_data = replaceBigSpaces(mod_data);

    header_data = extractHtmlNode(mod_data, "head");
    body_data = extractHtmlNode(mod_data, "body");

}
QString MyHTMLParser::extractHtmlNode(const QString &s, QString node_name) const
{
    QString result;
    int pos1 = s.indexOf(QString("<%1").arg(node_name));
    if (pos1 >= 0)
    {
        QString end_s = QString("</%1>").arg(node_name);
        int pos2 = s.indexOf(end_s, pos1+1);
        if (pos2 > pos1) result = s.mid(pos1, pos2-pos1+end_s.length());
    }
    return result;
}
QString MyHTMLParser::removeComment(const QString &s) const
{
    QString result = s.trimmed();
    bool was_remove = false;

    QString cs1("<!--");
    QString cs2("-->");

    int pos1 = result.indexOf(cs1);
    if (pos1 >= 0)
    {
        int pos2 = result.indexOf(cs2);
        if (pos2 > pos1)
        {
            result.remove(pos1, pos2 - pos1 + cs2.length());
            was_remove = true;
        }
    }

    if (was_remove) return removeComment(result);
    return result;
}
QString MyHTMLParser::replaceServiceSymbol(const QString &s, QString service_symb) const
{
    QString result = s.trimmed();
    if (!result.contains(service_symb)) return result;

    result.replace(service_symb, SPACE_SYMBOL);
    return replaceServiceSymbol(result, service_symb);
}
QString MyHTMLParser::replaceBigSpaces(const QString &s) const
{
    QString result = s.trimmed();
    QString big_space("                               ");
    while (big_space.length() > 1)
    {
        if (result.contains(big_space)) result.replace(big_space, SPACE_SYMBOL);
        else big_space = big_space.left(big_space.length()-1);
    }
    return result;
}
void MyHTMLParser::parseBodyNode()
{
    body_node.reset();
    if (body_data.isEmpty())
    {
        qWarning("MyHTMLParser::parseBodyNode()  WARNING - body_data.isEmpty()");
        return;
    }



}



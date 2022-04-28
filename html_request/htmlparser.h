#ifndef MY_HTMLPARSER_H
#define MY_HTMLPARSER_H

#include <QObject>


class MyHTMLNodeBase
{
public:
    MyHTMLNodeBase() :m_parent(NULL) {}
    virtual ~MyHTMLNodeBase() {reset();}

    void reset() {qDeleteAll(m_childs); m_childs.clear();}

protected:
    QList<MyHTMLNodeBase*> m_childs;
    MyHTMLNodeBase *m_parent;

};


//класс для парсинга html кода заданной страницы
// MyHTMLParser
class MyHTMLParser : public QObject
{
    Q_OBJECT
public:
    MyHTMLParser();
    virtual ~MyHTMLParser() {}

    void tryParseHtmlText(const QString&);
    inline QString bodyData() const {return body_data;}
    inline QString headData() const {return header_data;}

protected:
    QString m_err;
    QString header_data;
    QString body_data;

    MyHTMLNodeBase body_node;

    void parseBodyNode();

signals:
    void signalStart();
    void signalFinished();

private:
    QString removeComment(const QString&) const;
    QString replaceServiceSymbol(const QString&, QString) const;
    QString replaceBigSpaces(const QString&) const;
    QString extractHtmlNode(const QString&, QString) const;

};



#endif // HTMLPARSER_H

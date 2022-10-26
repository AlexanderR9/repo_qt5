#ifndef MY_HTMLPARSER_H
#define MY_HTMLPARSER_H

#include <QObject>
#include <QList>


class MyHTMLNode;
class MyHTMLScriptNode;


//класс контейнер тегов с описанием их особенностей
class MyHTMLTag
{
public:
    struct Tag
    {
        Tag() {need_close = 1; js = false;}
        Tag(QString s, int x, bool script = false) :name(s), need_close(x), js(script) {}

        QString name;
        int need_close; // 1 - должен закрываться, 2 - должен не закрываться, 0 - неопределенное поведение
        bool js;
    };

    MyHTMLTag();

    inline int count() const {return m_tags.count();}
    int needClosedCode(QString) const; //-1 означает что такой тег не найден в контейнере m_tags

protected:
    QList<Tag> m_tags; //контейнер со всеми возможными тегами

    void initListTags();

};


//класс для парсинга html кода заданной страницы
// MyHTMLParser
class MyHTMLParser : public QObject
{
    Q_OBJECT
public:
    MyHTMLParser(QObject *parent = NULL);
    virtual ~MyHTMLParser() {}

    void tryParseHtmlText(const QString&);
    inline QString bodyData() const {return body_data;}
    inline QString headData() const {return header_data;}
    void reset();

    //извлекает все содержимое внутри тега (сам тег отбрасывется),
    //тег может быть закрывающимся или нет.
    //подразумевается что html этой ноды корректный.
    //входные данные будут модифицированы.
    void trimFullNode(QString&) const;

protected:
    QString m_err;
    QString header_data;
    QString body_data;

    MyHTMLNode *body_node;
    MyHTMLTag m_tagObj;

    void parseBodyNode();
    void findScriptNodes(QString&); //найти все ноды js в содержимом body (это надо сделать предварительно, до загрузки body_node)
    void convertScriptCode(QString&, const MyHTMLScriptNode*); //заменить в общем коде html ноду script на нужный текст, либо просто вырезать эту ноду


signals:
    void signalStart();
    void signalFinished();

private:
    QString removeComment(const QString&) const;
    QString replaceServiceSymbol(const QString&, QString) const;
    QString replaceBigSpaces(const QString&) const;

    //извлечь html код заданной ноды, результат будет содержать этот же тег.
    //при ошибке вернет пустую строку
    QString extractHtmlNode(const QString&, QString) const;

};



#endif // HTMLPARSER_H

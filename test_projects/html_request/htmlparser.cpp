#include "htmlparser.h"
#include "htmlnode.h"

#include <QDebug>
#include <QTextDocument>
#include <QTextFrame>
#include <QElapsedTimer>
//#include <QWebEngineView>


#define SERVICE_SYMBOL1     QString("\t")
#define SERVICE_SYMBOL2     QString("\r")
#define SERVICE_SYMBOL3     QString("\n")
#define SPACE_SYMBOL        QString(" ")
#define START_TAG_SYMBOL    QString("<")
#define END_TAG_SYMBOL      QString(">")


MyHTMLParser::MyHTMLParser(QObject *parent)
    :QObject(parent),
      body_node(NULL)
{
    body_node = new MyHTMLNode(NULL);

    qDebug()<<QString("init tags %1").arg(m_tagObj.count());
}
void MyHTMLParser::reset()
{
    m_err.clear();
    header_data.clear();
    body_data.clear();

    body_node->reset();
}
void MyHTMLParser::tryParseHtmlText(const QString &data)
{
    m_err.clear();
    header_data.clear();
    body_node->reset();

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

    findScriptNodes(body_data);

    //parseBodyNode();
    qDebug()<<QString("internal body data size %1").arg(body_node->dataSize());



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
    body_node->reset();
    if (body_data.isEmpty())
    {
        qWarning("MyHTMLParser::parseBodyNode()  WARNING - body_data.isEmpty()");
        return;
    }

    QString internal_data = body_data;
    trimFullNode(internal_data);
    if (internal_data.isEmpty())
    {
        qWarning("MyHTMLParser::parseBodyNode()  WARNING - internal_data.isEmpty()");
        return;
    }
    if (internal_data == "err")
    {
        qWarning("MyHTMLParser::parseBodyNode()  WARNING - internal_data is err");
        return;
    }

    body_node->setData(internal_data);

}
void MyHTMLParser::trimFullNode(QString &s) const
{
    qDebug("------------------raw data-----------------------");
    qDebug()<<s;


    s = s.trimmed();
    if (s.isEmpty())
    {
        s = "err";
        qWarning("MyHTMLParser::trimFullNode - WARNING: node text is empty.");
        return;
    }
    if (s.left(1) != START_TAG_SYMBOL)
    {
        s = "err";
        qWarning("MyHTMLParser::trimFullNode - WARNING: node text invalid, start symbol != [<].");
        return;
    }

    int start_pos_tag_name = 1;
    int end_pos_tag_name = s.indexOf(END_TAG_SYMBOL);
    int space_pos = s.indexOf(SPACE_SYMBOL);
    if (space_pos > 0 && space_pos < end_pos_tag_name) end_pos_tag_name = space_pos;
    if (end_pos_tag_name <= start_pos_tag_name)
    {
        s = "err";
        qWarning("MyHTMLParser::trimFullNode - WARNING: tag name positions not found.");
        return;
    }

    QString tag_name = s.mid(start_pos_tag_name, end_pos_tag_name-start_pos_tag_name);
    int close_code = m_tagObj.needClosedCode(tag_name);
    if (close_code < 0)
    {
        s = "err";
        qWarning()<<QString("MyHTMLParser::trimFullNode - WARNING: tag name[%1] not found.").arg(tag_name);
        return;
    }

    if (close_code == 1) //тег должен закрыться
    {
        int pos = s.lastIndexOf(QString("%1/%2").arg(START_TAG_SYMBOL).arg(tag_name));
        if (pos < 0)
        {
            s.clear(); //простая  нода без вложений
        }
        else
        {
            int pos0 = s.indexOf(END_TAG_SYMBOL);
            if (pos0 < 0)
            {
                s = "err";
                qWarning()<<QString("MyHTMLParser::trimFullNode - WARNING: not found ending header node, tag_name=[%1]").arg(tag_name);
                return;
            }
            qDebug()<<QString("pos0=%1  pos=%2").arg(pos0).arg(pos);
            s = s.mid(pos0+1, pos-pos0-1);
        }
    }
    else if (close_code == 2) // простой незакрывающийся тег
    {
        s.clear();
    }
    else //неопределенное поведение
    {
        int pos = s.lastIndexOf(QString("%1/%2").arg(START_TAG_SYMBOL).arg(tag_name));
        if (pos > s.length() - (tag_name.length()+3+2))
        {
            int pos0 = s.indexOf(END_TAG_SYMBOL);
            if (pos0 < 0)
            {
                s = "err";
                qWarning()<<QString("MyHTMLParser::trimFullNode - WARNING: not found ending header node, tag_name=[%1]").arg(tag_name);
                return;
            }
            s = s.mid(pos0+1, pos-pos0-1);
        }
        else
        {
            pos = s.indexOf(END_TAG_SYMBOL);
            if (pos < 0)
            {
                s = "err";
                qWarning()<<QString("MyHTMLParser::trimFullNode - WARNING: not found ending node, tag_name=[%1]").arg(tag_name);
                return;
            }
            if (pos < s.length()-2) s.right(s.length()-pos+2);
            else s.clear();
        }
    }


    qDebug("------------------extract data-----------------------");
    qDebug()<<s;
    qDebug("-----------------------------------------------------");

}
void MyHTMLParser::findScriptNodes(QString &s)
{
    if (s.trimmed().isEmpty()) return;

    QList<MyHTMLScriptNode*> list;
    int pos_start = 0;
    while (2 > 1)
    {
        pos_start = s.indexOf(QString("%1script").arg(START_TAG_SYMBOL), pos_start);
        if (pos_start < 0) break;

        int pos_end = s.indexOf(QString("%1/script%2").arg(START_TAG_SYMBOL).arg(END_TAG_SYMBOL), pos_start+1);
        if (pos_end < 0)
        {
            qWarning()<<QString("MyHTMLParser::findScriptNodes  WARNING - not found pos_end, pos_start=%1").arg(pos_start);
            break;
        }


        QString data = s.mid(pos_start, pos_end-pos_start+8+1);
        trimFullNode(data);
        if (data == "err")
        {
            qWarning()<<QString("MyHTMLParser::findScriptNodes  WARNING - invalid trim data of script, data=[%1]").arg(data);
        }
        else
        {
            list.append(new MyHTMLScriptNode(NULL));
            list.last()->setPositions(pos_start, pos_end+8);
            list.last()->setData(data);
        }

        pos_start = pos_end + 8;
    }

    qDebug()<<QString("finded scripts nodes %1").arg(list.count());
    if (list.isEmpty()) return;

    for (int i=list.count()-1; i>=0; i--)
        convertScriptCode(s, list.at(i));

}
void MyHTMLParser::convertScriptCode(QString &html_data, const MyHTMLScriptNode *js_node)
{
    if (!js_node || js_node->invalid()) return;

    QString js_code(js_node->internalData());
    //QWebEngineView view;

}


//MyHTMLTag
MyHTMLTag::MyHTMLTag()
{
    initListTags();
}
int MyHTMLTag::needClosedCode(QString s) const
{
    foreach (Tag t, m_tags)
        if (t.name == s) return t.need_close;
    return -1;
}
void MyHTMLTag::initListTags()
{
    m_tags.append(Tag("script", 1, true));
    m_tags.append(Tag("title", 1)); //заголовок вкладки браузера, обычно в заголовке HTML
    m_tags.append(Tag("a", 1)); // link
    m_tags.append(Tag("meta", 0)); //мета данные, обычно в заголовке HTML

    //незакрывающиеся теги
    m_tags.append(Tag("img", 2)); // картинка
    m_tags.append(Tag("br", 2)); //перенос строки
    m_tags.append(Tag("hr", 2)); //Рисует горизонтальную линию, всегда на новой строке

    //оформление текста
    m_tags.append(Tag("b", 1)); //делает шрифт жирным
    m_tags.append(Tag("i", 1)); //делает шрифт курсивным
    m_tags.append(Tag("u", 1)); //подчеркивает текст
    m_tags.append(Tag("s", 1)); //зачеркивает текст
    m_tags.append(Tag("center", 1)); //выравнивает текст по центру
    m_tags.append(Tag("font", 1)); //задает тип и цвет шрифта
    m_tags.append(Tag("strong", 1)); //предназначен для акцентирования текста. Браузеры отображают такой текст жирным начертанием.
    m_tags.append(Tag("span", 1)); //предназначен для оформления части строки/слова (например 1-й буквы)

    //заголовки  (<h1> - самый важный)
    m_tags.append(Tag("h1", 1));
    m_tags.append(Tag("h2", 1));
    m_tags.append(Tag("h3", 1));
    m_tags.append(Tag("h4", 1));
    m_tags.append(Tag("h5", 1));
    m_tags.append(Tag("h6", 1));

    //списки
    m_tags.append(Tag("ol", 1)); //Нумерованный список
    m_tags.append(Tag("ul", 1)); //Маркированный список
    m_tags.append(Tag("li", 0)); //элемент списка
    m_tags.append(Tag("dl", 1)); //Список определений
    m_tags.append(Tag("dt", 1)); //Каждый отдельный термин в списке определений
    m_tags.append(Tag("dd", 1)); //само определение к термину

    //таблицы
    m_tags.append(Tag("table", 1)); //Таблица. В одну таблицу вкладывать другую таблицу возможно, но это не рекомендуется
    m_tags.append(Tag("tr", 0)); //строка
    m_tags.append(Tag("td", 0)); //ячейка
    m_tags.append(Tag("caption", 1)); //предназначен для создания заголовка к таблице и может размещаться только внутри контейнера <table>

    //блочные теги
    m_tags.append(Tag("p", 1)); //используется для разделения текста на абзацы.
    m_tags.append(Tag("pre", 1)); //Задает блок предварительно форматированного текста.
    m_tags.append(Tag("blockquote", 1)); //Предназначен для выделения длинных цитат внутри документа.
    m_tags.append(Tag("div", 1)); //Предназначен для выделения длинных цитат внутри документа.
    m_tags.append(Tag("nav", 1)); //задает навигацию по сайту
    m_tags.append(Tag("section", 1)); //Задаёт раздел документа, может применяться для блока новостей
    m_tags.append(Tag("frameset", 1)); //Определяет структуру фреймов на веб-странице. Фреймы разделяют окно браузера на отдельные области
    m_tags.append(Tag("noscript", 1)); //показывает свое содержимое, если браузер не поддерживает работу со скриптами
    m_tags.append(Tag("body", 1)); //тело всего документа


    //теги с неопределенным поведением закрытия: tr, td, li, meta


}

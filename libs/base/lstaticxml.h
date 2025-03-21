#ifndef LSTATIC_XML_H
#define LSTATIC_XML_H

#include <QString>
#include <QDomElement>


//набор статических функций для удобства парсинга xml файлов.
//работа с объектами классов QDomNode, QDomElement и т.п.

///////////LStaticXML//////////////////////////
class LStaticXML
{
public:
    //for xml nodes
    static void setAttrNode(QDomElement&, QString a1, QString v1, QString a2 = QString(), QString v2 = QString(),
	    QString a3 = QString(), QString v3 = QString(), QString a4 = QString(), QString v4 = QString(),
	    QString a5 = QString(), QString v5 = QString());

    static int getIntAttrValue(const QString&, const QDomNode&, int defValue = -99); //get integer value of attribute, params: attr_name, node, default_value
    static double getDoubleAttrValue(const QString&, const QDomNode&, double defValue = -99); //get double value of attribute, params: attr_name, node, default_value
    static QString getStringAttrValue(const QString&, const QDomNode&, QString defValue = QString()); //get string value of attribute, params: attr_name, node, default_value
    static void createDomHeader(QDomDocument&); // for create first section xml node

    //открывает xml file, загружает QDomDocumet, извлекает рутовую ноду и записывает в параметр r_node,
    //если параметр need_root_node_name не пустой, то предварительно проверяет имя рутовой ноды (онодолжно совпадать с need_root_node_name).
    //в случае любой ошибки r_node не запишется, а функция вернет текст ошибки.
    static QString getDomRootNode(const QString& fname, QDomNode &r_node, QString need_root_node_name = QString());
	
    //проверка, имеет ли нода атрибут с указанным названием
    static bool hasAttr(QString attr_name, const QDomNode&);

};



#endif //LSTATIC_XML_H






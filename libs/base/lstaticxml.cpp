#include "lstaticxml.h"

#include <QDebug>
#include <QFile>
#include <QDomNodeList>


//////////////// LStaticXML /////////////////////////

void LStaticXML::setAttrNode(QDomElement &node, QString a1, QString v1, QString a2, QString v2, QString a3, QString v3, QString a4, QString v4, QString a5, QString v5)
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
double LStaticXML::getDoubleAttrValue(const QString &attr_name, const QDomNode &node, double defValue)
{
    QString s_value = getStringAttrValue(attr_name, node, "err");
    if (s_value == "err") return defValue;

    bool ok;
    double a = s_value.toDouble(&ok);
    return (ok ? a : defValue);
}
int LStaticXML::getIntAttrValue(const QString &attr_name, const QDomNode &node, int defValue)
{
    if (attr_name.trimmed().isEmpty() || node.isNull()) return defValue;
    if (!node.toElement().hasAttribute(attr_name.trimmed()))  return defValue;
    QString s_value = node.toElement().attribute(attr_name.trimmed());

    bool ok;
    int value = s_value.toInt(&ok);
    if (!ok) return defValue;
    return value;
}
QString LStaticXML::getStringAttrValue(const QString &attr_name, const QDomNode &node, QString defValue)
{
    if (attr_name.trimmed().isEmpty() || node.isNull()) return defValue;
    if (!node.toElement().hasAttribute(attr_name.trimmed()))  return defValue;
    return node.toElement().attribute(attr_name.trimmed());
}
void LStaticXML::createDomHeader(QDomDocument &dom)
{
    dom.appendChild(dom.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
}
QString LStaticXML::getDomRootNode(const QString &fname, QDomNode &r_node, QString need_root_node_name)
{
    r_node.clear();
    if (fname.trimmed().isEmpty()) return QString("Input file is Empty");
    if (!fname.contains(".xml")) return QString("Input file [%1] is not XML-file").arg(fname);

    QFile f(fname.trimmed());
    if (!f.exists()) return QString("Input file [%1] not found").arg(fname);

    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        if (f.isOpen()) f.close();
        return QString("Can't load to DomDocument input file [%1]").arg(fname);
    }
    f.close();

    if (!need_root_node_name.isEmpty())
    {
        r_node = dom.namedItem(need_root_node_name);
        if (r_node.isNull()) return QString("Not found root_node by name <%1>").arg(need_root_node_name);
    }
    else
    {
        QDomNodeList childs = dom.childNodes();
        if (childs.isEmpty()) return QString("Childs list is empty, input file [%1]").arg(fname);
        for (int i=0; i< childs.count(); i++)
        {
            if (childs.at(i).isElement())
            {
                r_node = childs.at(i).cloneNode();
                break;
            }
        }
    }

    if (r_node.isNull()) return QString("In Childs list not found element node, input file [%1]").arg(fname);
    return QString();
}




#include "lstaticxml.h"

#include <QDebug>


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




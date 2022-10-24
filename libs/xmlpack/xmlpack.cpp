#include "xmlpack.h"
#include "xmlpacktype.h"
#include "lfile.h"
#include "lstatic.h"


#include <QTimer>
#include <QDebug>
#include <QColor>
#include <QDomDocument>
#include <QDomElement>


//LXMLPackObj
LXMLPackObj::LXMLPackObj(const QString &fname, QObject *parent)
    :LSimpleObject(parent),
    m_rootNode(NULL),
    m_fileName(fname.trimmed())
{



}
void LXMLPackObj::tryLoadPacket(bool &ok)
{
    ok = false;
    if (m_rootNode)
    {
        emit signalError(QString("Packet allready loaded: [%1]").arg(m_fileName));
        return;
    }
    if (m_fileName.isEmpty())
    {
        emit signalError(QString("Packet file is empty"));
        return;
    }
    if (!LFile::fileExists(m_fileName))
    {
        emit signalError(QString("Packet file [%1] not found").arg(m_fileName));
        return;
    }
    if (m_fileName.length() < 5 || m_fileName.right(4) != ".xml")
    {
        emit signalError(QString("Invalid file type [%1], must be '.xml'").arg(m_fileName));
        return;
    }

    //load file to QDomDocument
    QDomDocument dom;
    if (!dom.setContent(m_fileName))
    {
        emit signalError(QString("Error load XML content to DOM from: [%1]'").arg(m_fileName));
        return;
    }
    loadDom(dom, ok);
}
void LXMLPackObj::loadDom(const QDomDocument &dom, bool &ok)
{
    ok = false;
    QDomNode root_node = dom.namedItem(XMLPackStatic::rootNodeName());
    if (root_node.isNull())
    {
        emit signalError(QString("Error load XML from [%1], not found root node [%2]").arg(m_fileName).arg(XMLPackStatic::rootNodeName()));
        return;
    }
    if (!root_node.attributes().contains(XMLPackStatic::cationAttrName()))
    {
        emit signalError(QString("Error load XML from [%1], not found [%2] attribute of root node").arg(m_fileName).arg(XMLPackStatic::cationAttrName()));
        return;
    }

    QString err;
    m_rootNode = new LXMLPackElement();
    m_rootNode->loadNode(root_node, err);
    if (!err.isEmpty())
    {
        delete m_rootNode;
        m_rootNode = NULL;
        qWarning("invalid loading root node");
        emit signalError(QString("Error load XML from [%1]: %2").arg(m_fileName).arg(err));
        return;
    }
}





//LXMLPackElement
LXMLPackElement::LXMLPackElement()
    :m_dataType(petSection),
    m_arrSize(1),
    m_parentNode(NULL)
{

}
LXMLPackElement::LXMLPackElement(LXMLPackElement *parent_node)
    :m_dataType(petSection),
    m_arrSize(1),
    m_parentNode(parent_node)
{

}
bool LXMLPackElement::isNode() const
{
    return (m_dataType == petSection);
}
bool LXMLPackElement::isRoot() const
{
    return (m_parentNode == NULL);
}
void LXMLPackElement::reset()
{
    qDeleteAll(m_childs);
    m_childs.clear();
}
void LXMLPackElement::loadNode(const QDomNode &node, QString &err)
{
    bool ok;
    QString attr;
    err.clear();

    //read caption
    setCation(LStatic::getStringAttrValue(XMLPackStatic::cationAttrName(), node));

    //read datatype
    if (node.attributes().contains(XMLPackStatic::dataTypeAttrName()))
    {
        attr = LStatic::getStringAttrValue(XMLPackStatic::dataTypeAttrName(), node);
        m_dataType = XMLPackStatic::typeByXmlAttr(attr);
    }
    else m_dataType = petSection;

    if (invalid())
    {
        err = QString("loading node [%1] - invalid datatype %2").arg(node.nodeName()).arg(attr);
        return;
    }
    if (isRoot()) return;

    //read arr_size
    if (node.attributes().contains(XMLPackStatic::arrSizeAttrName()))
    {
        int a = LStatic::getIntAttrValue(XMLPackStatic::arrSizeAttrName(), node);
        if (a < 0)
        {
            err = QString("loading node [%1] - invalid arr_size %2").arg(node.nodeName()).arg(attr);
            return;
        }
        m_arrSize = quint16(a);
    }

    //load value and load childs
    loadValueAttrs(node);
    loadChilds(node);

}
void LXMLPackElement::loadValueAttrs(const QDomNode &node)
{
    m_value.reset();
    if (invalid() || isOff() || isNode()) return;

    m_value.isDouble = XMLPackStatic::isDoubleType(m_dataType);

    if (node.attributes().contains(XMLPackStatic::defValueAttrName()))
    {
        if (m_value.isDouble)
        {
            m_value.d_value = LStatic::getDoubleAttrValue(XMLPackStatic::defValueAttrName(), node, -1);
        }
        else
        {
            m_value.i_value = LStatic::getIntAttrValue(XMLPackStatic::defValueAttrName(), node, -1);
        }
    }

    if (node.attributes().contains(XMLPackStatic::errValueAttrName()))
    {
        m_value.rand_deviation = LStatic::getDoubleAttrValue(XMLPackStatic::errValueAttrName(), node, 0);
    }
}
void LXMLPackElement::loadChilds(const QDomNode &node)
{
    if (invalid() || isOff() || !isNode()) return;

    QString err;
    QDomNode child = node.firstChild();
    while (!child.isNull())
    {
        LXMLPackElement *el = new LXMLPackElement(this);

        el->loadNode(child, err);
        if (!err.isEmpty())
        {
            delete el;
            el = NULL;
            qWarning("");
            qWarning()<<QString("LXMLPackElement: [%1], invalid loading child node [%2]").arg(caption()).arg(child.nodeName());
            qWarning()<<QString("error: %1").arg(err);
        }
        else m_childs.append(el);

        child = child.nextSibling();
    }
}
bool LXMLPackElement::invalid() const
{
    if (m_dataType == petInvalid) return true;
    return false;
}



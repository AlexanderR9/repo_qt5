#include "xmlpackelement.h"
#include "xmlpacktype.h"
//#include "lfile.h"
#include "lstatic.h"


#include <QDebug>
//#include <QDomDocument>
#include <QDomElement>

/*
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
    QFile f(m_fileName);
    if (!dom.setContent(&f))
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

    retransformArrNodes();
    recalcOffset();
    ok = true;
}
void LXMLPackObj::recalcOffset()
{
    quint32 cur_offset = 0;
    if (m_rootNode)
        m_rootNode->calcOffset(m_rootNode, cur_offset);
}
void LXMLPackObj::retransformArrNodes()
{
    if (m_rootNode)
        m_rootNode->retransformArrChilds();
}


*/

//LXMLPackElement
LXMLPackElement::LXMLPackElement()
    :m_parentNode(NULL)
{
    reset();
}
LXMLPackElement::LXMLPackElement(LXMLPackElement *parent_node)
    :m_parentNode(parent_node)
{
    reset();
}
void LXMLPackElement::reset()
{
    m_dataType = petSection;
    m_arrSize = 1;
    m_offset = 0;
    m_value.reset();
}
bool LXMLPackElement::isNode() const
{
    return (m_dataType == petSection);
}
bool LXMLPackElement::isRoot() const
{
    return (m_parentNode == NULL);
}
bool LXMLPackElement::isData() const
{
    if (isNode() || invalid() || isOff()) return false;
    return true;
}
void LXMLPackElement::destroyChilds()
{
    qDeleteAll(m_childs);
    m_childs.clear();
}
void LXMLPackElement::loadNode(const QDomNode &node, QString &err)
{
    //qDebug()<<QString("LXMLPackElement::loadNode  %1").arg(node.nodeName());
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
    //if (isRoot()) return;

    //calcOffset();

    //read arr_size
    m_arrSize = 1;
    if (node.attributes().contains(XMLPackStatic::arrSizeAttrName()))
    {
        int a = LStatic::getIntAttrValue(XMLPackStatic::arrSizeAttrName(), node);
        if (a < 0)
        {
            err = QString("loading node [%1] - invalid arr_size %2").arg(node.nodeName()).arg(attr);
            return;
        }
        m_arrSize = quint16(a);
        qDebug()<<QString("find arr[%1] for %2").arg(arrSize()).arg(caption());
    }

    //load value and load childs
    loadValueAttrs(node);
    loadChilds(node);

    //qDebug()<<QString("node: %1,  childs %2").arg(caption()).arg(childsCount());
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
        else if (el->isOff())
        {
            delete el;
            el = NULL;
            qWarning("");
            qWarning()<<QString("LXMLPackElement: [%1], child node [%2] is OFF").arg(caption()).arg(child.nodeName());
        }
        else
        {
            m_childs.append(el);
        }

        child = child.nextSibling();
    }
}
bool LXMLPackElement::invalid() const
{
    if (m_dataType == petInvalid) return true;
    if (isOff()) return true;
    return false;
}
quint32 LXMLPackElement::size() const
{
    if (invalid()) return 0;
    if (isData()) return XMLPackStatic::sizeOf(dataType());
    return sectionSize(this);
}
quint32 LXMLPackElement::offset() const
{
    return m_offset;
}
quint32 LXMLPackElement::sectionSize(const LXMLPackElement*) const
{
    quint32 sec_size = 0;
    int n = childsCount();
    for (int i=0; i<n; i++)
        sec_size += childAt(i)->size();
    return sec_size;
}
void LXMLPackElement::calcOffset(LXMLPackElement*, quint32 &cur_offset)
{
    //qDebug()<<QString("LXMLPackElement::calcOffset  %1").arg(caption());
    m_offset = cur_offset;
    if (invalid()) return;
    if (!hasChilds())
    {
        cur_offset += size();
        return;
    }

    int n = childsCount();
    for (int i=0; i<n; i++)
    {
        m_childs[i]->calcOffset(this, cur_offset);
    }
}
LXMLPackElement* LXMLPackElement::clone(LXMLPackElement *parent) const
{
    LXMLPackElement *el = new LXMLPackElement(parent);
    el->setCation(caption());
    el->setDataType(dataType());

    if (isData()) el->setValueInfo(m_value);
    if (!hasChilds()) return el;

    //clone childs
    for (int i=0; i<childsCount(); i++)
    {
        LXMLPackElement *el_child = childAt(i)->clone(el);
        if (el_child) el->appendChild(el_child);
    }
    return el;
}
void LXMLPackElement::appendChild(LXMLPackElement *child)
{
    if (child) m_childs.append(child);
}
void LXMLPackElement::retransformArrChilds()
{
    if (!hasChilds()) return;

    for (int i=0; i<childsCount(); i++)
    {
        m_childs[i]->retransformArrChilds();
        if (childAt(i)->isArr()) retransformArrChild(i);
    }
}
void LXMLPackElement::retransformArrChild(int i)
{
    if (childAt(i)->invalid()) return;

    LXMLPackElement *el = m_childs.takeAt(i);

    LXMLPackElement *el_header = new LXMLPackElement(this);
    el_header->setCation(el->caption());
    el_header->setDataType(petSection);
    el_header->setArrSize(el->arrSize());
    m_childs.insert(i, el_header);


    for (int i=0; i<el->arrSize(); i++)
    {
        LXMLPackElement *el_arr = el->clone(el_header);
        el_arr->setCation(QString("%1 (%2)").arg(el->caption()).arg(i+1));
        el_header->appendChild(el_arr);
    }
}





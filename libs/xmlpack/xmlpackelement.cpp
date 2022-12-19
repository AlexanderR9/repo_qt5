#include "xmlpackelement.h"
#include "xmlpacktype.h"
#include "lstaticxml.h"
#include "ltime.h"


#include <QDebug>
#include <QDomElement>


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
bool LXMLPackElement::isTime() const
{
    return (m_dataType == petTimeSpec);
}
bool LXMLPackElement::isRoot() const
{
    return (m_parentNode == NULL);
}
bool LXMLPackElement::isData() const
{
    if (isNode() || invalid() || isOff() || isTime()) return false;
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
    setCation(LStaticXML::getStringAttrValue(XMLPackStatic::cationAttrName(), node));

    //read kks
    m_kks = LStaticXML::getStringAttrValue("kks", node);

    //read datatype
    if (node.attributes().contains(XMLPackStatic::dataTypeAttrName()))
    {
        attr = LStaticXML::getStringAttrValue(XMLPackStatic::dataTypeAttrName(), node);
        m_dataType = XMLPackStatic::typeByXmlAttr(attr);
    }
    else m_dataType = petSection;

    if (invalid())
    {
        err = QString("loading node [%1] - invalid datatype %2").arg(node.nodeName()).arg(attr);
        return;
    }

    if (isTime())
    {
        transformTimeSpec();
        return;
    }

    //read arr_size
    m_arrSize = 1;
    if (node.attributes().contains(XMLPackStatic::arrSizeAttrName()))
    {
        int a = LStaticXML::getIntAttrValue(XMLPackStatic::arrSizeAttrName(), node);
        if (a < 0)
        {
            err = QString("loading node [%1] - invalid arr_size %2").arg(node.nodeName()).arg(attr);
            return;
        }
        m_arrSize = quint16(a);
        //qDebug()<<QString("find arr[%1] for %2").arg(arrSize()).arg(caption());
    }

    //load value and load childs
    loadValueAttrs(node);
    loadChilds(node);
}
void LXMLPackElement::loadValueAttrs(const QDomNode &node)
{
    m_value.reset();
    if (invalid() || isOff() || isNode()) return;

    if (m_dataType == petDiscrete) m_value.isDiscrete = true;
    else
    {
        m_value.isDouble = XMLPackStatic::isDoubleType(m_dataType);
        m_value.isUnsigned = XMLPackStatic::isUnsignedType(m_dataType);
    }

    if (node.attributes().contains(XMLPackStatic::defValueAttrName()))
    {
        if (m_value.isDouble)
        {
            m_value.d_value = LStaticXML::getDoubleAttrValue(XMLPackStatic::defValueAttrName(), node, -1);
        }
        else
        {
            m_value.i_value = LStaticXML::getIntAttrValue(XMLPackStatic::defValueAttrName(), node, -1);
        }
    }

    if (node.attributes().contains(XMLPackStatic::errValueAttrName()))
    {
        m_value.rand_deviation = LStaticXML::getDoubleAttrValue(XMLPackStatic::errValueAttrName(), node, 0);
    }
}
void LXMLPackElement::loadChilds(const QDomNode &node)
{
    if (invalid() || isOff() || !isNode()) return;

    QString err;
    QDomNode child = node.firstChild();
    while (!child.isNull())
    {
        if (child.isComment())
        {
            child = child.nextSibling();
            continue;
        }

        LXMLPackElement *el = new LXMLPackElement(this);
        el->loadNode(child, err);
        if (!err.isEmpty())
        {
            delete el;
            el = NULL;
            qWarning()<<QString("LXMLPackElement: [%1], invalid loading child node [%2]").arg(caption()).arg(child.nodeName());
            qWarning()<<QString("error: %1").arg(err);
        }
        else if (el->isOff())
        {
            delete el;
            el = NULL;
            qWarning()<<QString("LXMLPackElement: [%1], child node [%2] is OFF").arg(caption()).arg(child.nodeName());
        }
        else m_childs.append(el);

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


void LXMLPackElement::nextRandValue()
{
    if (invalid()) return;

    if (isData()) m_value.recalcNext();
    else if (isTime()) nextTimeValue();
    else
    {
        int n = childsCount();
        for (int i=0; i<n; i++)
            m_childs[i]->nextRandValue();
    }
}
void LXMLPackElement::nextTimeValue()
{
    timespec tm;
    LTime::getTimeSpecCPP(tm);
    if (hasChilds())
    {
        bool ok;
        childAtVar(0)->setNewValue(QString::number(tm.tv_sec), ok);
        childAtVar(1)->setNewValue(QString::number(tm.tv_nsec), ok);
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
void LXMLPackElement::transformTimeSpec()
{
    if (invalid() || !isTime()) return;

    LXMLPackElement *el_sec = new LXMLPackElement(this);
    el_sec->setCation("seconds");
    el_sec->setDataType(petInt64);
    appendChild(el_sec);

    LXMLPackElement *el_nsec = new LXMLPackElement(this);
    el_nsec->setCation("nano seconds");
    el_nsec->setDataType(petInt64);
    appendChild(el_nsec);
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
void LXMLPackElement::writeToStream(QDataStream &stream)
{
    if (invalid()) return;

    //if (isData()) writeValueToStream(stream);
    if (isData()) m_value.writeToStream(dataType(), stream);
    else if (hasChilds())
    {
        for (int i=0; i<childsCount(); i++)
            m_childs[i]->writeToStream(stream);
    }
}
/*
void LXMLPackElement::writeValueToStream(QDataStream &stream) //записать в поток
{
    switch (dataType())
    {
        case petInt8:       {stream << qint8(m_value.i_value); break;}
        case petInt16:      {stream << qint16(m_value.i_value); break;}
        case petInt32:      {stream << qint32(m_value.i_value); break;}
        case petInt64:      {stream << qint64(m_value.i_value); break;}

        case petDiscrete:
        case petUint8:      {stream << quint8(m_value.i_value); break;}
        case petUint16:     {stream << quint16(m_value.i_value); break;}
        case petUint32:     {stream << quint32(m_value.i_value); break;}
        case petUint64:     {stream << quint64(m_value.i_value); break;}

        case petFloat:      {stream << float(m_value.d_value); break;}
        case petDouble:     {stream << m_value.d_value; break;}
        default: break;
    }
}
*/
void LXMLPackElement::readFromStream(QDataStream &stream)
{
    if (invalid()) return;

    //if (isData()) readValueFromStream(stream);
    if (isData()) m_value.readFromStream(dataType(), stream);
    else if (hasChilds())
    {
        for (int i=0; i<childsCount(); i++)
            m_childs[i]->readFromStream(stream);
    }
}
/*
void LXMLPackElement::readValueFromStream(QDataStream &stream) //считать из потока
{
    switch (dataType())
    {
        case petInt8:       {qint8 v=0; stream >> v; m_value.i_value = v; break;}
        case petInt16:      {qint16 v=0; stream >> v; m_value.i_value = v; break;}
        case petInt32:      {qint32 v=0; stream >> v; m_value.i_value = v; break;}
        case petInt64:      {stream >> m_value.i_value; break;}

        case petUint8:      {quint8 v=0; stream >> v; m_value.i_value = v; break;}
        case petUint16:     {quint16 v=0; stream >> v; m_value.i_value = v; break;}
        case petUint32:     {quint32 v=0; stream >> v; m_value.i_value = v; break;}
        case petUint64:     {quint64 v=0; stream >> v; m_value.i_value = v; break;}

        case petDiscrete:
        {
            quint8 v=0;
            stream >> v;
            if (v > 1) v = 0;
            m_value.i_value = v;
            break;
        }

        case petFloat:      {float v=0; stream >> v; m_value.d_value = v; break;}
        case petDouble:     {stream >> m_value.d_value; break;}
        default: break;
    }
}
*/
QString LXMLPackElement::strValue(quint8 precision) const
{
    if (invalid()) return ("-9999");
    if (!isData()) return ("-1");
    return m_value.strValue(precision);
}
QString LXMLPackElement::strValueDeviation() const
{
    if (invalid() || !isData()) return ("0");
    return m_value.strDeviation();
}
void LXMLPackElement::setNewValue(const QString &s, bool &ok)
{
    ok = false;
    if (invalid() || !isData()) return;

    QString err;
    m_value.setUserValue(s, err);
    if (!err.isEmpty()) qWarning() << err;
    else {m_value.conversionIntType(dataType()); ok=true;}

    /*
    if (m_value.isDouble)
    {
        double dv = s.toDouble(&ok);
        if (!ok)
        {
            qWarning()<<QString("LXMLPackElement::setNewValue (double) WARNING - node %1,  invalid double value: %2").arg(caption()).arg(s);
        }
        else m_value.d_value = dv;
    }
    else
    {
        qint64 iv = 0;
        if (XMLPackStatic::isUnsignedType(dataType()))
        {
            iv = s.toUInt(&ok);
            if (!ok)
            {
                qWarning()<<QString("LXMLPackElement::setNewValue (unit) WARNING - node %1,  invalid unsigned integer value: %2").arg(caption()).arg(s);
                return;
            }
        }
        else
        {
            iv = s.toInt(&ok);
            if (!ok)
            {
                qWarning()<<QString("LXMLPackElement::setNewValue (int) WARNING - node %1,  invalid integer value: %2").arg(caption()).arg(s);
                return;
            }
        }

        setIntValue(iv);
    }
    */
}
void LXMLPackElement::setNewValueDeviation(const QString &s, bool &ok)
{
    ok = false;
    if (invalid() || !isData()) return;

    QString err;
    m_value.setUserDeviation(s, err);
    if (!err.isEmpty()) qWarning() << err;
    else ok=true;


    /*
    double v = s.toDouble(&ok);
    if (!ok || v < 0 || v > 90)
    {
        ok = false;
        qWarning()<<QString("LXMLPackElement::setNewValueDeviation WARNING - node %1,  invalid value for update deviation: %2").arg(caption()).arg(s);
    }
    else
    {
        ok = true;
        m_value.rand_deviation = v;
    }
    */
}

/*
void LXMLPackElement::setIntValue(qint64 iv)
{
    switch (dataType())
    {
        case petInt8:       {qint8 v = qint8(iv);       m_value.i_value = v; break;}
        case petInt16:      {qint16 v = qint16(iv);     m_value.i_value = v; break;}
        case petInt32:      {qint32 v = qint32(iv);     m_value.i_value = v; break;}
        case petUint8:      {quint8 v = quint8(iv);     m_value.i_value = v; break;}
        case petUint16:     {quint16 v = quint16(iv);   m_value.i_value = v; break;}
        case petUint32:     {quint32 v = quint32(iv);   m_value.i_value = v; break;}

        case petDiscrete:
        {
            quint8 v = quint8(iv);
            if (v > 1) v = 0;
            m_value.i_value = v;
            break;
        }


        case petUint64:
        case petInt64:      {m_value.i_value = iv; break;}

        default: break;
    }
}
*/


//by path funcs
LXMLPackElement* LXMLPackElement::nodeByPath(const QList<quint16> &levels)
{
    if (levels.isEmpty()) {qWarning()<<QString("LXMLPackElement::nodeByPath WARNING level path is empty"); return NULL;}
    LXMLPackElement *node = this;
    foreach (quint16 l, levels)
    {
        if (l >= node->m_childs.count()) {qWarning()<<QString("LXMLPackElement::nodeByPath WARNING invalid level %1").arg(l); return NULL;}
        node = node->m_childs.at(l);
    }
    return node;
}
void LXMLPackElement::setIntValueByPath(const QList<quint16> &levels, qint64 v, bool &ok)
{
    ok = false;
    LXMLPackElement *node = nodeByPath(levels);
    if (!node) return;

    if (!XMLPackStatic::isIntegerType(node->dataType()))
    {
        qWarning()<<QString("LXMLPackElement::setIntValueByPath WARNING node [%1] not integer").arg(node->caption());
        return;
    }

    //node->setIntValue(v);
    //ok = true;
    node->setNewValue(QString::number(v), ok);
}
void LXMLPackElement::setDoubleValueByPath(const QList<quint16> &levels, double v, bool &ok)
{
    ok = false;
    LXMLPackElement *node = nodeByPath(levels);
    if (!node) return;

    if (!XMLPackStatic::isDoubleType(node->dataType()))
    {
        qWarning()<<QString("LXMLPackElement::setDoubleValueByPath WARNING node [%1] not double").arg(node->caption());
        return;
    }

    node->m_value.d_value = v;
    ok = true;
}
qint64 LXMLPackElement::getIntValueByPath(const QList<quint16> &levels, bool &ok)
{
    ok = false;
    LXMLPackElement *node = nodeByPath(levels);
    if (!node) return -1;

    if (!XMLPackStatic::isIntegerType(node->dataType()))
    {
        qWarning()<<QString("LXMLPackElement::getIntValueByPath WARNING node [%1] not double").arg(node->caption());
        return -1;
    }

    ok = true;
    return node->getValue().i_value;
}
double LXMLPackElement::getDoubleValueByPath(const QList<quint16> &levels, bool &ok)
{
    ok = false;
    LXMLPackElement *node = nodeByPath(levels);
    if (!node) return -1;

    if (!XMLPackStatic::isDoubleType(node->dataType()))
    {
        qWarning()<<QString("LXMLPackElement::getDoubleValueByPath WARNING node [%1] not double").arg(node->caption());
        return -1;
    }

    ok = true;
    return node->getValue().d_value;
}



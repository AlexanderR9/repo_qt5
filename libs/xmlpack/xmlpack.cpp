#include "xmlpack.h"
#include "xmlpackelement.h"
#include "xmlpacktype.h"
#include "lfile.h"
#include "lstaticxml.h"
#include "lstatic.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDataStream>
#include <QByteArray>


//LXMLPackObj
LXMLPackObj::LXMLPackObj(const QString &fname, QObject *parent)
    :LSimpleObject(parent),
    m_rootNode(NULL),
    m_fileName(fname.trimmed()),
    m_byteOrder(QDataStream::LittleEndian),
    m_kksUsed(false)
{



}
void LXMLPackObj::setByteOrder(int bo)
{
    if (bo == QDataStream::LittleEndian) m_byteOrder = bo;
    else if (bo == QDataStream::BigEndian) m_byteOrder = bo;
}
QString LXMLPackObj::caption() const
{
    if (m_rootNode) return m_rootNode->caption();
    return "??";
}
void LXMLPackObj::nextRandValues()
{
    if (m_rootNode) m_rootNode->nextRandValue();
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

    m_kksUsed = (LStaticXML::getStringAttrValue("use_kks", root_node).trimmed() == QString("true"));

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
quint32 LXMLPackObj::size() const
{
    if (invalid()) return 0;
    return m_rootNode->size();
}
void LXMLPackObj::destroyObj()
{
    if (m_rootNode)
    {
        delete m_rootNode;
        m_rootNode = NULL;
    }
}
void LXMLPackObj::toByteArray(QByteArray &ba, bool singleFloatPrecision)
{    
    ba.clear();
    if (invalid())
    {
        emit signalError("Packet object is invalid");
        return;
    }

    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::ByteOrder(m_byteOrder));
    if (singleFloatPrecision)
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    m_rootNode->writeToStream(stream);
}
void LXMLPackObj::fromByteArray(const QByteArray &ba, bool &ok, bool singleFloatPrecision)
{
    ok = false;
    if (invalid())
    {
        emit signalError("Packet object is invalid");
        return;
    }
    if (ba.isEmpty())
    {
        emit signalError("ByteArray is empty");
        return;
    }
    if (ba.size() != int(size()))
    {
        emit signalError(QString("ByteArray size(%1) != packet size(%2)").arg(ba.size()).arg(size()));
        return;
    }

    QDataStream stream(ba);
    stream.setByteOrder(QDataStream::ByteOrder(m_byteOrder));
    if (singleFloatPrecision)
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    m_rootNode->readFromStream(stream);
    ok =  true;
}


void LXMLPackObj::setIntValueByPath(QString path, qint64 v, bool &ok)
{
    ok = false;
    if (invalid()) {qWarning()<<QString("LXMLPackObj::setIntValueByPath WARNING packet is invalid"); return;}

    QStringList list = LStatic::trimSplitList(path, "/");
    if (list.count() < 2) {qWarning()<<QString("LXMLPackObj::setIntValueByPath WARNING invalid path [%1]").arg(path); return;}
    if (list.first() != "packet")  {qWarning()<<QString("LXMLPackObj::setIntValueByPath WARNING invalid path [%1], first element != packet").arg(path); return;}

    QList<quint8> levels;
    for (int i=1; i<list.count(); i++)
    {
        //qDebug()<<list.at(i);
        quint8 level = list.at(i).toUInt(&ok);
        //qDebug()<<QString::number(level)<<"   "<<QString(!ok?"false":"true");
        if (!ok) {qWarning()<<QString("LXMLPackObj::setIntValueByPath WARNING invalid path [%1], %2 element(%3) not uint").arg(path).arg(i).arg(list.at(i)); return;}
        levels.append(level);
    }
    m_rootNode->setIntValueByPath(levels, v, ok);
}
void LXMLPackObj::setDoubleValueByPath(QString path, double v, bool &ok)
{
    ok = false;
    if (invalid()) {qWarning()<<QString("LXMLPackObj::setDoubleValueByPath WARNING packet is invalid"); return;}

    QStringList list = LStatic::trimSplitList(path, "/");
    if (list.count() < 2) {qWarning()<<QString("LXMLPackObj::setDoubleValueByPath WARNING invalid path [%1]").arg(path); return;}
    if (list.first() != "packet")  {qWarning()<<QString("LXMLPackObj::setDoubleValueByPath WARNING invalid path [%1], first element != packet").arg(path); return;}

    QList<quint8> levels;
    for (int i=1; i<list.count(); i++)
    {
        quint8 level = list.at(i).toUInt(&ok);
        if (!ok) {qWarning()<<QString("LXMLPackObj::setDoubleValueByPath WARNING invalid path [%1], %2 element(%3) not uint").arg(path).arg(i).arg(list.at(i)); return;}
        levels.append(level);
    }
    m_rootNode->setDoubleValueByPath(levels, v, ok);
}



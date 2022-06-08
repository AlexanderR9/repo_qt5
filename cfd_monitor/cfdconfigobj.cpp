#include "cfdconfigobj.h"
#include "lstatic.h"

#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include <QDebug>



CFDConfigObject::CFDConfigObject(const QString &fname, QObject *parent)
    :LSimpleObject(parent),
    m_configFile(fname.trimmed()),
    m_curCFDIndex(-1)
{
    reset();
}
QStringList CFDConfigObject::getSources() const
{
    QStringList list;
    for (int i=0; i<m_sources.count(); i++)
    {
        int pos = m_sources.at(i).url.indexOf("//");
        if (pos > 0)
        {
            pos = m_sources.at(i).url.indexOf("/", pos+2);
            if (pos > 0) list.append(m_sources.at(i).url.left(pos));
        }
    }
    return list;
}
void CFDConfigObject::getNextTicker(QString &s)
{
    s.clear();
    if (m_cfdList.isEmpty()) return;

    m_curCFDIndex++;
    if (m_curCFDIndex >= m_cfdList.count()) m_curCFDIndex = 0;
    s = m_cfdList.at(m_curCFDIndex).ticker;
}
QStringList CFDConfigObject::getCFDObjectData(int i) const
{
    QStringList list;
    if (i < 0 || i >= m_cfdList.count())
    {
        qWarning()<<QString("CFDConfigObject::getCFDObjectData - INVALID cfd index: %1").arg(i);
        return list;
    }

    list.append(QString::number(i+1));
    list.append(m_cfdList.at(i).ticker);
    list.append(m_cfdList.at(i).name);
    list.append(m_cfdList.at(i).requestUrl(sourceByID(m_cfdList.at(i).source_id)));
    return list;
}
void CFDConfigObject::tryLoadConfig()
{
    reset();

    QString msg = QString("config_file = [%1]").arg(m_configFile);
    emit signalMsg(msg);

    if (m_configFile.trimmed().isEmpty())
    {
        msg = QString("config filename is empty ");
        emit signalError(msg);
        return;
    }
    QFile f(m_configFile.trimmed());
    if (!f.exists())
    {
        msg = QString("config file not found");
        emit signalError(msg);
        return;
    }
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        msg = QString("config file can't load to DomDocument");
        emit signalError(msg);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    //check main nodes
    QString root_node_name("config");
    QDomNode root_node = dom.namedItem(root_node_name);
    if (root_node.isNull())
    {
        msg = QString("invalid struct XML document, root node <%1> not found.").arg(root_node_name);
        emit signalError(msg);
        return;
    }

    QString cfdlist_node_name("cfd_list");
    QDomNode cfdlist_node = root_node.namedItem(cfdlist_node_name);
    if (cfdlist_node.isNull())
    {
        msg = QString("invalid struct XML document, node <%1> not found.").arg(cfdlist_node_name);
        emit signalError(msg);
        return;
    }

    QString sources_node_name("sources");
    QDomNode sources_node = root_node.namedItem(sources_node_name);
    if (sources_node.isNull())
    {
        msg = QString("invalid struct XML document, node <%1> not found.").arg(sources_node_name);
        emit signalError(msg);
        return;
    }

    loadSources(sources_node);
    loadCFDList(cfdlist_node);

    sendConfigInfo();
    emit signalMsg(QString("Finished OK!"));

}
void CFDConfigObject::loadSources(const QDomNode &node)
{
    if (node.isNull()) return;

    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "url")
        {
            CFDDataSource source;
            source.id = LStatic::getIntAttrValue("id", child_node, -1);
            source.url = LStatic::getStringAttrValue("value", child_node, QString("?")).trimmed();
            if (source.invalid())
            {
                qWarning()<<QString("Invalid readed source element from config: %1").arg(source.toStr());
            }
            else m_sources.append(source);
        }
        child_node = child_node.nextSibling();
    }
}
void CFDConfigObject::loadCFDList(const QDomNode &node)
{
    if (node.isNull()) return;

    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "cfd")
        {
            CFDObj cfd;
            cfd.ticker = LStatic::getStringAttrValue("ticker", child_node, QString()).trimmed().toUpper();
            cfd.name = LStatic::getStringAttrValue("name", child_node, QString()).trimmed();
            cfd.url_text = LStatic::getStringAttrValue("url", child_node, QString()).trimmed();
            cfd.source_id = LStatic::getIntAttrValue("source", child_node, 1);
            if (cfd.invalid())
            {
                qWarning()<<QString("Invalid readed CFD element from config: %1").arg(cfd.toStr());
            }
            else if (containsTicker(cfd.ticker))
            {
                qWarning()<<QString("WARNING: ticker %1 allready exist").arg(cfd.ticker);
            }
            else m_cfdList.append(cfd);
        }
        child_node = child_node.nextSibling();
    }
}
void CFDConfigObject::reset()
{
    m_sources.clear();
    m_cfdList.clear();
}
QString CFDConfigObject::sourceByID(int id) const
{
    for (int i=0; i<m_sources.count(); i++)
        if (m_sources.at(i).id == id) return m_sources.at(i).url;
    return QString();
}
bool CFDConfigObject::containsTicker(QString s) const
{
    s = s.trimmed().toUpper();
    for (int i=0; i<m_cfdList.count(); i++)
        if (m_cfdList.at(i).ticker == s) return true;
    return false;
}
void CFDConfigObject::sendConfigInfo()
{
    emit signalMsg(QString("readed %1 URL sourses:").arg(m_sources.count()));
    //for (int i=0; i<m_sources.count(); i++)
        //emit signalMsg(QString("   %1. %2").arg(i+1).arg(m_sources.at(i).toStr()));

    //emit signalMsg(QString("-----------------------------"));

    emit signalMsg(QString("readed %1 CFD objects:").arg(m_cfdList.count()));
    //for (int i=0; i<m_cfdList.count(); i++)
        //emit signalMsg(QString("   %1. %2").arg(i+1).arg(m_cfdList.at(i).toStr()));
}


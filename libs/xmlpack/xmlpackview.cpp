#include "xmlpackview.h"
#include "xmlpack.h"
#include "xmlpackelement.h"
#include "xmlpacktype.h"

#include <QDebug>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>


// LXMLPackView
LXMLPackView::LXMLPackView(const QString &title, QWidget *parent)
    :QGroupBox(title, parent),
      m_view(NULL),
      m_rootItem(NULL),
      m_packet(NULL),
      m_readOnly(true)
{
    setObjectName("xml_packet_view");
    initWidget();

}
void LXMLPackView::initWidget()
{
    if (layout()) delete layout();
    QVBoxLayout *v_lay = new QVBoxLayout(0);
    setLayout(v_lay);

    m_view = new QTreeWidget(this);
    v_lay->addWidget(m_view);

    m_view->setColumnCount(4);
    m_view->header()->setDefaultAlignment(Qt::AlignCenter);
    QStringList headers;
    headers << "Cation" << "Type" << "Size, b" << "Offset" << "Childs";
    m_view->setHeaderLabels(headers);

    m_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);

}
void LXMLPackView::setPacket(LXMLPackObj *p)
{
    if (!p)
    {
        qWarning("LXMLPackView::setPacket - WARNING packet object is NULL!");
        return;
    }

    m_packet = p;
    reloadView();
}
void LXMLPackView::reloadView()
{
    resetView();
    if (!m_packet) return;
    if (m_packet->invalid()) return;

    m_rootItem = new LXMLPackViewItem(m_packet->rootElement());
    m_view->addTopLevelItem(m_rootItem);
    m_view->expandAll();
    resizeColumns();
}
void LXMLPackView::resizeColumns()
{
    int n = m_view->columnCount();
    for (int i=0; i<n; i++)
        m_view->resizeColumnToContents(i);
}
void LXMLPackView::resetView()
{
    m_view->clear();
    if (m_rootItem)
    {
        delete m_rootItem;
        m_rootItem = NULL;
    }
}


//LXMLPackViewItem
LXMLPackViewItem::LXMLPackViewItem(const LXMLPackElement *node, QTreeWidgetItem *parent)
    :QTreeWidgetItem(parent),
      m_node(node)
{
    updateColumnsText();
    updateColumnsColor();

    if (m_node->hasChilds())
        loadNodeChilds();
}
void LXMLPackViewItem::loadNodeChilds()
{
    int n = m_node->childsCount();
    for (int i=0; i<n; i++)
    {
        const LXMLPackElement *child_node = m_node->childAt(i);
        if (!child_node) continue;
        if (child_node->invalid()) continue;

        LXMLPackViewItem *child_item = new LXMLPackViewItem(child_node, this);
        Q_UNUSED(child_item);
    }
}
void LXMLPackViewItem::updateColumnsText()
{
    setText(0, m_node->caption());
    setText(1, XMLPackStatic::xmlAttrName(m_node->dataType()));
    setText(2, QString::number(m_node->size()));
    setText(3, QString::number(m_node->offset()));
    setText(4, QString::number(m_node->childsCount()));
}
void LXMLPackViewItem::updateColumnsColor()
{
    if (m_node->isRoot())
    {
        for (int i=0; i<columnCount(); i++)
        {
            setTextColor(i, Qt::blue);
        }
        setText(1, "root");
    }
    if (m_node->isArr())
    {
        for (int i=0; i<columnCount(); i++) setTextColor(i, QColor(220, 120, 0));
        setText(1, QString("arr[%1]").arg(m_node->arrSize()));
    }


}




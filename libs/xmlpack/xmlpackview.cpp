#include "xmlpackview.h"
#include "xmlpack.h"

#include <QDebug>
#include <QTreeWidget>
#include <QVBoxLayout>


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

}
void LXMLPackView::resetView()
{
    m_view->clear();
    if (m_rootItem)
    {
        delete m_rootItem;
        m_rootItem = NULL;
    }

    m_rootItem = new LXMLPackViewItem(m_packet->rootElement());

}


//LXMLPackViewItem
LXMLPackViewItem::LXMLPackViewItem(const LXMLPackElement *node, QTreeWidgetItem *parent)
    :QTreeWidgetItem(parent),
      m_node(node)
{


}



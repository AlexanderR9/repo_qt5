#include "xmlpackview.h"
#include "xmlpack.h"
#include "xmlpackelement.h"
#include "xmlpacktype.h"

#include <QDebug>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>

#define VALUE_COL       5
#define DEVIATION_COL   6



// LXMLPackView
LXMLPackView::LXMLPackView(const QString &title, QWidget *parent)
    :QGroupBox(title, parent),
      m_view(NULL),
      m_rootItem(NULL),
      m_packet(NULL),
      m_readOnly(true),
      m_doublePrecision(2)
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

    QStringList headers;
    headers << "Cation" << "Type" << "Size, b" << "Offset" << "Childs" << "Value" << "Deviation, %";
    m_view->setColumnCount(headers.count());
    m_view->header()->setDefaultAlignment(Qt::AlignCenter);
    m_view->setHeaderLabels(headers);
    m_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //qDebug()<<QString("LXMLPackView::initWidget() headers size %1,   ColumnCount %2").arg(headers.count()).arg(m_view->columnCount());


}
void LXMLPackView::setPacket(LXMLPackObj *p)
{   
    if (!p)
    {
        qWarning("LXMLPackView::setPacket - WARNING packet object is NULL!");
        return;
    }

    qDebug("LXMLPackView::setPacket 1");
    disconnect(m_view, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotItemActivate(QTreeWidgetItem*, int)));
    disconnect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));

    //qDebug("LXMLPackView::setPacket 2");
    resetView();

    //qDebug("LXMLPackView::setPacket 3");
    m_packet = p;
    reloadView();
    //qDebug("LXMLPackView::setPacket 4");

    if (m_rootItem)
        m_rootItem->setReadOnly(m_readOnly);

    connect(m_view, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotItemActivate(QTreeWidgetItem*, int)));
    connect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));
    qDebug("LXMLPackView::setPacket 5");
}
void LXMLPackView::reloadView()
{
    if (!m_packet) return;
    if (m_packet->invalid()) return;

    m_rootItem = new LXMLPackViewItem(m_packet->rootElementVar());
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
    m_rootItem = NULL;
    /*
    if (m_rootItem)
    {
        delete m_rootItem;
        m_rootItem = NULL;
    }
    */

    if (m_packet)
    {
        delete m_packet;
        m_packet = NULL;
    }
}
void LXMLPackView::slotItemActivate(QTreeWidgetItem *item, int column)
{
    if (isEditableCol(column) && !isReadOnly())
    {
        m_view->editItem(item, column);
    }
}
void LXMLPackView::setPacketData(const QByteArray &ba, bool &ok, bool singleFloatPrecision)
{
    ok = false;
    if (m_packet)
        m_packet->fromByteArray(ba, ok, singleFloatPrecision);
}
void LXMLPackView::fromPacket(QByteArray &ba, bool singleFloatPrecision)
{
    if (m_packet)
        m_packet->toByteArray(ba, singleFloatPrecision);
}
void LXMLPackView::setPacketByteOrder(int bo)
{
    if (m_packet)
        m_packet->setByteOrder(bo);
}
void LXMLPackView::slotItemValueChanged(QTreeWidgetItem *item, int column)
{
    LXMLPackViewItem *pack_item = static_cast<LXMLPackViewItem*>(item);
    if (pack_item)
    {
        disconnect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));
        pack_item->changePackValue(column);
        connect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));

    }
}
bool LXMLPackView::isEditableCol(int col) const
{
    return (col == VALUE_COL || col == DEVIATION_COL);
}
void LXMLPackView::updateValues()
{
    if (m_rootItem)
        m_rootItem->updateValues();
}


//LXMLPackViewItem
LXMLPackViewItem::LXMLPackViewItem(LXMLPackElement *node, QTreeWidgetItem *parent)
    :QTreeWidgetItem(parent),
      m_node(node)
{
    updateColumnsText();
    updateValue();
    updateColumnsColor();

    if (m_node->hasChilds())
        loadNodeChilds();
}
void LXMLPackViewItem::loadNodeChilds()
{
    int n = m_node->childsCount();
    for (int i=0; i<n; i++)
    {
        LXMLPackElement *child_node = m_node->childAtVar(i);
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
    setText(5, QString());
    setText(6, QString());

}
void LXMLPackViewItem::updateColumnsColor()
{
    int n_col = columnCount();
    if (m_node->isRoot())
    {
        for (int i=0; i<n_col; i++)
            setTextColor(i, Qt::blue);
        setText(1, "root");
    }
    else if (m_node->isArr())
    {
        for (int i=0; i<n_col; i++)
            setTextColor(i, QColor(220, 120, 0));
        setText(1, QString("arr[%1]").arg(m_node->arrSize()));
    }
    else if (m_node->isNode())
    {
        for (int i=0; i<n_col; i++)
            setTextColor(i, QColor(150, 150, 150));
    }
    else if (m_node->isTime())
    {
        for (int i=0; i<n_col; i++)
            setTextColor(i, QColor(30, 170, 170));
    }

    setBackgroundColor(VALUE_COL, QColor(250, 250, 220));
    setBackgroundColor(DEVIATION_COL, QColor(250, 250, 220));

}
void LXMLPackViewItem::updateValue()
{
    if (m_node->isData())
    {
        setText(DEVIATION_COL, m_node->strValueDeviation());
        setText(VALUE_COL, m_node->strValue(3));
    }
}
void LXMLPackViewItem::changePackValue(int col)
{
    //qDebug()<<QString("%1: changePackValue col=%2").arg(text(0)).arg(col);
    bool ok;
    switch (col)
    {
        case VALUE_COL:
        {
            m_node->setNewValue(text(VALUE_COL), ok);
            break;
        }
        case DEVIATION_COL:
        {
            m_node->setNewValueDeviation(text(DEVIATION_COL), ok);
            break;
        }
        default:
        {
            qWarning()<<QString("LXMLPackViewItem::changePackValue  WARNING - col %1 is not editable").arg(col);
            return;
        }
    }

    updateValue();
}
void LXMLPackViewItem::setReadOnly(bool b)
{
    if (!b && m_node && m_node->isData()) setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    else setFlags(Qt::ItemIsEnabled);

    int n = childCount();
    for (int i=0; i<n; i++)
    {
        LXMLPackViewItem *pack_item = static_cast<LXMLPackViewItem*>(child(i));
        if (pack_item) pack_item->setReadOnly(b);
    }
}
void LXMLPackViewItem::updateValues()
{
    updateValue();

    int n = childCount();
    for (int i=0; i<n; i++)
    {
        LXMLPackViewItem *pack_item = static_cast<LXMLPackViewItem*>(child(i));
        if (pack_item) pack_item->updateValues();
        else qWarning("LXMLPackViewItem::updateValues() WARNING child pack_item is NULL");
    }
}



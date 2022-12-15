#include "xmlpackview.h"
#include "xmlpack.h"
#include "xmlpackelement.h"
#include "xmlpacktype.h"
#include "ltime.h"

#include <QDebug>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>

#define KKS_COL         4
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
    m_view->setObjectName("view_widget");
    v_lay->addWidget(m_view);

    QStringList headers;
    headers << "Caption" << "Type" << "Size, b" << "Offset" << "Childs" << "Value" << "Deviation, %";
    m_view->setColumnCount(headers.count());
    m_view->header()->setDefaultAlignment(Qt::AlignCenter);
    m_view->setHeaderLabels(headers);
    m_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
bool LXMLPackView::kksUsed() const
{
    return (m_packet ? m_packet->kksUsed() : false);
}
void LXMLPackView::setIntValueByPath(QString path, qint64 v, bool &ok)
{
    if (m_packet)
        m_packet->setIntValueByPath(path, v, ok);
}
void LXMLPackView::setDoubleValueByPath(QString path, double v, bool &ok)
{
    if (m_packet)
        m_packet->setDoubleValueByPath(path, v, ok);
}
qint64 LXMLPackView::getIntValueByPath(QString path, bool &ok)
{
    ok = false;
    if (m_packet)
        return m_packet->getIntValueByPath(path, ok);
    return -9;
}
double LXMLPackView::getDoubleValueByPath(QString path, bool &ok)
{
    ok = false;
    if (m_packet)
        return m_packet->getDoubleValueByPath(path, ok);
    return -9;
}
void LXMLPackView::setSelectionRowsMode()
{
    if (m_view)
    {
        m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    }
}
void LXMLPackView::setPacket(LXMLPackObj *p)
{   
    if (!p)
    {
        qWarning("LXMLPackView::setPacket - WARNING packet object is NULL!");
        return;
    }

    disconnect(m_view, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotItemActivate(QTreeWidgetItem*, int)));
    disconnect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));

    resetView();
    m_packet = p;
    if (m_packet->kksUsed())
        m_view->headerItem()->setText(KKS_COL, "KKS");
    reloadView();

    if (m_rootItem)
        m_rootItem->setReadOnly(m_readOnly);

    connect(m_view, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotItemActivate(QTreeWidgetItem*, int)));
    connect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));
}
void LXMLPackView::reloadView()
{
    if (!m_packet) return;
    if (m_packet->invalid()) return;

    m_rootItem = new LXMLPackViewItem(m_packet->rootElementVar(), NULL, kksUsed());
    m_view->addTopLevelItem(m_rootItem);
    m_view->expandAll();
    resizeColumns();

    if (isReadOnly())
        m_view->hideColumn(DEVIATION_COL);
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
void LXMLPackView::nextRandValues()
{
    if (m_packet)
    {
        m_packet->nextRandValues();
        updateValues();
    }
}
void LXMLPackView::slotItemValueChanged(QTreeWidgetItem *item, int column)
{
        //qDebug()<<QString("%1: slotItemValueChanged col=%2").arg(item->text(0)).arg(column);
    LXMLPackViewItem *pack_item = static_cast<LXMLPackViewItem*>(item);
    if (pack_item)
    {
        //disconnect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));
        pack_item->changePackValue(column);
        //connect(m_view, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemValueChanged(QTreeWidgetItem*, int)));
    }
}
bool LXMLPackView::isEditableCol(int col) const
{
    return (col == VALUE_COL || col == DEVIATION_COL);
}
void LXMLPackView::updateValues()
{
    if (m_rootItem)
    {
        m_rootItem->setDoublePrecision(m_doublePrecision);
        m_rootItem->updateValues();
    }
}
void LXMLPackView::setExpandLevel(int level)
{
    if (!m_rootItem) return;
    if (m_view) m_view->expandToDepth(level-1);
}


//LXMLPackViewItem
LXMLPackViewItem::LXMLPackViewItem(LXMLPackElement *node, QTreeWidgetItem *parent, bool kks_used)
    :QTreeWidgetItem(parent),
      m_node(node),
      m_editable(false)
{
    setData(VALUE_COL, Qt::UserRole, 3);
    setData(KKS_COL, Qt::UserRole, kks_used);
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

        LXMLPackViewItem *child_item = new LXMLPackViewItem(child_node, this, data(KKS_COL, Qt::UserRole).toBool());
        Q_UNUSED(child_item);
    }
}
void LXMLPackViewItem::updateColumnsText()
{
    setText(0, m_node->caption());
    setText(1, XMLPackStatic::xmlAttrName(m_node->dataType()));
    setText(2, QString::number(m_node->size()));
    setText(3, QString::number(m_node->offset()));
    setText(5, QString());
    setText(6, QString());

    //update kks
    QString kks_text = QString::number(m_node->childsCount());
    if (data(KKS_COL, Qt::UserRole).toBool()) kks_text = m_node->kks();
    setText(KKS_COL, kks_text);
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
            setTextColor(i, QColor(30, 140, 30));
    }

    QColor value_col = (isEditable() ? QColor(250, 250, 220) : QColor(220, 220, 220));
    setBackgroundColor(VALUE_COL, value_col);
    setBackgroundColor(DEVIATION_COL, value_col);
}
void LXMLPackViewItem::updateValue()
{
    if (m_node->isData())
    {
        setText(DEVIATION_COL, m_node->strValueDeviation());
        //QString sv = m_node->strValue(data(VALUE_COL, Qt::UserRole).toInt());
        //setText(VALUE_COL, sv);
        setText(VALUE_COL, m_node->strValue(data(VALUE_COL, Qt::UserRole).toInt()));
    }
    else if (m_node->isTime())
    {
        if (m_node->hasChilds())
        {
            timespec tm;
            tm.tv_sec = m_node->childAt(0)->getValue().i_value;
            tm.tv_nsec = m_node->childAt(1)->getValue().i_value;
            setText(VALUE_COL, LTime::strTimeSpec(tm));
        }
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
            //qDebug()<<QString("changePackValue for %1").arg(text(0));
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
    //updateValue();
}
void LXMLPackViewItem::setReadOnly(bool b)
{
    m_editable = !b;
    if (!b && m_node && m_node->isData()) setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    else setFlags(Qt::ItemIsEnabled);

    int n = childCount();
    for (int i=0; i<n; i++)
    {
        LXMLPackViewItem *pack_item = static_cast<LXMLPackViewItem*>(child(i));
        if (pack_item) pack_item->setReadOnly(b);
    }
    updateColumnsColor();
}
void LXMLPackViewItem::setDoublePrecision(quint8 dp)
{
    setData(VALUE_COL, Qt::UserRole, dp);
    int n = childCount();
    for (int i=0; i<n; i++)
    {
        LXMLPackViewItem *pack_item = static_cast<LXMLPackViewItem*>(child(i));
        if (pack_item) pack_item->setDoublePrecision(dp);
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



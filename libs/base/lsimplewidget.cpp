#include "lsimplewidget.h"
#include "ltable.h"


#include <QDebug>
#include <QSettings>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>


//LSimpleWidget
LSimpleWidget::LSimpleWidget(QWidget *parent, int t)
    :QWidget(parent),
      m_spliterType(t),
      v_splitter(NULL),
      h_splitter(NULL)
{
    setObjectName("lsimple_widget");

    init();
}
void LSimpleWidget::setSpacing(int a)
{
    if (a < 0 || a > 100) return;
    if (layout()) layout()->setSpacing(a);
}
void LSimpleWidget::init()
{
    if (layout()) delete layout();
    setLayout(new QVBoxLayout(0));

    if (!onlyHorizontal()) v_splitter = new QSplitter(Qt::Vertical, this);
    if (!onlyVertical()) h_splitter = new QSplitter(Qt::Horizontal, this);

    //one splitters variant
    if (onlyVertical())
    {
        layout()->addWidget(v_splitter);
        return;
    }
    if (onlyHorizontal())
    {
        layout()->addWidget(h_splitter);
        return;
    }

    //two splitters variant
    if ((m_spliterType%10 == 2))
    {
        layout()->addWidget(h_splitter);
        h_splitter->addWidget(v_splitter);
    }
    else
    {
        layout()->addWidget(v_splitter);
        v_splitter->addWidget(h_splitter);
    }
}
void LSimpleWidget::load(QSettings &settings)
{
    QByteArray ba = settings.value(QString("%1/v_spltitter_state").arg(objectName()), QByteArray()).toByteArray();
    if (v_splitter && !ba.isEmpty()) v_splitter->restoreState(ba);

    ba.clear();
    ba = settings.value(QString("%1/h_spltitter_state").arg(objectName()), QByteArray()).toByteArray();
    if (h_splitter && !ba.isEmpty()) h_splitter->restoreState(ba);

}
void LSimpleWidget::save(QSettings &settings)
{
    if (v_splitter)
        settings.setValue(QString("%1/v_spltitter_state").arg(objectName()), v_splitter->saveState());

    if (h_splitter)
        settings.setValue(QString("%1/h_spltitter_state").arg(objectName()), h_splitter->saveState());
}
bool LSimpleWidget::invalidType() const
{
    if (m_spliterType < 11 || m_spliterType > 32) return true;
    return false;
}
bool LSimpleWidget::onlyVertical() const
{
    if (invalidType()) return false;
    return (m_spliterType/10 == 1);
}
bool LSimpleWidget::onlyHorizontal() const
{
    if (invalidType()) return false;
    return (m_spliterType/10 == 2);
}


//LTableWidgetBox
LTableWidgetBox::LTableWidgetBox(QWidget *parent, int t)
    :QGroupBox("Table Box", parent),
      m_table(NULL)
{
    setObjectName("ltable_widget_box");

    if(layout()) delete layout();
    if (t == 2) setLayout(new QHBoxLayout(0));
    else setLayout(new QVBoxLayout(0));

    init();
}
void LTableWidgetBox::init()
{
    m_table = new QTableWidget(this);
    LTable::fullClearTable(m_table);
    layout()->addWidget(m_table);
}
void LTableWidgetBox::setHeaderLabels(const QStringList &list, int orintation)
{
    LTable::setTableHeaders(m_table, list, orintation);
}
void LTableWidgetBox::vHeaderHide()
{
    m_table->verticalHeader()->hide();
}
QTableWidget* LTableWidgetBox::table() const
{
    return m_table;
}


//LListWidgetBox
LListWidgetBox::LListWidgetBox(QWidget *parent, int t)
    :QGroupBox("List Box", parent),
      m_listWidget(NULL)
{
    setObjectName("llist_widget_box");

    if(layout()) delete layout();
    if (t == 2) setLayout(new QHBoxLayout(0));
    else setLayout(new QVBoxLayout(0));

    init();
}
void LListWidgetBox::init()
{
    m_listWidget = new QListWidget(this);
    m_listWidget->clear();
    layout()->addWidget(m_listWidget);
}
QListWidget* LListWidgetBox::listWidget() const
{
    return m_listWidget;
}


//LTreeWidgetBox
LTreeWidgetBox::LTreeWidgetBox(QWidget *parent, int t)
    :QGroupBox("Tree view Box", parent),
      m_view(NULL)
{
    setObjectName("ltreeview_widget_box");

    if(layout()) delete layout();
    if (t == 2) setLayout(new QHBoxLayout(0));
    else setLayout(new QVBoxLayout(0));

    init();
}
void LTreeWidgetBox::init()
{
    m_view = new QTreeWidget(this);
    m_view->clear();
    layout()->addWidget(m_view);
}
void LTreeWidgetBox::setHeaderLabels(const QStringList &list)
{
    m_view->setColumnCount(list.count());
    m_view->header()->setDefaultAlignment(Qt::AlignCenter);
    m_view->setHeaderLabels(list);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);

}
QTreeWidgetItem* LTreeWidgetBox::rootItem() const
{
    if (!m_view) return NULL;
    if (m_view->topLevelItemCount() > 0) return m_view->topLevelItem(0);
    return NULL;
}
void LTreeWidgetBox::resizeByContents()
{
    if (!m_view) return;
    int n = m_view->columnCount();
    if (n <= 0) return;
    for (int i=0; i<n; i++)
    {
        m_view->resizeColumnToContents(n-i-1);
    }
}
void LTreeWidgetBox::expandAll()
{
    if (m_view) m_view->expandAll();
}
void LTreeWidgetBox::expandLevel(int level)
{
    if (m_view)
    {
        if (level < 0) expandAll();
        else m_view->expandToDepth(level);
    }
}
void LTreeWidgetBox::clearView()
{
    if (m_view) m_view->clear();
}
void LTreeWidgetBox::loadJSON(const QJsonObject &json, QString root_title)
{
    clearView();
    QStringList headers;
    headers << "Key" << "Value" << "Data type" << "Size";
    setHeaderLabels(headers);

    if (root_title.isEmpty()) headers.replace(0, "JSON");
    else headers.replace(0, root_title);
    for (int i=1; i<headers.count(); i++) headers[i] = QString();
    this->addRootItem(headers);
    setRootItemAttrs(Qt::gray, 0, true);

    QStringList keys(json.keys());
    foreach (const QString &v, keys)
        loadJSONValue(v, json.value(v), rootItem());
}
void LTreeWidgetBox::loadJSONValue(const QString &key, const QJsonValue &j_value, QTreeWidgetItem *parent_item)
{
    if (!parent_item) return;

    QStringList data;
    data.append(key);
    getJSONValueType(data, j_value);
    QTreeWidgetItem *item = new QTreeWidgetItem(parent_item, data);
    if (j_value.isArray()) loadJSONValueArray(j_value.toArray(), item);
    else if (j_value.isObject()) loadJSONValueObj(j_value.toObject(), item);
}
void LTreeWidgetBox::loadJSONValueArray(const QJsonArray &j_arr, QTreeWidgetItem *parent_item)
{
    if (!parent_item) return;

    int n = j_arr.count();
    parent_item->setText(parent_item->columnCount()-1, QString::number(n));
    LTreeWidgetBox::setAttrsItem(parent_item, QColor(250, 130, 0), -1, false, true);

    if (j_arr.isEmpty())
    {
        QStringList data = (QStringList() << "empty" << QString() << QString() << QString());
        new QTreeWidgetItem(parent_item, data);
        setRootItemAttrs(Qt::gray, -1, false, true, 8);
        return;
    }
    for (int i=0; i<n; i++)
    {
        const QJsonValue& element = j_arr.at(i);
        QStringList data;
        data.append(QString("[%1]").arg(i));
        getJSONValueType(data, element);

        QTreeWidgetItem *item = new QTreeWidgetItem(parent_item, data);
        if (element.isArray()) loadJSONValueArray(element.toArray(), item);
        else if (element.isObject()) loadJSONValueObj(element.toObject(), item);
    }
}
void LTreeWidgetBox::loadJSONValueObj(const QJsonObject &j_obj, QTreeWidgetItem *parent_item)
{
    if (!parent_item) return;

    QStringList keys(j_obj.keys());
    parent_item->setText(parent_item->columnCount()-1, QString::number(keys.count()));
    LTreeWidgetBox::setAttrsItem(parent_item, QColor(30, 130, 230), -1, false, true);
    foreach (const QString &v, keys)
        loadJSONValue(v, j_obj.value(v), parent_item);
}
void LTreeWidgetBox::getJSONValueType(QStringList &data, const QJsonValue &j_value)
{
    switch (j_value.type())
    {
        case QJsonValue::Null:      {data << QString("err") << "NULL"; break;}
        case QJsonValue::Bool:      {data << QString(j_value.toBool()?"true":"false") << "BOOL"; break;}
        case QJsonValue::Double:    {data << QString::number(j_value.toDouble(-1), 'f' ,2) << "DOUBLE"; break;}
        case QJsonValue::String:    {data << j_value.toString() << "STRING"; break;}
        case QJsonValue::Array:     {data << QString("-") << "ARRAY"; break;}
        case QJsonValue::Object:    {data << QString("-") << "OBJECT"; break;}
        default:                    {data << QString("?") << "UNDEFINED"; break;}
    }
    data << QString();
}
void LTreeWidgetBox::clearRoot()
{
    QTreeWidgetItem *r_item = rootItem();
    if (r_item)
    {
        int n = r_item->childCount();
        if (n <= 0) return;

        for (int i=n-1; i>=0; i--)
        {
            QTreeWidgetItem *item = r_item->child(i);
            r_item->removeChild(item);
        }
    }
}
void LTreeWidgetBox::addRootItem(const QStringList &list)
{
    if (rootItem()) return;

    QTreeWidgetItem *r_item = new QTreeWidgetItem(list);
    m_view->addTopLevelItem(r_item);
}
void LTreeWidgetBox::setAttrsItem(QTreeWidgetItem *item, QColor tc, int col, bool bold, bool italic, int size)
{
    if (!item || col >= item->columnCount()) return;

    if (col < 0)
    {
        QFont font(item->font(0));
        for (int j=0; j<item->columnCount(); j++)
        {
            font.setBold(bold);
            font.setItalic(italic);
            if (size > 0) font.setPointSize(size);
            item->setFont(j, font);
            item->setTextColor(j, tc);
        }
    }
    else
    {
        QFont font(item->font(col));
        font.setBold(bold);
        font.setItalic(italic);
        if (size > 0) font.setPointSize(size);
        item->setFont(col, font);
        item->setTextColor(col, tc);
    }
}
void LTreeWidgetBox::setRootItemAttrs(QColor tc, int col, bool bold, bool italic, int size)
{
    QTreeWidgetItem *r_item = rootItem();
    LTreeWidgetBox::setAttrsItem(r_item, tc, col, bold, italic, size);
}



//LTabWidgetBox
LTabWidgetBox::LTabWidgetBox(QWidget *parent, int t)
    :QGroupBox("Tab pages box", parent),
      m_tab(NULL)
{
    setObjectName("ltab_widget_box");

    if(layout()) delete layout();
    if (t == 2) setLayout(new QHBoxLayout(0));
    else setLayout(new QVBoxLayout(0));

    init();
}
void LTabWidgetBox::init()
{
    m_tab = new QTabWidget(this);
    m_tab->clear();
    layout()->addWidget(m_tab);
}
void LTabWidgetBox::removeAllPages()
{
    if (!hasPages()) return;
    int n = pageCount();
    for(int i=n-1; i>=0; i--)
    {
        QWidget *w = m_tab->widget(i);
        m_tab->removeTab(i);
        delete w;
        w = NULL;
    }
}
int LTabWidgetBox::pageCount() const
{
    return m_tab->count();
}


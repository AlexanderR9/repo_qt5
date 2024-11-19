#include "lsimplewidget.h"
#include "ltable.h"
#include "lsearch.h"

#include <QDebug>
#include <QSettings>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QClipboard>
#include <QLabel>
#include <QLineEdit>
#include <QGuiApplication>


//LSimpleWidget
LSimpleWidget::LSimpleWidget(QWidget *parent, int t)
    :QWidget(parent),
      m_spliterType(t),
      v_splitter(NULL),
      h_splitter(NULL),
      m_userSign(-1),
      m_userData(QString())
{
    setObjectName("lsimple_widget");

    init();
}
void LSimpleWidget::setSpacing(int a)
{
    if (a < 0 || a > 100) return;
    if (layout()) layout()->setSpacing(a);
}
void LSimpleWidget::addWidgetToSplitter(QWidget *w, int orintation)
{
    if (!w) {qWarning("LSimpleWidget::addWidgetToSplitter WARNING - widget is NULL"); return;}
    switch (orintation)
    {
        case Qt::Horizontal:
        {
            if (h_splitter) h_splitter->addWidget(w);
            break;
        }
        case Qt::Vertical:
        {
            if (v_splitter) v_splitter->addWidget(w);
            break;
        }
        default: break;
    }
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
    if (m_spliterType < 10 || m_spliterType > 32) return true;
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
      //m_lastSortOrder(0)
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

    connect(m_table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(slotItemDoubleClicked(QTableWidgetItem*)));
}
void LTableWidgetBox::slotItemDoubleClicked(QTableWidgetItem *item)
{
    if (!item) return;
    QGuiApplication::clipboard()->setText(item->text());
}
void LTableWidgetBox::setHeaderLabels(const QStringList &list, int orintation)
{
    LTable::setTableHeaders(m_table, list, orintation);
}
void LTableWidgetBox::removeAllRows()
{
    LTable::removeAllRowsTable(m_table);
}
void LTableWidgetBox::vHeaderHide()
{
    m_table->verticalHeader()->hide();
}
QTableWidget* LTableWidgetBox::table() const
{
    return m_table;
}
void LTableWidgetBox::resizeByContents()
{
    LTable::resizeTableContents(m_table);
}
void LTableWidgetBox::setSelectionMode(int behavior, int mode)
{
    if (m_table)
    {
        m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior(behavior));
        m_table->setSelectionMode(QAbstractItemView::SelectionMode(mode));
        m_table->update();
    }
}
void LTableWidgetBox::setSelectionColor(QString bg_color, QString text_color)
{
    if (m_table)
    {
        QString style_value = QString("selection-color: %1; selection-background-color: %2;").arg(text_color).arg(bg_color);
        m_table->setStyleSheet(style_value);
    }
}
void LTableWidgetBox::sortingOn()
{
    if (m_table && m_table->columnCount() > 0)
    {
        QHeaderView *hv = m_table->horizontalHeader();
        if (hv && !hv->isHidden())
        {
            for (int j=0; j<m_table->columnCount(); j++)
                m_table->horizontalHeaderItem(j)->setData(Qt::UserRole, int(0));

            connect(hv, SIGNAL(sectionClicked(int)), this, SLOT(slotSortByColumn(int)));
        }
    }
}
void LTableWidgetBox::addSortingData(quint8 col, SortingDataType t)
{
    if (!m_table) return;
    if (col < m_table->columnCount())
        m_sortingData.insert(col, t);
}
void LTableWidgetBox::slotSortString(quint8 col, int order)
{
    Q_UNUSED(order);
    m_table->sortByColumn(col);
}
void LTableWidgetBox::slotSortNumeric(quint8 col, int order)
{
    int row = -1;
    int n_rows = m_table->rowCount();

    if (order == 0)
    {
        double min = 0;
        for (int i=0; i<n_rows; i++)
        {
            min = LTable::minNumericColValue(m_table, col, row, i);
            if (row > 0) LTable::shiftTableRowToBegin(m_table, row);
        }
    }
    else
    {
        double max = 0;
        for (int i=0; i<n_rows; i++)
        {
            max = LTable::maxNumericColValue(m_table, col, row, i);
            if (row > 0) LTable::shiftTableRowToBegin(m_table, row);
        }
    }
}
void LTableWidgetBox::slotSortByColumn(int col)
{
    qDebug()<<QString("slotSortByColumn %1").arg(col);

    if (!m_sortingData.contains(col)) return;
    if (m_table->rowCount() < 3) return;
    if (col < 0 || col >= m_table->columnCount()) return;

    m_table->clearSelection();
    int sort_order = m_table->horizontalHeaderItem(col)->data(Qt::UserRole).toInt();
    if (sort_order != 0) m_table->horizontalHeaderItem(col)->setData(Qt::UserRole, int(0));
    else m_table->horizontalHeaderItem(col)->setData(Qt::UserRole, int(1));

    switch (m_sortingData.value(col))
    {
        case sdtString: {slotSortString(col, sort_order); break;}
        case sdtNumeric: {slotSortNumeric(col, sort_order); break;}
        default: {qWarning()<<QString("LTableWidgetBox::slotSortByColumn WARNING invalid sort_type %1").arg(m_sortingData.value(col)); break;}
    }

    resizeByContents();
    m_table->scrollToTop();
    //m_table->selectRow(0);
}


//LSearchTableWidgetBox
LSearchTableWidgetBox::LSearchTableWidgetBox(QWidget *parent)
    :LTableWidgetBox(parent, 1),
      m_searchObj(NULL),
      m_searchEdit(NULL),
      m_searchLabel(NULL)
{
    setObjectName("lsearchtable_widget_box");

    m_searchEdit = new QLineEdit(this);
    m_searchLabel = new QLabel("Count: ", this);
    layout()->removeWidget(m_table);
    layout()->addWidget(m_searchEdit);
    layout()->addWidget(m_table);
    layout()->addWidget(m_searchLabel);

    m_searchObj = new LSearch(m_searchEdit, this);
    m_searchObj->addTable(m_table, m_searchLabel);
    m_searchObj->exec();

    connect(m_searchObj, SIGNAL(signalSearched()), this, SIGNAL(signalSearched()));
}
void LSearchTableWidgetBox::setTextLabel(const QString &s)
{
    if (m_searchLabel) m_searchLabel->setText(s);
}
void LSearchTableWidgetBox::searchExec()
{
    m_table->clearSelection();
    m_searchObj->exec();
    resizeByContents();
}
void LSearchTableWidgetBox::searchReset()
{
    m_searchEdit->clear();
    searchExec();
}
void LSearchTableWidgetBox::searchEditHide()
{
    if (m_searchEdit)
    {
        layout()->removeWidget(m_searchEdit);
        m_searchEdit->hide();
    }
}
void LSearchTableWidgetBox::setSearchEdit(const QLineEdit *edit)
{
    if (m_searchObj) m_searchObj->setSearchEdit(edit);
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

    connect(m_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));
}
QListWidget* LListWidgetBox::listWidget() const
{
    return m_listWidget;
}
void LListWidgetBox::setRowColor(quint16 i, QString s_color)
{
    if (!m_listWidget || !s_color.contains('#')) return;
    if (i < m_listWidget->count()) m_listWidget->item(i)->setBackgroundColor(s_color);
}
void LListWidgetBox::setRowTextColor(quint16 i, QString s_color)
{
    if (!m_listWidget || !s_color.contains('#')) return;
    if (i < m_listWidget->count()) m_listWidget->item(i)->setTextColor(s_color);
}
void LListWidgetBox::setSelectionColor(QString bg_color, QString text_color)
{
    if (m_listWidget)
    {
        QString style_value = QString("selection-color: %1; selection-background-color: %2;").arg(text_color).arg(bg_color);
        m_listWidget->setStyleSheet(style_value);
    }
}
void LListWidgetBox::setBaseColor(QString bg_color, QString text_color)
{
    if (m_listWidget)
    {
        QString style_value = QString("color: %1; background-color: %2;").arg(text_color).arg(bg_color);
        m_listWidget->setStyleSheet(style_value);
    }
}
void LListWidgetBox::addItem(QString text, QString icon_path)
{
    QListWidgetItem *l_item = NULL;
    if (icon_path.trimmed().isEmpty()) l_item = new QListWidgetItem(text);
    else l_item = new QListWidgetItem(QIcon(icon_path), text);
    m_listWidget->addItem(l_item);
}
void LListWidgetBox::removeItemByValue(QString text)
{
    if (m_listWidget)
    {
        for (int i=0; i<m_listWidget->count(); i++)
        {
            if (m_listWidget->item(i)->text() == text)
            {
                QListWidgetItem *item = m_listWidget->takeItem(i);
                delete item;
                item = NULL;
                break;
            }
        }
    }
}
void LListWidgetBox::setSelectionMode(int behavior, int mode)
{
    if (m_listWidget)
    {
        m_listWidget->setSelectionBehavior(QAbstractItemView::SelectionBehavior(behavior));
        m_listWidget->setSelectionMode(QAbstractItemView::SelectionMode(mode));
        m_listWidget->update();
    }
}
void LListWidgetBox::clearSelection()
{
    if (m_listWidget)
    {
        m_listWidget->clearSelection();
        m_listWidget->clearFocus();
    }
}
QStringList LListWidgetBox::selectedValues() const
{
    QStringList list;
    if (m_listWidget)
    {
        QList<QListWidgetItem*> i_list(m_listWidget->selectedItems());
        foreach (const QListWidgetItem *v, i_list)
            if (v) list << v->text();
    }
    return list;
}
QList<int> LListWidgetBox::selectedRows() const
{
    QList<int> list;
    if (m_listWidget)
    {
        for (int i=0; i<m_listWidget->count(); i++)
            if (m_listWidget->isItemSelected(m_listWidget->item(i))) list << i;
    }
    return list;
}
bool LListWidgetBox::valueContain(const QString &s) const
{
    if (m_listWidget)
    {
        for (int i=0; i<m_listWidget->count(); i++)
            if (m_listWidget->item(i)->text() == s) return true;
    }
    return false;
}
void LListWidgetBox::slotItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    QGuiApplication::clipboard()->setText(item->text());
}


//LSearchListWidgetBox
LSearchListWidgetBox::LSearchListWidgetBox(QWidget *parent)
    :LListWidgetBox(parent, 1),
      m_searchObj(NULL),
      m_searchEdit(NULL),
      m_searchLabel(NULL)
{
    setObjectName("lsearchlist_widget_box");

    m_searchEdit = new QLineEdit(this);
    m_searchLabel = new QLabel("Count: ", this);
    layout()->removeWidget(m_listWidget);
    layout()->addWidget(m_searchEdit);
    layout()->addWidget(m_listWidget);
    layout()->addWidget(m_searchLabel);

    m_searchObj = new LSearch(m_searchEdit, this);
    m_searchObj->addList(m_listWidget, m_searchLabel);
    m_searchObj->exec();

    connect(m_searchObj, SIGNAL(signalSearched()), this, SIGNAL(signalSearched()));
}
void LSearchListWidgetBox::searchExec()
{
    m_listWidget->clearSelection();
    m_searchObj->exec();
    //resizeByContents();
}
void LSearchListWidgetBox::searchReset()
{
    m_searchEdit->clear();
    searchExec();
}
void LSearchListWidgetBox::setTextLabel(const QString &s)
{
    if (m_searchLabel) m_searchLabel->setText(s);
}
void LSearchListWidgetBox::searchEditHide()
{
    if (m_searchEdit)
    {
        layout()->removeWidget(m_searchEdit);
        m_searchEdit->hide();
    }
}
void LSearchListWidgetBox::setSearchEdit(const QLineEdit *edit)
{
    if (m_searchObj) m_searchObj->setSearchEdit(edit);
}





//LTreeWidgetBox
LTreeWidgetBox::LTreeWidgetBox(QWidget *parent, int t)
    :QGroupBox("Tree view Box", parent),
      m_view(NULL),
      m_expandLevel(-1)
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

    connect(m_view, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*, int)));
}
void LTreeWidgetBox::slotItemDoubleClicked(QTreeWidgetItem *item, int col)
{
    if (!item || col < 0) return;
    QGuiApplication::clipboard()->setText(item->text(col));
}
void LTreeWidgetBox::setHeaderLabels(const QStringList &list)
{
    m_view->setColumnCount(list.count());
    m_view->header()->setDefaultAlignment(Qt::AlignCenter);
    m_view->setHeaderLabels(list);
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
void LTreeWidgetBox::expandLevel()
{
    if (m_view)
    {
        if (m_expandLevel < 0) expandAll();
        else m_view->expandToDepth(m_expandLevel);
    }
}
void LTreeWidgetBox::setSelectionMode(int behavior, int mode)
{
    if (m_view)
    {
        m_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior(behavior));
        m_view->setSelectionMode(QAbstractItemView::SelectionMode(mode));
        m_view->update();
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


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


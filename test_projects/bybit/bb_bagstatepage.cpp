#include "bb_bagstatepage.h"
#include "ltable.h"

#include <QTableWidget>
#include <QSplitter>


//BB_BagStatePage
BB_BagStatePage::BB_BagStatePage(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_table(NULL)
{
    setObjectName("bag_state_page");
    init();

}
void BB_BagStatePage::init()
{
    m_table = new LTableWidgetBox(this);
    m_table->setObjectName("bag_table");
    m_table->setTitle("Current state");
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    QStringList headers;
    headers.append("Value");
    m_table->setHeaderLabels(headers);

    headers.clear();
    headers << "Positions" << "Orders" << "Freezed sum" << "Current result (opened pos)";
    m_table->setHeaderLabels(headers, Qt::Vertical);
    for (int i=0; i<m_table->table()->rowCount(); i++)
        LTable::createTableItem(m_table->table(), i, 0, "-");

    h_splitter->addWidget(m_table);
    m_table->resizeByContents();
}


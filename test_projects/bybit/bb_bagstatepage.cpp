#include "bb_bagstatepage.h"
#include "ltable.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDebug>


//BB_BagStatePage
BB_BagStatePage::BB_BagStatePage(QWidget *parent)
    :BB_BasePage(parent, 20, rtBag),
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
    headers << "Positions" << "Orders" << "Freezed sum (pos/order)" << "Freezed sum (total)" << "Current result (opened pos)";
    m_table->setHeaderLabels(headers, Qt::Vertical);
    for (int i=0; i<m_table->table()->rowCount(); i++)
        LTable::createTableItem(m_table->table(), i, 0, "-");

    h_splitter->addWidget(m_table);
    m_table->resizeByContents();
}
void BB_BagStatePage::updateDataPage(bool force)
{
    Q_UNUSED(force);
    emit signalGetPosState(m_state);
    updateTable();
    m_table->resizeByContents();
}
void BB_BagStatePage::updateTable()
{
    QTableWidget *t = m_table->table();
    int row = 0;
    t->item(row, 0)->setText(QString::number(m_state.n_pos)); row++;
    t->item(row, 0)->setText(QString::number(m_state.n_order)); row++;
    t->item(row, 0)->setText(QString("%1/%2").arg(QString::number(m_state.freezed_pos, 'f', 1)).arg(QString::number(m_state.freezed_order, 'f', 1))); row++;
    t->item(row, 0)->setText(QString::number(m_state.sumFreezed(), 'f', 1)); row++;
    t->item(row, 0)->setText(QString::number(m_state.pos_result, 'f', 1));
    if (m_state.pos_result < 0) t->item(row, 0)->setTextColor(Qt::red);
    else if (m_state.pos_result < 0) t->item(row, 0)->setTextColor(Qt::blue);
    else t->item(row, 0)->setTextColor(Qt::gray);

}


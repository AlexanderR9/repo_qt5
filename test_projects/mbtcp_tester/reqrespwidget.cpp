#include "reqrespwidget.h"
#include "ltable.h"

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>

//Parametes ViewWidget
ReqRespWidget::ReqRespWidget(QWidget *parent)
    :LTableWidgetBox(parent, 1)
{

    this->setTitle("Last REQUEST-RESPONSE");
    QStringList headers;
    headers << "Request" << "Response";
    setHeaderLabels(headers, Qt::Horizontal);

    headers.clear();
    headers << "Transaction ID" << "Protocol ID" << "Payload len" << "Unit address" << "Function";
    setHeaderLabels(headers, Qt::Vertical);


    //QTableWidgetItem *item = new QTableWidgetItem("");
    for (int i=0; i<staticRows(); i++)
    {
        m_table->setItem(i, 0, new QTableWidgetItem(""));
        m_table->setItem(i, 1, new QTableWidgetItem(""));
    }

    for (int i=0; i<staticRows(); i++)
        for (int j=0; j<m_table->columnCount(); j++)
        {
            m_table->item(i, j)->setFlags(Qt::ItemIsEnabled);
            m_table->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }


    setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::NoSelection);
    this->resizeByContents();
}
void ReqRespWidget::updateTable(const QStringList &req_list, const QStringList &resp_list)
{
    if (!req_list.isEmpty()) resetTable();

    int cur_rows = m_table->rowCount();
    int n = qMax(req_list.count(), resp_list.count());
    if (req_list.isEmpty()) n = qMax(cur_rows, resp_list.count());

    QStringList empty_row; empty_row << "" << "";
    qDebug()<<QString("ReqRespWidget::updateTable   req_list(%1)  resp_list(%2)  max(%3)").arg(req_list.count()).arg(resp_list.count()).arg(n);
    foreach (const QString &v, req_list) qDebug()<<v;
    while (m_table->rowCount() < n)
    {
        LTable::addTableRow(m_table, empty_row);
        QTableWidgetItem *v_item = new QTableWidgetItem(QString("BYTE %1").arg(cur_rows-staticRows()));
        v_item->setTextAlignment(Qt::AlignCenter);
        m_table->setVerticalHeaderItem(cur_rows, v_item);
        cur_rows++;
    }

    for (int i=0; i<m_table->rowCount(); i++)
    {
        if (req_list.count() > i)
        {
            m_table->item(i, 0)->setText(req_list.at(i));
        }
        if (resp_list.count() > i)
        {
            m_table->item(i, 1)->setText(resp_list.at(i));
        }
        else m_table->item(i, 1)->setText("");
    }
    QColor c = (m_table->item(0, 1)->text() == "---") ? Qt::black : Qt::red;
    m_table->item(0, 1)->setTextColor(c);

    resizeByContents();
}
void ReqRespWidget::resetTable()
{
    while (m_table->rowCount() > staticRows())
        m_table->removeRow(staticRows());

    for (int i=0; i<staticRows(); i++)
        for (int j=0; j<m_table->columnCount(); j++)
            m_table->item(i, j)->setText("");
}



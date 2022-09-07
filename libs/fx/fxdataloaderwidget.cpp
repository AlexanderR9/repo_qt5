#include "fxdataloaderwidget.h"
#include "ltable.h"
#include "fxenums.h"

#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>


#define FXW_COUPLE_COL      0
#define FXW_TIMEFRAME_COL   1
#define FXW_DATASIZE_COL    2


//FXDataLoader
FXDataLoaderWidget::FXDataLoaderWidget(QWidget *parent)
    :QGroupBox(parent),
      m_table(NULL)
{
    setObjectName("data_loader_widget");

    initTable();
    updateTitle();

    connect(m_table, SIGNAL(itemSelectionChanged()), this, SIGNAL(signalSelectionChanged()));
}
void FXDataLoaderWidget::initTable()
{
    if (layout()) delete layout();
    setLayout(new QVBoxLayout(0));

    m_table = new QTableWidget(this);
    LTable::fullClearTable(m_table);
    m_table->verticalHeader()->hide();
    layout()->addWidget(m_table);

    QStringList headers;
    headers << "Couple" << "TF" << "Size";
    LTable::setTableHeaders(m_table, headers);

    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    LTable::resizeTableContents(m_table);
}
void FXDataLoaderWidget::clearTable()
{
    LTable::removeAllRowsTable(m_table);
}
void FXDataLoaderWidget::reloadData(const QList<FXCoupleDataParams> &list)
{
    clearTable();
    for (int i=0; i<list.count(); i++)
    {
        qDebug()<<list.at(i).toStr();
        QStringList row_data;
        row_data << list.at(i).couple << FXEnumStaticObj::strTimeFrame(list.at(i).timeframe) << QString::number(list.at(i).bar_count);
        LTable::addTableRow(m_table, row_data);
        m_table->item(i, FXW_TIMEFRAME_COL)->setData(Qt::UserRole, list.at(i).timeframe);
    }

    LTable::resizeTableContents(m_table);
    updateTitle();
}
void FXDataLoaderWidget::getSelection(QList<FXCoupleDataParams> &list)
{
    list.count();
    QList<int> sel_rows = LTable::selectedRows(m_table);
    if (sel_rows.isEmpty()) {qWarning("FXDataLoaderWidget::getSelection WARNING: empty selection data of table"); return;}

    for (int i=0; i<sel_rows.count(); i++)
    {
        int row = sel_rows.at(i);
        FXCoupleDataParams p;
        p.couple = m_table->item(row, FXW_COUPLE_COL)->text();
        p.timeframe = m_table->item(row, FXW_TIMEFRAME_COL)->data(Qt::UserRole).toInt();
        p.bar_count = m_table->item(row, FXW_DATASIZE_COL)->text().toUInt();
        list.append(p);
    }
}
void FXDataLoaderWidget::updateTitle()
{
    QString title("Loaded data");
    if (!emptyData()) title = QString("%1  (%2)").arg(title).arg(count());
    setTitle(title);
}
int FXDataLoaderWidget::count() const
{
    return m_table->rowCount();
}




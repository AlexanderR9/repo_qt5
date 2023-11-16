#include "apieventspage.h"
#include "ltable.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>

#define AMOUNT_COL      8


//APICouponPageAbstract
APIEventsPage::APIEventsPage(QWidget *parent)
    :APITablePageBase(parent)
{
    setObjectName("api_events_page");
    m_userSign = aptEvent;

    QStringList headers;
    headers << "Kind" << "Company" << "Ticker" << "Paper type" << "Currency" << "Date" << "N" << "Size" << "Amount";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Bag operations");

}
void APIEventsPage::slotLoadEvents(const QJsonObject &j_obj)
{
    m_events.clear();
    if (j_obj.isEmpty()) return;

    const QJsonValue &j_operations = j_obj.value("operations");
    if (!j_operations.isArray()) return;

    const QJsonArray &j_arr = j_operations.toArray();
    int n = j_arr.count();
    qDebug()<<QString(" APIEventsPage::slotLoadEvents arr_size %1").arg(n);
    for (int i=0; i<n; i++)
    {
        EventOperation e;
        e.fromJson(j_arr.at(i));
        if (!e.invalid()) m_events.append(e);
    }

    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_events.count()));
    reloadTableByData();
}
void APIEventsPage::reloadTableByData()
{
    resetPage();
    if (m_events.isEmpty())
    {
        emit signalMsg(QString("Event operations is empty!"));
        return;
    }

    foreach (const EventOperation &rec, m_events)
    {
        QPair<QString, QString> pair;
        emit signalGetPaperInfoByFigi(rec.uid, pair);

        QString ticker;
        emit signalGetTickerByFigi(rec.uid, ticker);
        if (ticker.length() < 2)  ticker = "---";
        if (pair.first.length() < 5)  pair.first = "---";

        QStringList row_data;
        row_data << rec.strKind() << pair.first << ticker << rec.paper_type << rec.currency;
        row_data << rec.date.toString(InstrumentBase::userDateMask()) << QString::number(rec.n_papers);
        row_data << QString::number(rec.size, 'f', 2) << QString::number(rec.amount, 'f', 1);
        LTable::addTableRow(m_tableBox->table(), row_data);

        int l_row = m_tableBox->table()->rowCount() - 1;
        if (rec.amount < 0) m_tableBox->table()->item(l_row, AMOUNT_COL)->setTextColor(Qt::darkRed);
        //if (rec.kind == EventOperation::etTax || rec.kind == EventOperation::etCommission)
            //LTable::setTableRowColor(m_tableBox->table(), l_row, "#FFCDFB");
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#D3D3D3", "#2F4F4F");
    m_tableBox->searchExec();
}



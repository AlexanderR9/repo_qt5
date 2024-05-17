#include "apicouponpage.h"
#include "cycleworker.h"
#include "lstaticxml.h"
#include "ltable.h"


#include <QDebug>
#include <QDomNode>
#include <QTableWidget>

#define FIGI_COL        1
#define DAY_SIZE_COL    7
#define DIV_SIZE_COL    5
//#define COUPON_DATE_COL 5

#define DAY_SIZE_LIMIT_HIGH         0.4
#define DAY_SIZE_LIMIT_LOW          0.2
#define DIV_YIELD_LIMIT_HIGH        6.5
#define DIV_YIELD_LIMIT_LOW         4.4




//APICouponPage
APICouponPage::APICouponPage(QWidget *parent)
    :APICouponPageAbstract(parent)
{
    setObjectName("api_coupon_page");
    m_userSign = aptCoupon;

    QStringList headers;
    headers << "Company" << "FIGI" << "Ticker" << "Currency" << "Period" << "Date" << "Size" << "Day size" << "Days to";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Coupon list");
}
void APICouponPage::createRecord(CouponRecordAbstract* &rec, const QString &figi)
{
    if (rec) {delete rec; rec = NULL;}
    rec = new BCoupon(figi);
}
void APICouponPage::slotGetCouponInfoByTicker(const QString &ticker, QDate &d, float &size)
{
    d = QDate();
    size = -1;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    for (int i=0; i<n; i++)
    {
        if (ticker == t->item(i, FIGI_COL+1)->text().trimmed())
        {
            QString figi = t->item(i, FIGI_COL)->text().trimmed();
            const BCoupon *c_rec = NULL;
            slotGetCouponRec(figi, c_rec);
            if (c_rec) {d = c_rec->date; size = c_rec->size;}
            break;
        }
    }
}
void APICouponPage::slotGetCouponRec(const QString &figi, const BCoupon* &c_rec)
{
    foreach (const CouponRecordAbstract *rec, m_data)
    {
        if (rec->figi == figi)
        {c_rec = static_cast<const BCoupon*>(rec); break;}
    }
}
QString APICouponPage::dataFile() const
{
    return CycleWorker::couponsFile();
}
void APICouponPage::addRowRecord(const CouponRecordAbstract *rec, const QPair<QString, QString> &b_info, const QString &ticker)
{
    const BCoupon *b_rec = static_cast<const BCoupon*>(rec);
    if (!b_rec) {qWarning("APICouponPage::addRowRecord WARNING invalid convert CouponRecordAbstract* to BCoupon*"); return;}

    QStringList row_data;
    row_data << b_info.first << rec->figi << ticker << b_info.second << QString::number(b_rec->period);
    row_data << rec->date.toString(InstrumentBase::userDateMask()) << QString::number(rec->size, 'f', 2);
    row_data << QString::number(b_rec->daySize(), 'f', 4) << QString::number(rec->daysTo());
    LTable::addTableRow(m_tableBox->table(), row_data);

    int l_row = m_tableBox->table()->rowCount() - 1;
    if (b_rec->daySize() > DAY_SIZE_LIMIT_HIGH) m_tableBox->table()->item(l_row, DAY_SIZE_COL)->setTextColor(Qt::blue);
    else if (b_rec->daySize() < DAY_SIZE_LIMIT_LOW) m_tableBox->table()->item(l_row, DAY_SIZE_COL)->setTextColor(Qt::lightGray);
}



//APIDivPage
APIDivPage::APIDivPage(QWidget *parent)
    :APICouponPageAbstract(parent)
{
    setObjectName("api_div_page");
    m_userSign = aptDiv;

    QStringList headers;
    headers << "Company" << "FIGI" << "Ticker" << "Currency" << "Date" << "Size" << "Last price" << "Days to";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Calendar");
}
void APIDivPage::getBagContains()
{
    QStringList list;
    emit signalGetBagStocks(list);
    if (list.isEmpty()) {emit signalMsg("Bag has not stocks"); return;}

    QTableWidget *t = m_tableBox->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        QString ticker = t->item(i, FIGI_COL+1)->text().trimmed();
        if (list.contains(ticker))
            LTable::setTableRowColor(t, i, "#98FB98");
    }
}
QString APIDivPage::dataFile() const
{
    return CycleWorker::divsFile();
}
void APIDivPage::createRecord(CouponRecordAbstract* &rec, const QString &figi)
{
    if (rec) {delete rec; rec = NULL;}
    rec = new SDiv(figi);
}
void APIDivPage::addRowRecord(const CouponRecordAbstract *rec, const QPair<QString, QString> &s_info, const QString &ticker)
{
    const SDiv *d_rec = static_cast<const SDiv*>(rec);
    if (!d_rec) {qWarning("APICouponPage::addRowRecord WARNING invalid convert CouponRecordAbstract* to SDiv*"); return;}

    //calc precisions
    int p_size = (d_rec->size < 10) ? 2 : 1;
    int p_yield = (d_rec->yield < 1) ? 3 : 2;


    QStringList row_data;
    row_data << s_info.first << rec->figi << ticker << s_info.second;    
    row_data << rec->date.toString(InstrumentBase::userDateMask());
    row_data << QString("%1 (%2%)").arg(QString::number(rec->size, 'f', p_size)).arg(QString::number(d_rec->yield, 'f', p_yield));
    if (d_rec->last_price > 1000) p_size = 0;
    row_data << QString::number(d_rec->last_price, 'f', p_size) << QString::number(rec->daysTo());
    LTable::addTableRow(m_tableBox->table(), row_data);

    int l_row = m_tableBox->table()->rowCount() - 1;
    if (d_rec->yield > DIV_YIELD_LIMIT_HIGH) m_tableBox->table()->item(l_row, DIV_SIZE_COL)->setTextColor(Qt::blue);
    else if (d_rec->yield < DIV_YIELD_LIMIT_LOW) m_tableBox->table()->item(l_row, DIV_SIZE_COL)->setTextColor(Qt::lightGray);
}




//APICouponPageAbstract
APICouponPageAbstract::APICouponPageAbstract(QWidget *parent)
    :APITablePageBase(parent)
{

}
void APICouponPageAbstract::clearData()
{
    qDeleteAll(m_data);
    m_data.clear();
}
void APICouponPageAbstract::loadData()
{
    qDebug()<<QString("APICouponPageAbstract::loadData()  page[%1]").arg(objectName());
    clearData();
    emit signalMsg(QString("OPEN FILE: %1").arg(dataFile()));

    QDomNode root_node;
    QString err = LStaticXML::getDomRootNode(dataFile(), root_node, QString("calendar"));
    if (!err.isEmpty()) {emit signalError(err); return;}

    qDebug() << QString("APICouponPageAbstract::loadData(%0): loaded root node OK - %1").arg(objectName()).arg(root_node.nodeName());
    QDomNode node = root_node.firstChild();
    while (!node.isNull())
    {
        loadFigiRecord(node);
        node = node.nextSibling();
    }
    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_data.count()));

    sortByDate();
    reloadTableByData();
}
void APICouponPageAbstract::loadFigiRecord(const QDomNode &figi_node)
{
    if (figi_node.isNull()) return;

    QString figi = figi_node.nodeName();
    //qDebug()<<QString("APICouponPageAbstract::loadFigiRecord %1").arg(figi);
    QDomNode node = figi_node.firstChild();
    QDate cur_d = QDate::currentDate();
    while (!node.isNull())
    {
        CouponRecordAbstract *rec = NULL;
        createRecord(rec, figi);
        if (!rec) {qWarning("APICouponPageAbstract::loadFigiRecord WARNING did not create record"); return;}

        rec->fromNode(node);
        if (!rec->invalid())
        {
            if (rec->date > cur_d)
            {
                m_data.append(rec);
                break;
            }
        }
        else qWarning()<<QString("APICouponPageAbstract::loadFigiCoupons - WARNING coupon record is invalid, %1").arg(rec->toString());
        node = node.nextSibling();
    }
}
void APICouponPageAbstract::reloadTableByData()
{
    resetPage();
    if (m_data.isEmpty())
    {
        emit signalMsg(QString("Coupons data is empty!"));
        return;
    }

    foreach (const CouponRecordAbstract *rec, m_data)
    {
        if (rec->invalid()) continue;
        QPair<QString, QString> pair;
        emit signalGetPaperInfoByFigi(rec->figi, pair);

        QString ticker;
        emit signalGetTickerByFigi(rec->figi, ticker);

        addRowRecord(rec, pair, ticker);
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#D3D3D3", "#2F4F4F");
    m_tableBox->searchExec();
}
void APICouponPageAbstract::slotFilter(const QStringList &list)
{
    qDebug()<<QString("APICouponPage::slotFilter figi list size %1").arg(list.count());
    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    for (int i=n-1; i>=0; i--)
    {
        QString figi = t->item(i, FIGI_COL)->text().trimmed();
        if (!list.contains(figi)) t->removeRow(i);
    }
    m_tableBox->searchExec();
}
void APICouponPageAbstract::sortByDate()
{
    if (m_data.count() < 3) return;

    int n = m_data.count();
    while (1 == 1)
    {
        bool has_replace = false;
        for (int i=0; i<n-1; i++)
        {
            const QDate &d = m_data.at(i)->date;
            const QDate &d_next = m_data.at(i+1)->date;
            if (!d.isValid() && d_next.isValid())
            {
                CouponRecordAbstract *rec = m_data.takeAt(i);
                m_data.insert(i+1, rec);
                has_replace = true;
            }
            else if (d.isValid() && d_next.isValid())
            {
                if (d_next < d)
                {
                    CouponRecordAbstract *rec = m_data.takeAt(i);
                    m_data.insert(i+1, rec);
                    has_replace = true;
                }
            }
        }
        if (!has_replace) break;
    }
}

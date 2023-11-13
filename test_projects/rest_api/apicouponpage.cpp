#include "apicouponpage.h"
#include "cycleworker.h"
#include "lstaticxml.h"
#include "ltable.h"


#include <QDebug>
#include <QDomNode>
#include <QTableWidget>

//APICouponPage
APICouponPage::APICouponPage(QWidget *parent)
    :APITablePageBase(parent)
{
    setObjectName("api_coupon_page");
    m_userSign = aptCoupon;


    QStringList headers;
    headers << "Company" << "FIGI" << "Currency" << "Period" << "Date" << "Size" << "Day size" << "Days to";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Coupon list");


}
void APICouponPage::loadData()
{
    m_data.clear();
    emit signalMsg(QString("OPEN FILE: %1").arg(CycleWorker::couponsFile()));

    QDomNode root_node;
    QString err = LStaticXML::getDomRootNode(CycleWorker::couponsFile(), root_node, QString("calendar"));
    if (!err.isEmpty()) {emit signalError(err); return;}

    qDebug() << QString("APICouponPage::loadData(): loaded root node OK - %1").arg(root_node.nodeName());
    QDomNode node = root_node.firstChild();
    while (!node.isNull())
    {
        loadFigiCoupons(node);
        node = node.nextSibling();
    }
    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_data.count()));

    reloadTableByData();
}
void APICouponPage::loadFigiCoupons(const QDomNode &figi_node)
{
    if (figi_node.isNull()) return;
    QString figi = figi_node.nodeName();
    QDomNode node = figi_node.firstChild();
    QDate cur_d = QDate::currentDate();
    while (!node.isNull())
    {
        BCoupon rec(figi);
        rec.fromNode(node);
        if (!rec.invalid())
        {
            if (rec.pay_date > cur_d)
            {
                m_data.append(rec);
                break;
            }
        }
        else qWarning()<<QString("APICouponPage::loadFigiCoupons - WARNING coupon record is invalid, %1").arg(rec.toString());

        node = node.nextSibling();
    }
}
void APICouponPage::reloadTableByData()
{
    resetPage();
    if (m_data.isEmpty())
    {
        emit signalMsg(QString("Coupons data is empty!"));
        return;
    }

    foreach (const BCoupon &rec, m_data)
    {
        if (rec.invalid()) continue;
        QStringList row_data;
        QPair<QString, QString> pair;
        emit signalGetPaperInfoByFigi(rec.figi, pair);

        row_data << pair.first << rec.figi << pair.second << QString::number(rec.period);
        row_data << rec.pay_date.toString(InstrumentBase::userDateMask()) << QString::number(rec.pay_size, 'f', 2);
        row_data << QString::number(rec.daySize(), 'f', 4) << QString::number(rec.daysTo());
        LTable::addTableRow(m_tableBox->table(), row_data);
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#E6E6FA", "#800000");
    m_tableBox->searchExec();
}

//APIDivPage
APIDivPage::APIDivPage(QWidget *parent)
    :APITablePageBase(parent)
{
    setObjectName("api_div_page");
    m_userSign = aptDiv;


    QStringList headers;
    headers << "Company" << "Ticker" << "Country" << "Currency" << "Date" << "Size" << "Days to";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Calendar");

}

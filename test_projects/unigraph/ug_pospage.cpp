#include "ug_pospage.h"
#include "subcommonsettings.h"
#include "ug_apistruct.h"
#include "ltable.h"
#include "lmath.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>


//UG_PosPage
UG_PosPage::UG_PosPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtPositions),
    m_tableBox(NULL)
{
    setObjectName("ug_pos_page");

    initTable();

    //LBigInt bi("39434925698191646345954912238072");
    quint16 deg = 128;
    LBigInt bi1(deg);
    //LBigInt bi2("90150012");
    LBigInt bi3("150000000012");

    bi1.toDebug();
   // bi2.toDebug();
    bi3.toDebug();

    //if (bi1.isLarger(bi2)) qDebug("bi1 > bi2");
    //if (bi1.isSmaller(bi2)) qDebug("bi1 < bi2");
    //if (bi1.isEqual(bi2)) qDebug("bi1 == bi2");
    /*
    bi2.increase(bi1);
    qDebug("result after bi2.increase(bi1) : ");
    bi1.decrease(bi3);
    qDebug("result after bi1.decrease(bi3) : ");
    */


    //qDebug()<<QString("final_value_b2 = %1").arg(bi2.finalValue());
    //bi1.multiply(bi3);
    //bi1.toDebug();
    //qDebug()<<QString("b1*b3: final_value_b1 = %1").arg(bi1.finalValue());




}
void UG_PosPage::updateDataPage(bool forcibly)
{
    qDebug("UG_PosPage::updateDataPage");
    qDebug()<<QString("forcibly: %1").arg(forcibly?"true":"false");
    if (!forcibly) return;
}
void UG_PosPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) {qDebug("req_type != userSign()"); return;}
    qDebug()<<QString("UG_PosPage::slotJsonReply  req_type=%1, OK!").arg(req_type);

    emit signalMsg("Updating pos list finished!");
    emit signalStopUpdating();

    const QJsonValue &j_data = j_obj.value("data");
    if (j_data.isNull()) {emit signalError("UG_PosPage: result QJsonValue <data> not found"); return;}
    const QJsonArray &j_arr = j_data.toObject().value("positions").toArray();
    if (j_arr.isEmpty()) {emit signalError("UG_PosPage: positions QJsonArray is empty"); return;}

    parseJArrPos(j_arr);
    updateTableData();

}
void UG_PosPage::parseJArrPos(const QJsonArray &j_arr)
{
    qDebug()<<QString("j_arr POSITIONS %1").arg(j_arr.count());
    for (int i=0; i<j_arr.count(); i++)
    {
        UG_PosInfo pos(sub_commonSettings.curChain());
        pos.fromJson(j_arr.at(i).toObject());

        qDebug()<<pos.toStr();
        if (!pos.invalid())
        {
            m_positions.append(pos);
            if (!pos.isClosed()) qDebug()<<pos.toStrFeeGrowth();

        }
        else qWarning("WARNING: INVALID POS DATA!!!");
    }

}
void UG_PosPage::slotReqBuzyNow()
{
    if (updatingRunning())
    {
        //m_skip -= m_reqLimit;
    }
}
void UG_PosPage::initTable()
{
    m_tableBox = new LSearchTableWidgetBox(this);

    QStringList headers;
    headers << "ID" << "Creation time" << "Pool params" << "Chain" << "TVL" << "Deposited" // << "Collected"
            << "Withdrawn" << "Collected fees" << "Tick range" << "Range" << "Unclaimed fees";

    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Positions LP");
    m_tableBox->setObjectName("table_box");
    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    v_splitter->addWidget(m_tableBox);

}
void UG_PosPage::prepareQuery()
{
    QString w = sub_commonSettings.wallet.trimmed();
    if (w.isEmpty())
    {
        emit signalError("You must enter wallet address.");
        emit signalStopUpdating();
        return;
    }

    QString query = QString("positions(first: %1, where:{owner: \"%2\"})").arg(m_reqLimit).arg(w);
    QString s_fields = ("id depositedToken0 depositedToken1 withdrawnToken0 withdrawnToken1 collectedFeesToken0 collectedFeesToken1");
    s_fields = QString("%1 liquidity feeGrowthInside0LastX128 feeGrowthInside1LastX128").arg(s_fields);
    s_fields = QString("%1 tickLower{tickIdx feeGrowthOutside0X128 feeGrowthOutside1X128}").arg(s_fields);
    s_fields = QString("%1 tickUpper{tickIdx feeGrowthOutside0X128 feeGrowthOutside1X128}").arg(s_fields);

    s_fields = QString("%1 pool{tick feeGrowthGlobal0X128 feeGrowthGlobal1X128 token0Price token1Price token0{symbol} token1{symbol} feeTier totalValueLockedUSD}").arg(s_fields);
    //s_fields = QString("%1 transaction{timestamp mints(first: 10) {timestamp tickLower tickUpper amount0 amount1} }").arg(s_fields);
    s_fields = QString("%1 transaction{timestamp}").arg(s_fields);

    query = QString("%1 { %2 }").arg(query).arg(s_fields);
    m_reqData->query = QString("{ %1 }").arg(query);

}
void UG_PosPage::startUpdating(quint16 t)
{
    qDebug("UG_PosPage::startUpdating");
    UG_BasePage::startUpdating(t);
    clearPage();
    emit signalGetReqLimit(m_reqLimit);
    prepareQuery();

    emit signalMsg("try next query ......");
    sendRequest();
}
void UG_PosPage::updateTableData()
{
    if (m_positions.isEmpty())
    {
        emit signalError("Readed positions: 0");
        return;
    }

    sortPositions();

    QTableWidget *tw = m_tableBox->table();
    foreach (const UG_PosInfo &pos, m_positions)
    {
        int last_row = tw->rowCount();

        QStringList row_data;
        pos.toTableRow(row_data);
        LTable::addTableRow(tw, row_data);

        if (pos.isClosed())
        {
            LTable::setTableRowColor(tw, last_row, Qt::lightGray);
        }
        else if(pos.isOut())
        {
            LTable::setTableRowColor(tw, last_row, "#ff8800");
        }
        else LTable::setTableRowColor(tw, last_row, "#ccffcc");
    }
    m_tableBox->searchExec();
    m_tableBox->resizeByContents();
}
void UG_PosPage::sortPositions()
{
    if (m_positions.count() < 3) return;

    //look opened pos
    QList<int> i_opened(openedIndexes());
    if (!i_opened.isEmpty())
    {
        for (int i=0; i<i_opened.count(); i++)
        {
            UG_PosInfo pos(m_positions.takeAt(i_opened.at(i)));
            m_positions.insert(0, pos);
        }
    }

    // sort closed pos
    int n = m_positions.count();
    bool b = true;
    while (b)
    {
        b = false;
        for (int i=i_opened.count()+1; i<n; i++)
        {
            if (m_positions.at(i).ts > m_positions.at(i-1).ts)
            {
                UG_PosInfo pos(m_positions.takeAt(i));
                m_positions.insert(i-1, pos);
                b = true;
                break;
            }
        }
    }
}
QList<int> UG_PosPage::openedIndexes() const
{
    QList<int> list;
    for (int i=0; i<m_positions.count(); i++)
        if (!m_positions.at(i).isClosed()) list.append(i);
    return list;
}
void UG_PosPage::clearPage()
{
    m_positions.clear();
    m_tableBox->removeAllRows();
    m_tableBox->resizeByContents();
}





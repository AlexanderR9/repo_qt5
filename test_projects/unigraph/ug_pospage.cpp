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

#define ASSETS_COL  6



//UG_PosPage
UG_PosPage::UG_PosPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtPositions),
    m_tableBox(NULL)
{
    setObjectName("ug_pos_page");

    initTable();
    m_tableBox->resizeByContents();


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

        //qDebug()<<pos.toStr();
        if (!pos.invalid())
        {
            m_positions.append(pos);
            if (!pos.isClosed())
            {
                qDebug()<<QString("assets: %1").arg(pos.curAssets());
            }

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

    m_tableBox->vHeaderHide();
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

    s_fields = QString("%1 pool{id tick sqrtPrice feeGrowthGlobal0X128 feeGrowthGlobal1X128 token0Price token1Price token0{symbol} token1{symbol} feeTier totalValueLockedUSD}").arg(s_fields);
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
        QStringList row_data;
        pos.toTableRow(row_data);
        LTable::addTableRow(tw, row_data);
        updateLastRowColor(pos);
    }
    m_tableBox->searchExec();
    m_tableBox->resizeByContents();
}
void UG_PosPage::updateLastRowColor(const UG_PosInfo &pos)
{
    QTableWidget *tw = m_tableBox->table();
    int last_row = tw->rowCount()-1;
    if (last_row < 0) return;
    tw->item(last_row, 2)->setToolTip(pos.pool.id);

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



///////////////////UG_ActivePosPage///////////////////////////////////
UG_ActivePosPage::UG_ActivePosPage(QWidget *parent)
    :UG_PosPage(parent),
      m_chainIndex(-1)
{
    m_userSign = rtPositionsAct;
    m_reqData->req_type = m_userSign;
    setObjectName("ug_pos_act_page");

    initTable();
    m_tableBox->resizeByContents();

    /*
    LBigInt Q96(96);
    LBigInt psqrt("68401484055501712117026");
    LBigInt liq("2591590707903345");
    //qDebug("----");
    Q96.toDebug("Q96");
    psqrt.toDebug("sqrtPriceX96");
    qDebug("----");
    psqrt.division(Q96);
    psqrt.toDebug("p_cur");
*/


}
void UG_ActivePosPage::initTable()
{
    //m_tableBox = new LSearchTableWidgetBox(this);
    LTable::fullClearTable(m_tableBox->table());

    QStringList headers;
    headers << "ID" << "Age" << "Pool params" << "Chain" << "TVL" << "Deposited" << "Assets" << "Price Range";// << "Unclaimed fees";

    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Active positions");
}
void UG_ActivePosPage::startUpdating(quint16 t)
{
    qDebug("UG_ActivePosPage::startUpdating");
    UG_BasePage::startUpdating(t);
    clearPage();
    emit signalGetReqLimit(m_reqLimit);
    m_chainIndex = 0;
    wait_data = false;
}
void UG_ActivePosPage::slotTimer()
{
    UG_BasePage::slotTimer();
    if (wait_data) return;

    nextChainReq();
}
void UG_ActivePosPage::updateLastRowColor(const UG_PosInfo &pos)
{
    QTableWidget *tw = m_tableBox->table();
    int last_row = tw->rowCount()-1;
    if (last_row < 0) return;

    if(pos.isOut())
        LTable::setTableRowColor(tw, last_row, "#ffeecc");
}
void UG_ActivePosPage::nextChainReq()
{
    if (m_chainIndex < sub_commonSettings.factories.count())
    {
        emit signalMsg("try next query ......");
        emit signalChangeSubGraph(sub_commonSettings.factories.at(m_chainIndex).chain);
        wait_data = true;

        prepareQuery();
        sendRequest();
    }
    else
    {
        emit signalMsg("Updating active positions finished!");
        emit signalStopUpdating();
        updateTableData();
    }
}
void UG_ActivePosPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) {qDebug("req_type != userSign()"); return;}
    qDebug()<<QString("UG_ActivePosPage::slotJsonReply  req_type=%1, OK!").arg(req_type);

    const QJsonValue &j_data = j_obj.value("data");
    if (j_data.isNull()) {emit signalError("UG_PosPage: result QJsonValue <data> not found"); return;}
    const QJsonArray &j_arr = j_data.toObject().value("positions").toArray();
    if (j_arr.isEmpty()) {emit signalError("UG_PosPage: positions QJsonArray is empty"); return;}

    parseJArrPos(j_arr);
    m_chainIndex++;
    wait_data = false;
}
void UG_ActivePosPage::slotReqBuzyNow()
{
    //emit signalError("UG_ActivePosPage::slotReqBuzyNow()");
    //if (updatingRunning()) m_chainIndex--;
}
void UG_ActivePosPage::updateTableData()
{
    if (m_positions.isEmpty()) return;
    sortPositions();

    QTableWidget *tw = m_tableBox->table();
    foreach (const UG_PosInfo &pos, m_positions)
    {
        QStringList row_data;
        pos.toTableRow_act(row_data);
        LTable::addTableRow(tw, row_data);
        calcStatbleAssetsLastRow(pos);
        updateLastRowColor(pos);
    }
    m_tableBox->searchExec();
    m_tableBox->resizeByContents();
}
void UG_ActivePosPage::calcStatbleAssetsLastRow(const UG_PosInfo &pos)
{
    double p_cur = pos.byStablePrice();
    if (p_cur<0) return;

    double ast0 = pos.curAsset0();
    double ast1 = pos.curAsset1();

    double ast_stable = 0;
    if (pos.pool.token0.trimmed().toLower().contains("usd"))
    {
        ast_stable = ast0 + ast1*p_cur;
    }
    else
    {
        ast_stable = ast0*p_cur + ast1;
    }

    //update cell
    QTableWidget *tw = m_tableBox->table();
    int last_row = tw->rowCount()-1;
    QString s(tw->item(last_row, ASSETS_COL)->text());
    s = QString("%1 (%2 ST)").arg(s).arg(QString::number(ast_stable, 'f', 1));
    tw->item(last_row, ASSETS_COL)->setText(s);
}
void UG_ActivePosPage::sortPositions()
{
    int n = m_positions.count();
    for (int i=n-1; i>=0; i--)
        if (m_positions.at(i).isClosed()) m_positions.removeAt(i);
}

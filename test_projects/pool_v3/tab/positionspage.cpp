#include "positionspage.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "nodejsbridge.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

/*
#define ADDRESS_COL             2
#define PAIR_COL                0
#define FEE_COL                 1
#define TICK_COL                3
#define PRICE_COL               4
#define TVL_COL                 5
*/


// DefiPositionsPage
DefiPositionsPage::DefiPositionsPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkPositions)
{
    setObjectName("defi_poositions_tab_page");

    //init table
    initTable();

    // init popup
    //initPopupMenu();
}
void DefiPositionsPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Wallet positions list");
    m_table->vHeaderHide();

    QStringList headers;
    headers << "PID" << "Pool" << "Price range" << "Current price" << "Current assets" << "Rewards" << "Liquidity";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");
    h_splitter->addWidget(m_table);

    /////////////////////////////////////////

    m_integratedTable = new LTableWidgetBox(this);
    m_integratedTable->setTitle("Integrated state");
    m_integratedTable->setObjectName("positions_integrated_table");
    headers.clear();
    headers << "Value";
    LTable::setTableHeaders(m_integratedTable->table(), headers);
    headers.clear();
    headers << "Total positions" << "Active positions" << "Minted positions" <<  "Burned positions" << "Locked sum" << "Rewards sum";
    LTable::setTableHeaders(m_integratedTable->table(), headers, Qt::Vertical);
    m_integratedTable->setSelectionMode(QAbstractItemView::NoSelection, QAbstractItemView::NoSelection);
    LTable::createAllItems(m_integratedTable->table());
    h_splitter->addWidget(m_integratedTable);

}
void DefiPositionsPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);
   // initPoolList(cid);

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();

}
void DefiPositionsPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiPositionsPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcPositions)) updatePositionsData(js_reply);
    //else if (req == NodejsBridge::jsonCommandValue(txSwap)) checkTxResult(req, js_reply);
    else return;

    m_table->resizeByContents();

}
void DefiPositionsPage::sendUpdateDataRequest()
{
    qDebug("DefiPositionsPage::sendUpdateDataRequest()");
    //prepare json params
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPositions));
    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::updatePositionsData(const QJsonObject &js_reply)
{
    qDebug("--DefiPositionsPage::updatePositionsData--");
    bool result = false;
    if (js_reply.contains("result")) result = (js_reply.value("result").toString().trimmed().toLower() == "true");
    int n_pos = js_reply.value("pos_count").toString().trimmed().toInt();

    QString s = QString("Getting positions data result: %1").arg(result ? "OK" : "FAULT");
    if (!result) emit signalError(s);
    else
    {
        emit signalMsg(s);
        emit signalMsg(QString("Current positions: %1").arg(n_pos));
        if (n_pos > 0)
        {
            //need read posdata file
        }
        else emit signalError("positions data is empty");
    }
}




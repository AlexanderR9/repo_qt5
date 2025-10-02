#include "positionspage.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "lfile.h"
#include "deficonfig.h"



#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>


#define POOL_COL             1
#define RANGE_COL            2
#define PRICE_COL            3
#define ASSETS_COL           4
#define REWARDS_COL          5
#define LIQ_COL              6


// DefiPositionsPage
DefiPositionsPage::DefiPositionsPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkPositions)
{
    setObjectName("defi_poositions_tab_page");
    m_positions.clear();

    //init table
    initTable();

    // init popup
    initPopupMenu();


}
void DefiPositionsPage::initPopupMenu()
{
    QString path = AppCommonSettings::commonIconsPath(); // icons path


    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state (selected)", QString("%1/edit-redo.svg").arg(path));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Get state (with liquidity)", QString("%1/edit-undo.svg").arg(path));
    act_list.append(pair2);
//    QPair<QString, QString> pair2("Swap tokens", TxDialogBase::iconByTXType(txSwap));
  //  act_list.append(pair2);


    //init popup menu actions
    m_table->popupMenuActivate(act_list);
    m_table->setMaxPopupSelectRows(8);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetSelectedPosState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetLiquidityPosState())); i_menu++;
    //m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTxSwap())); i_menu++;


}
void DefiPositionsPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Wallet positions list");
    m_table->vHeaderHide();

    QStringList headers;
    headers << "PID" << "Pool" << "Price range" << "Current price" << "Current assets" << "Rewards" << "Liquidity";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
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
    headers << "Total positions" << "With liquidity" << "Minted positions" <<  "Burned positions" << "Locked sum" << "Rewards sum";
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
    else if (req == NodejsBridge::jsonCommandValue(nrcPosState)) updatePositionsState(js_reply);
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
void DefiPositionsPage::updatePositionsState(const QJsonObject &js_reply)
{
    qDebug("--DefiPositionsPage::updatePositionsState--");
    bool ok;
    QStringList keys = js_reply.keys();
    qDebug("received keys:");
    foreach (const QString &v, keys)
    {
         qDebug() << v;
         int pid = v.toInt(&ok);
         if (!ok || pid <= 0) continue;

         int i_pos = posIndexOf(pid);
         if (i_pos >= 0) updatePositionState(i_pos, js_reply.value(v));
    }        
}
void DefiPositionsPage::updatePositionState(int i, const QJsonValue &js_value)
{
    if (i<0 || i>=posCount()) return;
    if (!js_value.isObject()) return;

    QJsonObject j_obj = js_value.toObject();
    if (!j_obj.isEmpty()) m_positions[i].state.update(j_obj);

    //update row table
    QTableWidget *t = m_table->table();
    t->item(i, RANGE_COL)->setText(m_positions[i].interfacePriceRange());
    t->item(i, PRICE_COL)->setText(m_positions[i].interfaceCurrentPrice());
    t->item(i, ASSETS_COL)->setText(m_positions[i].interfaceAssetAmounts());
    t->item(i, REWARDS_COL)->setText(m_positions[i].interfaceRewards());

    t->item(i, PRICE_COL)->setToolTip(QString("tick: %1").arg(m_positions[i].state.pool_tick));
}
void DefiPositionsPage::updatePositionsData(const QJsonObject &js_reply)
{
    m_positions.clear();
    m_table->removeAllRows();

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
            readNodejsPosFile();
            reloadTableByRecords();
        }
        else emit signalError("positions data is empty");
    }
}
void DefiPositionsPage::readNodejsPosFile()
{
    QString fname = QString("%1/%2").arg(AppCommonSettings::nodejsPath()).arg(AppCommonSettings::positionsListFile());
    if (!LFile::fileExists(fname))
    {
        emit signalError(QString("DefiPositionsPage: file [%1] not found").arg(fname));
        return;
    }

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty())
    {
        emit signalError(QString("DefiPositionsPage: %1").arg(err));
        return;
    }
    if (fdata.isEmpty())
    {
        emit signalError(QString("DefiPositionsPage: positions data is empty"));
        return;
    }

    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;

        DefiPosition   rec;
        rec.fromFileLine(fline);
        if (!rec.invalid())
        {
            if (rec.hasLiquidity()) m_positions.insert(0, rec);
            else m_positions.append(rec);
            qDebug()<<rec.toStr();
        }
        else qWarning()<<QString("DefiPositionsPage: WARNING invalid fline_pos: %1").arg(fline);
    }
}
void DefiPositionsPage::reloadTableByRecords()
{
    qDebug()<<QString("DefiPositionsPage::reloadTableByRecords()  pos_count=%1").arg(posCount());
    if (!hasPos()) return;

    QTableWidget *t = m_table->table();
    QStringList row_data;
    int n = posCount();
    for (int i=0; i<n; i++)
    {
        const DefiPosition &rec = m_positions.at(i);

        row_data.clear();
        row_data << QString::number(rec.pid) << poolInfo(rec);
        row_data << QString("-") << QString("-") << QString("-") << QString("-") << rec.liq;
        LTable::addTableRow(t, row_data);
        t->item(i, 0)->setTextColor(Qt::darkGreen);

        if (!rec.hasLiquidity())
            LTable::setTableTextRowColor(t, i, Qt::lightGray);

        int pool_i = defi_config.getPoolIndexByPosition(rec);
        if (pool_i >= 0) t->item(i, POOL_COL)->setToolTip(defi_config.pools.at(pool_i).address);
        else t->item(i, POOL_COL)->setToolTip("WARNING: pool not found");

        t->item(i, RANGE_COL)->setToolTip(rec.interfaceTickRange());
    }

    m_table->resizeByContents();
}
QString DefiPositionsPage::poolInfo(const DefiPosition &rec) const
{
    if (rec.invalid()) return "???";

    int cid = defi_config.getChainID(curChainName());
    if (cid < 0) return "???";

    QString s = QString("%1/%2").arg(defi_config.tokenNameByAddress(rec.token_addrs.first, cid)).arg(defi_config.tokenNameByAddress(rec.token_addrs.second, cid));
    s = QString("%1 (%2)").arg(s).arg(rec.interfaceFee());
    return s;
}
void DefiPositionsPage::slotGetSelectedPosState()
{
    qDebug("DefiPositionsPage::slotGetSelectedPosState()");
    QTableWidget *t = m_table->table();
    QList<int> list = LTable::selectedRows(t);
    if (list.isEmpty()) {emit signalError("DefiPositionsPage: You must select several positions"); return;}

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcPosState));
    QJsonArray j_arr_pid;
    QJsonArray j_arr_pool;
    foreach (int row, list)
    {
        QString s_pid = t->item(row, 0)->text().trimmed();
        j_arr_pid.push_back(s_pid);
        QString pool_addr = t->item(row, POOL_COL)->toolTip().trimmed();
        j_arr_pool.push_back(pool_addr);
    }
    j_params.insert("pid_arr", j_arr_pid);
    j_params.insert("pool_addresses", j_arr_pool);

    sendReadNodejsRequest(j_params);
}
void DefiPositionsPage::slotGetLiquidityPosState()
{
    qDebug("DefiPositionsPage::slotGetLiquidityPosState()");


}
int DefiPositionsPage::posIndexOf(int pid) const
{
    if (m_positions.isEmpty()) return -1;

    int n = posCount();
    for (int i=0; i<n; i++)
        if (m_positions.at(i).pid == pid) return i;

    return -1;
}



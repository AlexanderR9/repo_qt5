#include "jsposmanagertab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>

#define POOL_COL            1
#define P_RANGE_COL         2
#define T_RANGE_COL         3
#define STATE_COL           5
#define LIQ_COL             6
#define ASSETS_COL          4





// JSPosManagerTab
JSPosManagerTab::JSPosManagerTab(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_tablePos(NULL),
      m_tableLog(NULL)
{
    setObjectName("js_posmanager_tab");

    //init tables
    initTables();

    // init context menu
    initPopupMenu();

}
void JSPosManagerTab::initTables()
{
    m_tablePos = new LSearchTableWidgetBox(this);
    m_tablePos->setTitle("Positions list");
    m_tablePos->vHeaderHide();
    QStringList headers;
    headers << "PID" << "Pool" << "Price range" << "Tick range" << "Assets amount" << "State" << "Liquidity";
    LTable::setTableHeaders(m_tablePos->table(), headers);
    m_tablePos->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_tablePos->setSelectionColor("#87CEEB");
    h_splitter->addWidget(m_tablePos);
    m_tablePos->searchExec();

    m_tableLog = new LSearchTableWidgetBox(this);
    m_tableLog->setTitle("Pool log (closed)");
    m_tableLog->vHeaderHide();
    headers.clear();
    headers << "Pool" << "Opened Date/Time" << "Closed Date/Time" << "PID" << "Price range" << "Deposited" << "Claimed" << "Rewards";
    LTable::setTableHeaders(m_tableLog->table(), headers);
    m_tableLog->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_tableLog->setSelectionColor("#87CEEB");
    h_splitter->addWidget(m_tableLog);
    m_tableLog->searchExec();

}
void JSPosManagerTab::updatePidList()
{
    QStringList params;
    m_tableLog->removeAllRows();
    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);

    params << "qt_pidlist.js";
    emit signalPosManagerAction(params);
}
void JSPosManagerTab::parseJSResult(const QJsonObject &j_result)
{
    qDebug()<<LString::symbolString('-');
    qDebug("JSPosManagerTab::parseJSResult");
    qDebug() << j_result;

    m_tablePos->setEnabled(true);
    m_tableLog->setEnabled(true);

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "pid_list")
    {
        jsonPidListReceived(j_result);
    }
    else if (operation == "pos_file_data")
    {
        jsonPosFileDataReceived(j_result);
    }
    else if (operation == "pos_state")
    {
        jsonPosStateReceived(j_result);
    }



    m_tablePos->searchExec();
    m_tableLog->searchExec();
}
void JSPosManagerTab::jsonPosStateReceived(const QJsonObject &j_result)
{
    qDebug("JSPosManagerTab::jsonPosStateReceived");
    int row = m_tablePos->curSelectedRow();
    QTableWidget *t = m_tablePos->table();

    t->item(row, ASSETS_COL)->setText(j_result.value("assets").toString());

    QString s_state = QString("unclaimed(%1)").arg(j_result.value("reward").toString());
    QString p_location = j_result.value("price_location").toString().toUpper().trimmed();
    s_state = QString("%1 P=%2(%3)").arg(s_state).arg(j_result.value("price_current").toString()).arg(p_location);
    t->item(row, STATE_COL)->setText(s_state);

    if (j_result.value("liq").toString().trimmed().length() > 4)
    {
        if (p_location.contains("OUT")) LTable::setTableRowColor(t, row, "#FFDCDC");
        else LTable::setTableRowColor(t, row, "#BAFBBA");
    }
}
void JSPosManagerTab::jsonPosFileDataReceived(const QJsonObject &j_result)
{
    qDebug("JSPosManagerTab::jsonPosFileDataReceived");
    QStringList keys(j_result.keys());

    QTableWidget *t = m_tablePos->table();
    int n_row = t->rowCount();
    if (n_row == 0) {emit signalError("table of positions is empty for sync pod_file_data"); return;}

    for (int i=0; i<n_row; i++)
    {
        QString pid = t->item(i, 0)->text().trimmed();
        if (!keys.contains(pid)) continue;

        QJsonValue jv = j_result.value(pid);
        QStringList pos_data = LString::trimSplitList(jv.toString().trimmed(), "/");
        if (pos_data.count() != 5) {qWarning()<<QString("invalid posdata fline [%1]").arg(jv.toString()); continue;}


        QStringList pool_info = LString::trimSplitList(pos_data.at(1), ";");
        if (pool_info.count() == 3)
            pos_data[1] = QString("%1/%2  (%3)").arg(pool_info.at(0).trimmed()).arg(pool_info.at(1).trimmed()).arg(pool_info.at(2).trimmed());
        t->item(i, POOL_COL)->setText(pos_data.at(1));
        t->item(i, POOL_COL)->setData(Qt::UserRole, pos_data.first());
        t->item(i, POOL_COL)->setToolTip(pos_data.first());

        t->item(i, P_RANGE_COL)->setText(pos_data[2].remove("price_range"));
        t->item(i, T_RANGE_COL)->setText(pos_data[3].remove("tick_range"));

        QString liq = pos_data.last().trimmed();
        t->item(i, LIQ_COL)->setText(liq);
        if (liq == '0') t->item(i, LIQ_COL)->setTextColor(Qt::lightGray);
        else t->item(i, LIQ_COL)->setTextColor(Qt::blue);

    }

}
void JSPosManagerTab::jsonPidListReceived(const QJsonObject &j_result)
{
    m_tablePos->removeAllRows();
    int pos_count = j_result.value("pos_count").toString().trimmed().toInt();
    emit signalMsg(QString("GETTED_POS_COUNT: %1").arg(pos_count));
    QJsonValue j_list = j_result.value("pids");
    if (!j_list.isArray()) {emit signalError("JSON field <pids> is not ARRAY"); return;}

    QJsonArray j_arr = j_list.toArray();
    emit signalMsg(QString("QJsonArray size: %1").arg(j_arr.size()));
    if (j_arr.size() == 0) {emit signalError("<pids> ARRAY is empty"); return;}
    reloadPidListToTable(j_arr);

    //next req, get pos_data from file
    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);
    QStringList params;
    params << "qt_show_posdata.js";
    emit signalPosManagerAction(params);
}
void JSPosManagerTab::reloadPidListToTable(const QJsonArray &j_arr)
{
    QTableWidget *t = m_tablePos->table();
    int n = j_arr.size();
    int n_cols = t->columnCount();
    qDebug()<<QString("JSPosManagerTab::reloadPidListToTable   j_arr.size() %1,  n_cols=%2").arg(n).arg(n_cols);

    for (int i=0; i<n; i++)
    {
        QStringList row_data;
        if (j_arr.at(i).isDouble())
        {
            double pid = j_arr.at(i).toDouble();
            row_data << QString::number(int(pid));
        }
        else if (j_arr.at(i).isString()) row_data << j_arr.at(i).toString();
        else row_data << "invalid_type";

        for (int j=0; j<(n_cols-1); j++) row_data << "-";
        LTable::addTableRow(t, row_data);
        t->item(i, 0)->setTextColor("#6B8E23");
    }
}
void JSPosManagerTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state", QString("%1/r_scale.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1);

    //QPair<QString, QString> pair2("Swap assets", TxDialogBase::iconByTXType(txSwap));
    //act_list.append(pair2);

    //init popup menu actions
    m_tablePos->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_tablePos->connectSlotToPopupAction(i_menu, this, SLOT(slotGetPositionState())); i_menu++;
    //m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTrySwapAssets())); i_menu++;

}
void JSPosManagerTab::slotGetPositionState()
{
   // qDebug("JSPoolTab::slotGetPoolState()");
    int row = m_tablePos->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QStringList params;
    m_tablePos->setEnabled(false);
    m_tableLog->setEnabled(false);

    params << "qt_posstate.js" << m_tablePos->table()->item(row, 0)->text().trimmed();
    emit signalPosManagerAction(params);
}



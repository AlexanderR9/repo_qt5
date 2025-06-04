#include "jsposhistorytab.h"
#include "ltable.h"
#include "lfile.h"
#include "ltime.h"
#include "lstring.h"
#include "subcommonsettings.h"
//#include "jstxdialog.h"
//#include "ethers_js.h"
#include "jstxlogger.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
/*
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
*/



// JSPosHistoryTab
JSPosHistoryTab::JSPosHistoryTab(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_table(NULL),
      m_txHistoryObj(NULL)
{
    setObjectName("js_poshistory_tab");

    //init tables
    initTable();

    // init context menu
    //initPopupMenu();

    m_txHistoryObj = new JSTxLogger(this);
    connect(m_txHistoryObj, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_txHistoryObj, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));

}
void JSPosHistoryTab::initTable()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setTitle("Positions history");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Pool" << "Opened time" << "Closed time" << "Position range" << "Open/close price" <<
               "Deposited" << "Decreased" << "Rewards" << "APR";

    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#87CEEB");
    h_splitter->addWidget(m_table);
    m_table->searchExec();

   // m_table->sortingOn();
   // m_table->addSortingData(LIQ_COL, LTableWidgetBox::sdtNumeric);
   // m_table->addSortingData(POOL_COL, LTableWidgetBox::sdtString);

}
void JSPosHistoryTab::loadTxLogFile(QString chain)
{
    m_txHistoryObj->setChainName(chain.trimmed().toLower());
    m_txHistoryObj->reloadLogFile();
    convertLogDataToStepData();
    reloadTableData();
}
void JSPosHistoryTab::convertLogDataToStepData()
{
    m_stepData.clear();
    if (m_txHistoryObj->logEmpty()) {emit signalError("JSPosHistoryTab: log_data is empty"); return;}

    bool tx_ok = false;
    int n = m_txHistoryObj->logSize();
    qDebug()<<QString("JSPosHistoryTab::convertLogDataToStepData() n=%1").arg(n);
    for (int i=0; i<n; i++)
    {
        const JSTxLogRecord& log_rec = m_txHistoryObj->recordAt(i);
        emit signalCheckTxResult(log_rec.tx_hash, tx_ok);
        if (!tx_ok) continue;

        if (log_rec.tx_kind == "mint" || log_rec.tx_kind == "increase")
        {
            qDebug()<<QString("find start point, pool [%1], TIME[%2]").arg(log_rec.pool_address).arg(LTime::strDateTime(log_rec.dt));
            addNewRecord(log_rec);
            continue;
        }


        if (log_rec.tx_kind == "decrease")
        {
            qDebug()<<QString("find decrease point, pool [%1]").arg(log_rec.pool_address);
        }
        if (log_rec.tx_kind == "collect")
        {
            qDebug()<<QString("find exit point, pool [%1], TIME[%2]").arg(log_rec.pool_address).arg(LTime::strDateTime(log_rec.dt));
        }


    }
}
void JSPosHistoryTab::addNewRecord(const JSTxLogRecord &log_rec)
{
    PosLineStep pos;
    pos.tx_kind = log_rec.tx_kind;
    log_rec.fromPriceRange(pos.pos_range.first, pos.pos_range.second);
    emit signalGetPoolInfo(log_rec.pool_address, pos.pool_info);

    //fill start state
    pos.start_state.current_price = log_rec.current_price;
    pos.start_state.dt = log_rec.dt;
    pos.start_state.token0_size = log_rec.token0_size;
    pos.start_state.token1_size = log_rec.token1_size;

    m_stepData.append(pos);
}
void JSPosHistoryTab::reloadTableData()
{
    m_table->removeAllRows();
    int n = m_txHistoryObj->logSize();
    qDebug()<<QString("try reloadTableData of JSPosHistoryTab, records %1").arg(n);

   // headers << "Pool" << "Opened time" << "Closed time" << "Position range" << "Open/close price" <<
    //           "Deposited" << "Decreased" << "Rewards" << "APR";

    QTableWidget *t = m_table->table();
    for (int i=0; i<m_stepData.count(); i++)
    {
        const PosLineStep &pos = m_stepData.at(i);
        QStringList row_data;
        row_data << pos.pool_info << pos.start_state.strTime() << pos.exit_state.strTime();
        row_data << pos.strPriceRange() << pos.strCurrentPrices();
        row_data << pos.strDeposited() << pos.strDecreased() << pos.strClaimedFees();
        row_data << QString("%1%").arg(QString::number(pos.calcAPR(), 'f', 1));
        LTable::addTableRow(t, row_data);
    }

    m_table->searchExec();
}




//////////////PosLineStep//////////////////////
void PosLineStep::reset()
{
    tx_kind.clear();
    pool_info.clear();
    pos_range.first = pos_range.second = -1;
    claimed_rewards.first = claimed_rewards.second = 0;
    start_state.reset();
    exit_state.reset();
}
float PosLineStep::calcAPR() const
{
    return 0;
}
QString PosLineStep::strPriceRange() const
{
    QString s = QString::number(pos_range.first, 'f', SubGraph_CommonSettings::interfacePrecision(pos_range.first));
    s = QString("[%1 : %2]").arg(s).arg(QString::number(pos_range.second, 'f', SubGraph_CommonSettings::interfacePrecision(pos_range.second)));
    return s;
}
QString PosLineStep::strCurrentPrices() const
{
    QString s = QString::number(start_state.current_price, 'f', SubGraph_CommonSettings::interfacePrecision(start_state.current_price));
    s = QString("%1/%2").arg(s).arg(QString::number(exit_state.current_price, 'f', SubGraph_CommonSettings::interfacePrecision(exit_state.current_price)));
    return s;
}
QString PosLineStep::strDeposited() const
{
    QString s = QString::number(start_state.token0_size, 'f', SubGraph_CommonSettings::interfacePrecision(start_state.token0_size));
    s = QString("%1/%2").arg(s).arg(QString::number(start_state.token1_size, 'f', SubGraph_CommonSettings::interfacePrecision(start_state.token1_size)));
    return s;
}
QString PosLineStep::strDecreased() const
{
    QString s = QString::number(exit_state.token0_size, 'f', SubGraph_CommonSettings::interfacePrecision(exit_state.token0_size));
    s = QString("%1/%2").arg(s).arg(QString::number(exit_state.token1_size, 'f', SubGraph_CommonSettings::interfacePrecision(exit_state.token1_size)));
    return s;
}
QString PosLineStep::strClaimedFees() const
{
    QString s = QString::number(claimed_rewards.first, 'f', SubGraph_CommonSettings::interfacePrecision(claimed_rewards.first));
    s = QString("%1/%2").arg(s).arg(QString::number(claimed_rewards.second, 'f', SubGraph_CommonSettings::interfacePrecision(claimed_rewards.second)));
    return s;
}





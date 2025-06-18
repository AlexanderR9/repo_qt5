#include "jsposhistorytab.h"
#include "jsposmanagertab.h"
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

#define OPEN_TIME_COL       1
#define DEPOSITED_COL       5
#define DECREASED_COL       6
#define REWARDS_COL         7
#define APR_COL             8
#define RANGE_COL           3
#define CP_COL              4
#define T_LIFE_COL          9



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
    m_table = new JSPosTableWidgetBox(this);
    m_table->setTitle("Positions history");
    m_table->setFilterCol(0);
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Pool" << "Opened time" << "Closed time" << "Position range" << "Open/close price" <<
               "Deposited" << "Decreased" << "Rewards" << "APR" << "Lasting";

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
    //qDebug()<<QString("JSPosHistoryTab::convertLogDataToStepData() n=%1").arg(n);
    for (int i=0; i<n; i++)
    {
        const JSTxLogRecord& log_rec = m_txHistoryObj->recordAt(i);
        emit signalCheckTxResult(log_rec.tx_hash, tx_ok);
        if (!tx_ok) continue;

        if (log_rec.tx_kind == "mint" || log_rec.tx_kind == "increase")
        {
            //qDebug()<<QString("find start point, pool [%1], TIME[%2]").arg(log_rec.pool_address).arg(LTime::strDateTime(log_rec.dt));
            addNewRecord(log_rec);
            continue;
        }


        if (log_rec.tx_kind == "decrease")
        {
            //qDebug()<<QString("find decrease point, pool [%1]").arg(log_rec.pool_address);
            decreaseRecord(log_rec);
        }
        else if (log_rec.tx_kind == "collect")
        {
            //qDebug()<<QString("find exit point, pool [%1], TIME[%2]").arg(log_rec.pool_address).arg(LTime::strDateTime(log_rec.dt));
            collectRecord(log_rec);
        }
    }
}
void JSPosHistoryTab::addNewRecord(const JSTxLogRecord &log_rec)
{
    PosLineStep pos;
    pos.tx_kind = log_rec.tx_kind;
    pos.pool_address = log_rec.pool_address.trimmed();
    pos.tick_range = log_rec.tick_range.trimmed();
    log_rec.fromPriceRange(pos.pos_range.first, pos.pos_range.second);
    emit signalGetPoolInfo(log_rec.pool_address, pos.pool_info);

    //fill start state
    pos.start_state.current_price = log_rec.current_price;
    pos.start_state.dt = log_rec.dt;
    pos.start_state.token0_size = log_rec.token0_size;
    pos.start_state.token1_size = log_rec.token1_size;

    m_stepData.append(pos);
}
void JSPosHistoryTab::decreaseRecord(const JSTxLogRecord &log_rec)
{
    if (m_stepData.isEmpty()) return;

    int n = m_stepData.count();
    for (int i=n-1; i>=0; i--)
    {
        if (m_stepData.at(i).pool_address == log_rec.pool_address.trimmed())
        {
            if (m_stepData.at(i).tick_range == log_rec.tick_range.trimmed())
            {
                if (!m_stepData.at(i).isClosed())
                {
                    m_stepData[i].exit_state.dt = log_rec.dt;
                    m_stepData[i].exit_state.current_price = log_rec.current_price;
                    m_stepData[i].exit_state.token0_size = log_rec.token0_size;
                    m_stepData[i].exit_state.token1_size = log_rec.token1_size;
                    m_stepData[i].wait_collect = true;
                    break;
                }
            }
        }
    }
}
void JSPosHistoryTab::collectRecord(const JSTxLogRecord &log_rec)
{
    if (m_stepData.isEmpty()) return;

    int n = m_stepData.count();
    for (int i=n-1; i>=0; i--)
    {
        if (m_stepData.at(i).pool_address == log_rec.pool_address.trimmed())
        {
            if (m_stepData.at(i).tick_range == log_rec.tick_range.trimmed())
            {
                if (m_stepData.at(i).wait_collect)
                {
                    m_stepData[i].claimed_rewards.first = log_rec.token0_size - m_stepData.at(i).exit_state.token0_size;
                    m_stepData[i].claimed_rewards.second = log_rec.token1_size - m_stepData.at(i).exit_state.token1_size;
                    m_stepData[i].wait_collect = false;
                    break;
                }
            }
        }
    }
}
void JSPosHistoryTab::reloadTableData()
{
    m_table->removeAllRows();
    int n = m_txHistoryObj->logSize();
    //qDebug()<<QString("try reloadTableData of JSPosHistoryTab, records %1").arg(n);

    QTableWidget *t = m_table->table();
    for (int i=0; i<m_stepData.count(); i++)
    {
        const PosLineStep &pos = m_stepData.at(i);
        QStringList row_data;
        row_data << pos.pool_info << pos.start_state.strTime() << pos.exit_state.strTime();
        row_data << pos.strPriceRange() << pos.strCurrentPrices();
        row_data << pos.strDeposited() << pos.strDecreased() << pos.strClaimedFees();
        row_data << QString("%1%").arg(QString::number(pos.calcAPR(), 'f', 1)) << pos.strLasting();
        LTable::addTableRow(t, row_data);

        if (pos.isClosed())
        {
            if (pos.zeroResult()) LTable::setTableRowColor(t, i, "#778899");
            else LTable::setTableRowColor(t, i, "#FAEBD7");
        }

        //t->item(i, OPEN_TIME_COL+1)->setToolTip(QString("%1 days").arg(QString::number(pos.lifeTime(), 'f', 1)));
        t->item(i, DEPOSITED_COL)->setToolTip(QString("%1 %2").arg(QString::number(pos.startAssetsSize(), 'f', 2)).arg(pos.totalSizeTokenName()));
        t->item(i, DECREASED_COL)->setToolTip(QString("%1 %2").arg(QString::number(pos.decreasedAssetsSize(), 'f', 2)).arg(pos.totalSizeTokenName()));
        t->item(i, REWARDS_COL)->setToolTip(QString("%1 %2").arg(QString::number(pos.rewardsAssetsSize(), 'f', 2)).arg(pos.totalSizeTokenName()));
        t->item(i, RANGE_COL)->setToolTip(pos.tick_range);

    }

    m_table->searchExec();
}
void JSPosHistoryTab::slotPosStateReceived(QString pool_key, QString pos_state)
{
    qDebug()<<QString("JSPosHistoryTab::slotPosStateReceived  pool_key[%1]  pos_state[%2]").arg(pool_key).arg(pos_state);
    QStringList list = LString::trimSplitList(pos_state, "/");
    if (list.count() != 4) {emit signalError("JSPosHistoryTab: pos_state count !=4"); return;}

    QTableWidget *t = m_table->table();
    for (int i=0; i<m_stepData.count(); i++)
    {
        PosLineStep &pos = m_stepData[i];
        if (pos.isClosed()) continue;

        if (pool_key.contains(pos.pool_info) && pool_key.contains(pos.tick_range))
        {
            pos.exit_state.current_price = list.at(0).toFloat();
            pos.claimed_rewards.first = list.at(1).toFloat();
            pos.claimed_rewards.second = list.at(2).toFloat();
            t->item(i, CP_COL)->setText(pos.strCurrentPrices());
            t->item(i, REWARDS_COL)->setText(pos.strClaimedFees());
            t->item(i, APR_COL)->setText(QString("%1%").arg(QString::number(pos.calcAPR(), 'f', 1)));
            t->item(i, REWARDS_COL)->setToolTip(QString("%1 %2").arg(QString::number(pos.rewardsAssetsSize(), 'f', 2)).arg(pos.totalSizeTokenName()));
            if (list.at(3).toLower().contains("active")) LTable::setTableRowColor(t, i, "#7FFFA0");
            else LTable::setTableRowColor(t, i, "#FFFF0F");
            break;
        }
    }
}




//////////////PosLineStep//////////////////////
void PosLineStep::reset()
{
    tx_kind.clear();
    pool_info.clear();
    pool_address.clear();
    tick_range.clear();

    pos_range.first = pos_range.second = -1;
    claimed_rewards.first = claimed_rewards.second = 0;
    start_state.reset();
    exit_state.reset();
    wait_collect = false;
}
bool PosLineStep::invalid() const
{
    if (!start_state.dt.isValid()) return true;
    if (totalSizeTokenIndex() < 0) return true;
    if (start_state.current_price <= 0) return true;
    if (start_state.token0_size <= 0 && start_state.token1_size <= 0) return true;
    return false;
}
bool PosLineStep::isClosed() const
{
    return exit_state.dt.isValid();
}
bool PosLineStep::zeroResult() const
{
    if (!isClosed()) return false;
    if (claimed_rewards.first > 0 || claimed_rewards.second > 0) return false;
    if (start_state.token0_size == 0 && exit_state.token0_size == 0)
    {
        if (start_state.token1_size == exit_state.token1_size) return true;
    }
    if (start_state.token1_size == 0 && exit_state.token1_size == 0)
    {
        if (start_state.token0_size == exit_state.token0_size) return true;
    }
    return false;
}
float PosLineStep::lifeTime() const
{
    if (!start_state.dt.isValid()) return -1;
    QDateTime second_dt = (isClosed() ? exit_state.dt : QDateTime::currentDateTime());
    qint64 mins = start_state.dt.secsTo(second_dt)/60;
    float h = float(mins)/float(60);
    return h/float(24);
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
QString PosLineStep::strLasting() const
{
    if (invalid()) return "?";
    return QString("%1 days").arg(QString::number(lifeTime(), 'f', 1));
}
float PosLineStep::startAssetsSize() const
{
    if (invalid()) return -1;

    int t_index = totalSizeTokenIndex();
    float liq_size = 0;
    if (customPool())
    {
        if (t_index == 0) liq_size = start_state.token0_size + start_state.token1_size/start_state.current_price;
        else liq_size = start_state.token1_size + start_state.token0_size/start_state.current_price;
    }
    else
    {
        if (t_index == 0) liq_size = start_state.token0_size + start_state.token1_size*start_state.current_price;
        else liq_size = start_state.token1_size + start_state.token0_size*start_state.current_price;
    }
    return liq_size;
}
float PosLineStep::rewardsAssetsSize() const
{
    if (invalid()) return -1;
    if (zeroResult()) return 0;
    if (exit_state.current_price <= 0) return 0;

    int t_index = totalSizeTokenIndex();
    float rew_size = 0;
    if (customPool())
    {
        if (t_index == 0) rew_size = claimed_rewards.first + claimed_rewards.second/exit_state.current_price;
        else rew_size = claimed_rewards.second + claimed_rewards.first/exit_state.current_price;
    }
    else
    {
        if (t_index == 0) rew_size = claimed_rewards.first + claimed_rewards.second*exit_state.current_price;
        else rew_size = claimed_rewards.second + claimed_rewards.first*exit_state.current_price;
    }
    return rew_size;
}
float PosLineStep::collectedAssetsSize() const
{
    if (invalid()) return -1;
    if (!isClosed()) return 0;

    int t_index = totalSizeTokenIndex();
    float liq_size = 0;
    if (customPool())
    {
        if (t_index == 0) liq_size = exit_state.token0_size + exit_state.token1_size/exit_state.current_price;
        else liq_size = exit_state.token1_size + exit_state.token0_size/exit_state.current_price;
    }
    else
    {
        if (t_index == 0) liq_size = exit_state.token0_size + exit_state.token1_size*exit_state.current_price;
        else liq_size = exit_state.token1_size + exit_state.token0_size*exit_state.current_price;
    }
    return (liq_size + rewardsAssetsSize());
}
float PosLineStep::decreasedAssetsSize() const
{
    if (invalid()) return -1;
    if (!isClosed()) return 0;
    return (collectedAssetsSize() - rewardsAssetsSize());
}
int PosLineStep::totalSizeTokenIndex() const
{
    QStringList s_tokens = LString::trimSplitList(pool_info.trimmed(), "/");
    if (s_tokens.count() != 3)  return -1;
    if (s_tokens.at(0).trimmed() == "USDT") return 0;
    if (s_tokens.at(1).trimmed() == "USDT") return 1;
    if (s_tokens.at(0).trimmed() == "USDC") return 0;
    if (s_tokens.at(1).trimmed() == "USDC") return 1;
    //if (s_tokens.at(0).trimmed().contains("POL")) return 0;
    if (s_tokens.at(0).trimmed().contains("ETH")) return 1;
    return 0;
}
QString PosLineStep::totalSizeTokenName() const
{
    int t_index = totalSizeTokenIndex();
    if (t_index < 0) return QString("???");
    QStringList s_tokens = LString::trimSplitList(pool_info.trimmed(), "/");
    return s_tokens.at(t_index).trimmed();
}
bool PosLineStep::customPool() const
{
    return (!pool_info.toUpper().contains("USD") && !pool_info.toUpper().contains("ETH"));
}
float PosLineStep::calcAPR() const
{

    if (invalid()) return -1;
    if (zeroResult()) return 0;

    float started_liq = startAssetsSize();
    float fees = rewardsAssetsSize();
    float days = lifeTime();
    if (started_liq <= 0 || fees <= 0 || days <= 0) return 0;

    float p = 100*fees/started_liq;
    float k = float(365)/days;
    return k*p;
}





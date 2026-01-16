#include "statpospage.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "txlogrecord.h"
#include "nodejsbridge.h"
#include "txlogger.h"
#include "deficonfig.h"
#include "lstring.h"



#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#define DATE_COL            1
#define POOL_COL            3
#define P_RANGE_COL         4
#define DEPOSITED_COL       5
#define CUR_ASSETS_COL      6
#define REWARDS_COL         8
#define YIELD_COL           9

#define YIELD_PRECISION     1



// DefiStatPosPage
DefiStatPosPage::DefiStatPosPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkStatPositions)
{
    setObjectName("defi_statpos_tab_page");
    m_txHashList.clear();

    //init table
    initTable();

}
void DefiStatPosPage::sendUpdateDataRequest()
{
  //  qDebug("DefiStatPosPage::sendUpdateDataRequest()");

    m_table->removeAllRows();
    emit signalGetTxHashHistory(m_txHashList);

   // qDebug()<<QString("got %1 hashs").arg(m_txHashList.count());
    if (m_txHashList.isEmpty()) return;

    const DefiTxLogger *plog = NULL;
    emit signalGetTxLogger(plog);
    if (plog) qDebug("plog is OK");
    else {qWarning("plog is NULL"); return;}


    foreach (const QString &hash, m_txHashList) // заполняем таблицу
    {
        const TxLogRecord *tx_rec = plog->recByHash(hash);
        checkTx(tx_rec);
    }
    foreach (const QString &hash, m_txHashList) // отмечаем закрытые позы
    {
        const TxLogRecord *tx_rec = plog->recByHash(hash);
        checkClosedTx(tx_rec);
    }

    checkOpenedPositions();
    m_table->resizeByContents();
}
QString DefiStatPosPage::strTickRange(const TxLogRecord *tx_rec) const
{
    int t1 = tx_rec->pool.tick_range.first;
    int t2 = tx_rec->pool.tick_range.second;
    return QString("[%1 : %2]").arg(t1).arg(t2);
}
QString DefiStatPosPage::strPriceRange(const TxLogRecord *tx_rec) const
{
    int prior_index = defi_config.getPriorPriceIndexByPoolAddr(tx_rec->pool.pool_addr);

    float p1 = tx_rec->pool.price_range.first;
    float p2 = tx_rec->pool.price_range.second;
    if (prior_index == 1)
    {
        float m = float(1)/p2;
        p2 = float(1)/p1;
        p1 = m;
    }

    quint8 prec = AppCommonSettings::interfacePricePrecision(p1);
    return QString("[%1 : %2]").arg(QString::number(p1, 'f', prec)).arg(QString::number(p2, 'f', prec));
}
QString DefiStatPosPage::strDepositedAssets(const TxLogRecord *tx_rec) const
{
    float size0 = tx_rec->pool.token_sizes.first;
    float size1 = tx_rec->pool.token_sizes.second;
    quint8 prec0 = AppCommonSettings::interfacePricePrecision(size0);
    quint8 prec1 = AppCommonSettings::interfacePricePrecision(size1);
    return QString("%1 / %2").arg(QString::number(size0, 'f', prec0)).arg(QString::number(size1, 'f', prec1));
}
QString DefiStatPosPage::strClosedAssets(const TxLogRecord *tx_rec) const
{
    return strDepositedAssets(tx_rec);
}
QString DefiStatPosPage::strClosedRewards(const TxLogRecord *tx_rec) const
{
    float rwd0 = tx_rec->pool.reward_sizes.first;
    float rwd1 = tx_rec->pool.reward_sizes.second;
    quint8 prec0 = AppCommonSettings::interfacePricePrecision(rwd0);
    quint8 prec1 = AppCommonSettings::interfacePricePrecision(rwd1);
    return QString("%1 / %2").arg(QString::number(rwd0, 'f', prec0)).arg(QString::number(rwd1, 'f', prec1));
}
QString DefiStatPosPage::strClosedUserRewards(const TxLogRecord *tx_rec) const
{
    int p_index = defi_config.getPoolIndex(tx_rec->pool.pool_addr);
    if (p_index < 0) return "???";
    const DefiPoolV3 &p = defi_config.pools.at(p_index);

    int prior_index = defi_config.getPriorAmountIndexByPoolAddr(tx_rec->pool.pool_addr);
    float size0 = tx_rec->pool.reward_sizes.first;
    float size1 = tx_rec->pool.reward_sizes.second;
    float p0 = tx_rec->pool.price;

    float amount = -1;
    QString token_name = "?";
    if (prior_index == 0)
    {
        amount = size0;
        if (size1 > 0) amount += (size1/p0);
        token_name = defi_config.tokenNameByAddress(p.token0_addr, p.chain_id).trimmed(); // token0
    }
    else
    {
        amount = size1;
        if (size0 > 0) amount += (size0*p0);
        token_name = defi_config.tokenNameByAddress(p.token1_addr, p.chain_id).trimmed(); // token1
    }

    quint8 prec = AppCommonSettings::interfacePricePrecision(amount);
    return QString("%1 %2").arg(QString::number(amount, 'f', prec)).arg(token_name);
}
QString DefiStatPosPage::strNestedUserAmount(const TxLogRecord *tx_rec) const
{
    int p_index = defi_config.getPoolIndex(tx_rec->pool.pool_addr);
    if (p_index < 0) return "???";
    const DefiPoolV3 &p = defi_config.pools.at(p_index);

    int prior_index = defi_config.getPriorAmountIndexByPoolAddr(tx_rec->pool.pool_addr);
    float size0 = tx_rec->pool.token_sizes.first;
    float size1 = tx_rec->pool.token_sizes.second;
    float p0 = tx_rec->pool.price;

    float amount = -1;
    QString token_name = "?";
    if (prior_index == 0)
    {
        amount = size0;
        if (size1 > 0) amount += (size1/p0);
        token_name = defi_config.tokenNameByAddress(p.token0_addr, p.chain_id).trimmed(); // token0
    }
    else
    {
        amount = size1;
        if (size0 > 0) amount += (size0*p0);
        token_name = defi_config.tokenNameByAddress(p.token1_addr, p.chain_id).trimmed(); // token1
    }

    quint8 prec = AppCommonSettings::interfacePricePrecision(amount);
    return QString("%1 %2").arg(QString::number(amount, 'f', prec)).arg(token_name);
}
float DefiStatPosPage::lastingDays(int row) const
{
    QTableWidget *t = m_table->table();
    QDateTime start_time = QDateTime::fromSecsSinceEpoch(t->item(row, DATE_COL)->data(Qt::UserRole).toInt());
    QDateTime end_time = QDateTime::fromSecsSinceEpoch(t->item(row, DATE_COL+1)->data(Qt::UserRole).toInt());
    if (!start_time.isValid() || !end_time.isValid()) return -1;

    int h_to = (start_time.secsTo(end_time)/3600);
    float d_to = float(h_to)/float(24);
    return d_to;
}
void DefiStatPosPage::checkClosedTx(const TxLogRecord *tx_rec)
{
    if (!tx_rec) {qWarning("WARNING tx_rec==NULL"); return; }
    if ((tx_rec->tx_kind != NodejsBridge::jsonCommandValue(txTakeaway)) && (tx_rec->tx_kind != NodejsBridge::jsonCommandValue(txDecrease))) return;

    int row = findRowClosedPosision(tx_rec);
    if (row < 0) {qDebug()<<QString("not found row, pid=%1").arg(tx_rec->pool.pid); return;}

    QTableWidget *t = m_table->table();
    t->item(row, DATE_COL+1)->setText(tx_rec->strDate());
    t->item(row, DATE_COL+1)->setData(Qt::UserRole, tx_rec->dt.toSecsSinceEpoch());

    float d_to = lastingDays(row);
    QString s_days = QString("%1 days").arg(QString::number(d_to, 'f', 1));
    t->item(row, DATE_COL+1)->setToolTip(s_days);

    t->item(row, CUR_ASSETS_COL)->setText(QString("0.0 / 0.0"));
    t->item(row, CUR_ASSETS_COL+1)->setText(strClosedAssets(tx_rec));
    t->item(row, CUR_ASSETS_COL+1)->setToolTip(strNestedUserAmount(tx_rec));

    t->item(row, REWARDS_COL)->setText(strClosedRewards(tx_rec));
    if (t->item(row, 0)->text().length() < 3)
        t->item(row, 0)->setText(QString::number(tx_rec->pool.pid));
    t->item(row, REWARDS_COL)->setToolTip(strClosedUserRewards(tx_rec));


    LTable::setTableRowColor(t, row, "#FFDEAD");
    calcClosedYield(row);
    calcClosedTotalResult(row);
}
void DefiStatPosPage::checkTx(const TxLogRecord *tx_rec)
{
    if (!tx_rec) {qWarning("WARNING tx_rec==NULL"); return; }

    QTableWidget *t = m_table->table();
    if (tx_rec->tx_kind == NodejsBridge::jsonCommandValue(txMint) || tx_rec->tx_kind == NodejsBridge::jsonCommandValue(txIncrease))
    {
        QStringList row_data;
        row_data << QString::number(tx_rec->pool.pid) << tx_rec->strDate() << QString("-1");
        row_data << defi_config.shortPoolDescByAddr(tx_rec->pool.pool_addr);
        row_data << strPriceRange(tx_rec);
        row_data << strDepositedAssets(tx_rec);


        for (int i=0; i<5; i++)
            row_data << QString("-");

        LTable::addTableRow(t, row_data);

        int l_row = t->rowCount()-1;
        t->item(l_row, DATE_COL)->setData(Qt::UserRole, tx_rec->dt.toSecsSinceEpoch());
        t->item(l_row, DATE_COL+1)->setData(Qt::UserRole, -1);
        t->item(l_row, DATE_COL)->setToolTip(tx_rec->strTime());
        t->item(l_row, P_RANGE_COL)->setToolTip(strTickRange(tx_rec));
        t->item(l_row, 0)->setToolTip(tx_rec->tx_kind);
        t->item(l_row, POOL_COL)->setToolTip(tx_rec->pool.pool_addr);

        if (defi_config.isStablePool(tx_rec->pool.pool_addr))
            t->item(l_row, POOL_COL)->setTextColor("#808000");

        float cur_pool_price = tx_rec->pool.price;
        if (defi_config.getPriorPriceIndexByPoolAddr(tx_rec->pool.pool_addr) == 1 && tx_rec->pool.price > 0)
            cur_pool_price = float(1)/tx_rec->pool.price;
        t->item(l_row, P_RANGE_COL)->setData(Qt::UserRole, cur_pool_price);


        t->item(l_row, DEPOSITED_COL)->setToolTip(strNestedUserAmount(tx_rec));
    }
}
void DefiStatPosPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Wallet positions list");
    m_table->vHeaderHide();

    QStringList headers;
    headers << "PID" << "Open date" << "Lasting" << "Pool" << "Price range" << "Deposited"
            << "Current assets" << "Closed assets" << "Rewards" << "Yield" << "Total result";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_table->setSelectionColor("#BFBFBF");
    h_splitter->addWidget(m_table);

    /////////////////////////////////////////

    /*
    m_integratedTable = new LTableWidgetBox(this);
    m_integratedTable->setTitle("Integrated state");
    m_integratedTable->setObjectName("positions_integrated_table");
    headers.clear();
    headers << "Value";
    LTable::setTableHeaders(m_integratedTable->table(), headers);
    headers.clear();
    headers << "Total positions" << "With liquidity" << "Minted positions" <<  "Burned positions" << "Locked sum (USDT)" << "Rewards sum (USDT)";
    LTable::setTableHeaders(m_integratedTable->table(), headers, Qt::Vertical);
    m_integratedTable->setSelectionMode(QAbstractItemView::NoSelection, QAbstractItemView::NoSelection);
    LTable::createAllItems(m_integratedTable->table());
    h_splitter->addWidget(m_integratedTable);
    */

}
int DefiStatPosPage::findRowClosedPosision(const TxLogRecord *tx_rec) const
{
    if (!tx_rec) return -1;
    QTableWidget *t = m_table->table();
    int n_row = t->rowCount();
    if (n_row <= 0) return -1;

    for (int i=0; i<n_row; i++)
    {
        QString tp_addr = t->item(i, POOL_COL)->toolTip();
        if (tp_addr != tx_rec->pool.pool_addr) continue; // другой пул
        if (t->item(i, DATE_COL+1)->data(Qt::UserRole).toInt() > 0) continue; // позиция уже отмечена

       // qDebug()<<QString("tp_addr  [%1]").arg(tp_addr);
        QString rec_ticks = strTickRange(tx_rec);
        QString t_ticks = t->item(i, P_RANGE_COL)->toolTip();
       // qDebug()<<QString("tick_range:  [%1] / [%2]").arg(rec_ticks).arg(t_ticks);
        if (rec_ticks == t_ticks) return i;
    }

    return -1;
}
float DefiStatPosPage::userTokenSum(int row, QString amount_type) const
{
    bool ok = false;
    float a = 0;

    QString cell_value = "?";
    if (amount_type == "nested") cell_value = m_table->table()->item(row, DEPOSITED_COL)->toolTip();
    if (amount_type == "closed") cell_value = m_table->table()->item(row, CUR_ASSETS_COL+1)->toolTip();
    if (amount_type == "reward") cell_value = m_table->table()->item(row, REWARDS_COL)->toolTip();
    if (amount_type == "assets") cell_value = m_table->table()->item(row, CUR_ASSETS_COL)->toolTip();

    int space_pos = cell_value.indexOf(QChar(' '));
    if (space_pos > 0) cell_value = cell_value.left(space_pos).trimmed();
    a = cell_value.toFloat(&ok);
    if (!ok) return -1;
    return a;
}
void DefiStatPosPage::calcClosedYield(int row)
{
    QTableWidget *t = m_table->table();
    if (row < 0 || row >= t->rowCount()) return;

    float days = lastingDays(row);
    float y = -1;
    float nested = userTokenSum(row, "nested");
    float rwd = userTokenSum(row, "reward");
    if (nested > 0 && days > 0)
    {
        if (rwd <= 0) y = 0;
        else y = (float(100)*rwd/nested); // заработанный процент за время жизни позы
        if (y > 0) y = float(365)*(y/days);
    }

    QString y_str = QString("%1 %").arg(QString::number(y, 'f', YIELD_PRECISION));
    t->item(row, YIELD_COL)->setText(y_str);
}
void DefiStatPosPage::calcClosedTotalResult(int row)
{
    QTableWidget *t = m_table->table();
    if (row < 0 || row >= t->rowCount()) return;

    float result = 0;
    float nested = userTokenSum(row, "nested");
    float rwd = userTokenSum(row, "reward");
    float closed = userTokenSum(row, "closed");

    QString s_result = "?";
    QString s_color = "#000000";
    if (nested > 0 && closed > 0)
    {
        if (rwd > 0) closed += rwd;
        float d = closed - nested;
        result = float(100)*d/nested;

        if (d < 0) {s_result.clear(); s_color = "#AD0000";}
        else if (d > 0) {s_result = "+"; s_color = "#0000DD";}
        else s_result.clear();

        s_result = QString("%1%2 %").arg(s_result).arg(QString::number(result, 'f', 2));
    }

    t->item(row, YIELD_COL+1)->setText(s_result);
    t->item(row, YIELD_COL+1)->setTextColor(s_color);

}
void DefiStatPosPage::calcOpenedTotalResult(int row)
{
    QTableWidget *t = m_table->table();
    if (row < 0 || row >= t->rowCount()) return;

    float result = 0;
    float nested = userTokenSum(row, "nested");
    float rwd = userTokenSum(row, "reward");
    float cur_assets = userTokenSum(row, "assets");

    QString s_result = "?";
    QString s_color = "#000000";
    float d = 0;
    if (nested > 0 && cur_assets > 0)
    {
        if (rwd > 0) cur_assets += rwd;
        d = cur_assets - nested; // абсолютная величина прироста/убытка в приоритетном токене
        result = float(100)*d/nested;

        if (d < 0) {s_result.clear(); s_color = "#AD0000";}
        else if (d > 0) {s_result = "+"; s_color = "#0000DD";}
        else s_result.clear();

        s_result = QString("%1%2 %").arg(s_result).arg(QString::number(result, 'f', 2));
    }

    t->item(row, YIELD_COL+1)->setText(s_result);
    t->item(row, YIELD_COL+1)->setTextColor(s_color);

    // set tooltip absolute value amount changing
    QString p_addr = t->item(row, POOL_COL)->toolTip();
    int prior_index_amount = defi_config.getPriorAmountIndexByPoolAddr(p_addr);
    QString token_name = ((prior_index_amount == 1) ? defi_config.token1NameByPoolAddr(p_addr) : defi_config.token0NameByPoolAddr(p_addr));
    int prec = AppCommonSettings::interfacePricePrecision(d);
    QString d_str = QString("%1 %2").arg(QString::number(d, 'f', prec)).arg(token_name);
    t->item(row, YIELD_COL+1)->setToolTip(d_str);

}
void DefiStatPosPage::checkOpenedPositions()
{
   // qDebug("DefiStatPosPage::checkOpenedPositions()");
    QMap<int, QStringList> data;
    emit signalGetOpenedPosState(data);

  //  qDebug()<<QString("opened positions: %1").arg(data.count());

    // it.value() - QStringList => one pos_data
    // 6 элементов: pool_addr / tick_range / cur_price(by prior_index) / cur_assets / cur_rewards / 'out' or 'active' range
    QMap<int, QStringList>::const_iterator it = data.constBegin();
    while (it != data.constEnd())
    {
        checkOpenedPosition(it.key(), it.value());
        it++;
    }
}
void DefiStatPosPage::checkOpenedPosition(int pid, const QStringList &pos_data)
{
    QTableWidget *t = m_table->table();
    int n_row = t->rowCount();
    if (n_row <= 0) return;
    if (pos_data.count() != 6) return;

    int modified_index = -1;
    for (int i=0; i<n_row; i++)
    {
        QString tp_addr = t->item(i, POOL_COL)->toolTip();
        if (tp_addr != pos_data.first()) continue;
        if (t->item(i, DATE_COL+1)->data(Qt::UserRole).toInt() > 0) continue; // позиция уже отмечена

        QString t_ticks = t->item(i, P_RANGE_COL)->toolTip();
        //qDebug()<<QString("tick_range:  [%1] / [%2]").arg(rec_ticks).arg(t_ticks);
        if (pos_data.at(1) == t_ticks) // позиция найдена
        {
            updateOpenedPosition(i, pid, pos_data);
            modified_index = i;
            break;
        }
    }

    if (modified_index > 0)
        LTable::shiftTableRowToBegin(t, modified_index);
}
void DefiStatPosPage::updateOpenedPosition(int row, int pid, const QStringList &pos_data)
{
    QTableWidget *t = m_table->table();
    t->item(row, 0)->setText(QString::number(pid));
    t->item(row, 0)->setTextColor(Qt::darkGreen);
    t->item(row, CUR_ASSETS_COL)->setText(pos_data.at(3));
    t->item(row, CUR_ASSETS_COL)->setData(Qt::UserRole, pos_data.at(2));
    t->item(row, REWARDS_COL)->setText(pos_data.at(4));

    if (pos_data.last().trimmed().toLower() == "out")
        t->item(row, P_RANGE_COL)->setTextColor(Qt::darkRed);

    // set current ts point
    t->item(row, DATE_COL+1)->setData(Qt::UserRole, QDateTime::currentDateTime().toSecsSinceEpoch());
    float d_to = lastingDays(row);
    QString s_days = QString("%1 d").arg(QString::number(d_to, 'f', 1));
    t->item(row, DATE_COL+1)->setText(s_days);

    ////////////////// calc assets in prior token ///////////////////////////
    updatePriorTokenOpenedPosition(row, CUR_ASSETS_COL, pos_data);
    updatePriorTokenOpenedPosition(row, REWARDS_COL, pos_data);

    // calc yield/total_result
    calcClosedYield(row);
    calcOpenedTotalResult(row);
}
void DefiStatPosPage::updatePriorTokenOpenedPosition(int row, int col, const QStringList &pos_data)
{
    //qDebug()<<QString("----updatePriorTokenOpenedPosition row=%1  col=%2 -----").arg(row).arg(col);
    QString toolTip_value = "?";
    QTableWidget *t = m_table->table();


    // pos_data: pool_addr / tick_range / cur_price(by prior_index) / cur_assets / cur_rewards / 'out' or 'active' range
    bool ok = false;
    float cur_p = pos_data.at(2).toFloat(&ok);
    QStringList a_sizes = LString::trimSplitList(t->item(row, col)->text(), QString("/"));
    int p_index = defi_config.getPoolIndex(pos_data.first());
    //qDebug()<<QString("p_index=%1 cur_p=%2  a_sizes(%3)").arg(p_index).arg(cur_p).arg(a_sizes.count());

    if (ok && a_sizes.count() == 2 && p_index >= 0)
    {
        float size0 = a_sizes.first().trimmed().toFloat(&ok);
        if (ok)
        {
            //qDebug("PASS 2");
            float size1 = a_sizes.last().trimmed().toFloat(&ok);
            if (ok)
            {
                const DefiPoolV3 &p = defi_config.pools.at(p_index);
                int prior_index_amount = defi_config.getPriorAmountIndexByPoolAddr(p.address);
                int prior_index_price = defi_config.getPriorPriceIndexByPoolAddr(p.address);
                //qDebug()<<QString("PASS 3: [%1]   prior_index_amount=%2  prior_index_price=%3").arg(defi_config.shortPoolDescByAddr(p.address)).arg(prior_index_amount).arg(prior_index_price);

                if (cur_p > 0)
                {
                    if (prior_index_price == 1 ) cur_p = 1/cur_p;

                    float amount = -1;
                    QString token_name = "?";
                    if (prior_index_amount == 1)
                    {
                        amount = size1;
                        if (size0 > 0) amount += (size0*cur_p);
                        token_name = defi_config.token1NameByPoolAddr(p.address).trimmed(); // token1
                    }
                    else
                    {
                        amount = size0;
                        if (size1 > 0) amount += (size1/cur_p);
                        token_name = defi_config.token0NameByPoolAddr(p.address).trimmed(); // token0
                    }

                    quint8 prec = AppCommonSettings::interfacePricePrecision(amount);
                    toolTip_value = QString("%1 %2").arg(QString::number(amount, 'f', prec)).arg(token_name);
                }
            }
        }
    }

    t->item(row, col)->setToolTip(toolTip_value);
}








#include "ug_daysdatapage.h"
#include "subcommonsettings.h"
#include "lchart.h"
#include "lfile.h"

#include <QTableWidget>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>



#define TRIM_DATA_SIZE      500

//UG_DaysDataPage
UG_DaysDataPage::UG_DaysDataPage(QWidget *parent)
    :UG_BasePage(parent, 32, rtDaysData),
      m_poolListWidget(NULL),
      m_historyTable(NULL),
      m_chart(NULL)
{
    setObjectName("ug_daysdata_page");

    initPage();
    initTable();

}
void UG_DaysDataPage::prepareQuery()
{
    QString pool_id(curPoolID());
    if (pool_id.isEmpty())
    {
        emit signalError("You must select pool item.");
        return;
    }

    emit signalMsg(QString("id=%1").arg(pool_id));

    QString query = QString("pool(id: \"%1\")").arg(pool_id);
    QString s_fields = ("close tvlUSD volumeUSD date");
    query = QString("%1 {poolDayData(first: %2, skip: 0) {%3} }").arg(query).arg(m_reqLimit).arg(s_fields);
    m_reqData->query = QString("{ %1 }").arg(query);
}
void UG_DaysDataPage::updateDataPage(bool forcibly)
{
    qDebug("UG_DaysDataPage::updateDataPage");
    qDebug()<<QString("forcibly: %1").arg(forcibly?"true":"false");
    if (!forcibly) return;

    /*
    emit signalGetReqLimit(m_reqLimit);


    m_data.clear();
    m_historyTable->removeAllRows();

    m_historyTable->resizeByContents();
    QString pool_id(curPoolID());
    if (pool_id.isEmpty())
    {
        emit signalError("You must select pool item.");
        return;
    }

    emit signalMsg(QString("id=%1").arg(pool_id));

    QString query = QString("pool(id: \"%1\")").arg(pool_id);
    query = QString("%1 {poolDayData(first: %1, skip: 0) {close tvlUSD volumeUSD date} }").arg(m_reqLimit);
    m_reqData->query = QString("{ %1 }").arg(query);

    sendRequest();
    */

    /*
    QString s_fields = "id";
    s_fields = QString("%1 feeTier").arg(s_fields);
    s_fields = QString("%1 totalValueLockedUSD").arg(s_fields);
    s_fields = QString("%1 volumeUSD").arg(s_fields);
    s_fields = QString("%1 createdAtTimestamp").arg(s_fields);
    s_fields = QString("%1 token0{symbol}").arg(s_fields);
    s_fields = QString("%1 token1{symbol}").arg(s_fields);
    m_reqData->query = QString("{pools (first: %1) {%2}}").arg(m_reqLimit).arg(s_fields);
    */
}
void UG_DaysDataPage::startUpdating(quint16 t)
{
    UG_BasePage::startUpdating(t);
    clearPage();
    emit signalGetReqLimit(m_reqLimit);
    prepareQuery();
//    m_skip = 0;
}
void UG_DaysDataPage::slotTimer()
{
    if (m_reqData->query.isEmpty())
    {
        stopUpdating();
        return;
    }

    UG_BasePage::slotTimer();
    emit signalMsg("try next query ......");
//    prepareQuery();
  //  m_skip += m_reqLimit;

    sendRequest();
    //stopUpdating();

}
void UG_DaysDataPage::clearPage()
{
    m_data.clear();
    m_historyTable->removeAllRows();
    m_historyTable->resizeByContents();
    m_reqData->query.clear();
}
void UG_DaysDataPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;
    qDebug()<<QString("UG_PoolPage::slotJsonReply  req_type=%1, OK!").arg(req_type);

    /*
    const QJsonValue &j_data = j_obj.value("data");
    if (j_data.isNull()) {emit signalError("UG_PoolPage: result QJsonValue <data> not found"); return;}
    const QJsonValue &j_list = j_data.toObject().value("pools");
    if (j_list.isNull()) {emit signalError("UG_PoolPage: QJsonValue <pools> not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty()) {emit signalError("UG_PoolPage: pools QJsonArray is empty"); return;}
    */

   // qDebug()<<QString("j_arr DATA %1").arg(j_arr.count());
    /*
    for (int i=0; i<j_arr.count(); i++)
    {
        UG_PoolInfo pool;
        pool.fromJson(j_arr.at(i).toObject());

        //qDebug()<<pool.toStr();
        if (!pool.invalid())
        {
            if (pool.tvl >= m_minTVL && pool.volume_all > 0)
                m_pools.append(pool);
        }
        //else qWarning("WARNING: INVALID POOL DATA!!!");
    }
    */

    updateTableData();

    //if (j_arr.count() < m_reqLimit)
    {
        //qDebug()<<QString("m_reqLimit=%1 < j_arr(%2), need stop updating").arg(m_reqLimit).arg(j_arr.count());
        emit signalMsg("Updating days data finished!");
        emit signalStopUpdating();
    }
}
QString UG_DaysDataPage::curPoolID() const
{
    QList<int> sel_rows(m_poolListWidget->selectedRows());
    if (sel_rows.isEmpty()) return QString();
    return m_poolListWidget->listWidget()->item(sel_rows.first())->data(Qt::UserRole).toString().trimmed();
}
void UG_DaysDataPage::initPage()
{
    m_poolListWidget = new LListWidgetBox(this);
    m_historyTable = new LTableWidgetBox(this);
    m_chart = new LChartWidget(this);

    v_splitter->addWidget(m_historyTable);
    v_splitter->addWidget(m_chart);
    h_splitter->insertWidget(0, m_poolListWidget);
}
void UG_DaysDataPage::initTable()
{
    QStringList headers;
    headers << "Date" << "TVL, M" << "Volume" << "Fee size" << "Factor_TH";
    m_historyTable->setHeaderLabels(headers);
    m_historyTable->setTitle("Days data");
    m_historyTable->setObjectName("table_box");

    m_historyTable->resizeByContents();
}
void UG_DaysDataPage::updateTableData()
{
    QTableWidget *tw = m_historyTable->table();
    int start_index = tw->rowCount();
    if (m_data.isEmpty())
    {
        qDebug()<<QString("WARNIG pools data is empty");
        return;
    }

    if (m_data.count() > start_index)
    {
        /*
        for (int i=start_index; i<m_data.count(); i++)
        {
            QStringList row_data;
            m_pools.at(i).toTableRow(row_data);
            LTable::addTableRow(tw, row_data);
        }
        m_tableBox->resizeByContents();
        m_tableBox->searchExec();
        */
    }
    qDebug()<<QString("m_data size %1, table row count %2").arg(m_data.count()).arg(tw->rowCount());
}
void UG_DaysDataPage::loadData()
{
    QListWidget *lw = m_poolListWidget->listWidget();
    lw->clear();
    m_data.clear();

    QString fname("pools");
    if (sub_commonSettings.cur_factory >= 0)
    {
        QString chain = sub_commonSettings.factories.at(sub_commonSettings.cur_factory).chain.toLower().trimmed();
        fname += QString("_%1").arg((chain.length()>4) ? chain.left(3) : chain);
    }
    fname += QString(".txt");
    fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(fname);



    emit signalMsg(QString("try load pools list [%1] ........").arg(fname));

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }

    foreach (const QString v, fdata)
    {
        if (v.trimmed().isEmpty()) continue;

        UG_PoolInfo pool;
        pool.fromFileLine(v);
        if (!pool.invalid())
        {
            QString item_text = QString("%1/%2").arg(pool.token0).arg(pool.token1);
            item_text = QString("%1 (%2%)").arg(item_text).arg(QString::number(pool.fee, 'f', 2));
            m_poolListWidget->addItem(item_text);
            lw->item(lw->count()-1)->setData(Qt::UserRole, pool.id);
        }
    }
    emit signalMsg(QString("loaded %1 records").arg(m_poolListWidget->listWidget()->count()));
    m_poolListWidget->setTitle(QString("Pools (%1)").arg(m_poolListWidget->listWidget()->count()));


}



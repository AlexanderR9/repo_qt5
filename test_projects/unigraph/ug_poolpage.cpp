#include "ug_poolpage.h"
#include "ltable.h"
#include "lfile.h"
#include "subcommonsettings.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QDir>


//UG_PoolPage
UG_PoolPage::UG_PoolPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtPools),
      m_tableBox(NULL),
      m_minTVL(2000),
      m_skip(0)
{
    setObjectName("ug_pool_page");

    initTable();

}
void UG_PoolPage::initTable()
{
    m_tableBox = new LSearchTableWidgetBox(this);

    QStringList headers;
    headers << "ID" << "TVL, M" << "Volume_all, M" << "Token_0" << "Token_1" << "Fee" << "Creation time";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Pool list");
    m_tableBox->setObjectName("table_box");
    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    v_splitter->addWidget(m_tableBox);

    m_tableBox->sortingOn();
    m_tableBox->addSortingData(1, LTableWidgetBox::sdtNumeric);
    m_tableBox->addSortingData(2, LTableWidgetBox::sdtNumeric);
    m_tableBox->addSortingData(3, LTableWidgetBox::sdtString);
    m_tableBox->addSortingData(4, LTableWidgetBox::sdtString);

}
void UG_PoolPage::updateDataPage(bool forcibly)
{
    qDebug("UG_PoolPage::updateDataPage");
    qDebug()<<QString("forcibly: %1").arg(forcibly?"true":"false");
    if (!forcibly) return;

    emit signalGetFilterParams(m_reqLimit, m_minTVL);


    m_pools.clear();
    m_tableBox->removeAllRows();

    m_tableBox->resizeByContents();
    QString s_fields = "id";
    s_fields = QString("%1 feeTier").arg(s_fields);
    s_fields = QString("%1 totalValueLockedUSD").arg(s_fields);
    s_fields = QString("%1 volumeUSD").arg(s_fields);
    s_fields = QString("%1 createdAtTimestamp").arg(s_fields);
    s_fields = QString("%1 token0{symbol}").arg(s_fields);
    s_fields = QString("%1 token1{symbol}").arg(s_fields);
    m_reqData->query = QString("{pools (first: %1) {%2}}").arg(m_reqLimit).arg(s_fields);
    sendRequest();
}
void UG_PoolPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;
    qDebug()<<QString("UG_PoolPage::slotJsonReply  req_type=%1, OK!").arg(req_type);

    const QJsonValue &j_data = j_obj.value("data");
    if (j_data.isNull()) {emit signalError("UG_PoolPage: result QJsonValue <data> not found"); return;}
    const QJsonValue &j_list = j_data.toObject().value("pools");
    if (j_list.isNull()) {emit signalError("UG_PoolPage: QJsonValue <pools> not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty()) {emit signalError("UG_PoolPage: pools QJsonArray is empty"); return;}

    qDebug()<<QString("j_arr POOLS %1").arg(j_arr.count());
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

    updateTableData();

    if (j_arr.count() < m_reqLimit)
    {
        qDebug()<<QString("m_reqLimit=%1 < j_arr(%2), need stop updating").arg(m_reqLimit).arg(j_arr.count());
        emit signalMsg("Updating pools data finished!");
        emit signalStopUpdating();
    }
}
void UG_PoolPage::slotTimer()
{
    UG_BasePage::slotTimer();
    emit signalMsg("try next query ......");
    prepareQuery();
    m_skip += m_reqLimit;

    sendRequest();
}
QString UG_PoolPage::dataFile() const
{
    QString fname("pools");
    if (sub_commonSettings.cur_factory >= 0)
    {
        QString chain = sub_commonSettings.factories.at(sub_commonSettings.cur_factory).chain.toLower().trimmed();
        fname += QString("_%1").arg((chain.length()>4) ? chain.left(3) : chain);
    }
    fname += QString(".txt");

    return QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(fname);
}
void UG_PoolPage::saveData()
{
    qDebug("UG_PoolPage::saveData()");

    if (m_pools.isEmpty())
    {
        emit signalError("Pools container is empty.");
        return;
    }

    QString err;
    QString fname(dataFile());
    QStringList fdata;
    fdata << QString("POOLS INFO:");

    foreach (const UG_PoolInfo &p, m_pools)
        fdata << p.toFileLine();
    fdata.append(QString());

    emit signalMsg(QString("filiname[%1]  datasize[%2]").arg(fname).arg(fdata.count()));
    err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg("Ok!");

}
void UG_PoolPage::loadData()
{
    qDebug("UG_PoolPage::loadData()");
    clearPage();
    emit signalMsg(QString("try load pools data from file [%1] ........").arg(dataFile()));

    QStringList fdata;
    QString err = LFile::readFileSL(dataFile(), fdata);
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
            m_pools.append(pool);
    }
    emit signalMsg(QString("loaded %1 records").arg(m_pools.count()));

    updateTableData();
}
void UG_PoolPage::prepareQuery()
{
    QString s_fields = "id";
    s_fields = QString("%1 feeTier").arg(s_fields);
    s_fields = QString("%1 totalValueLockedUSD").arg(s_fields);
    s_fields = QString("%1 volumeUSD").arg(s_fields);
    s_fields = QString("%1 createdAtTimestamp").arg(s_fields);
    s_fields = QString("%1 token0{symbol id}").arg(s_fields);
    s_fields = QString("%1 token1{symbol id}").arg(s_fields);

    QString s_where = QString("where: {totalValueLockedUSD_gt: %1}").arg(quint32(m_minTVL));

    QString s_tag = QString("pools(first: %1, skip: %2, %3)").arg(m_reqLimit).arg(m_skip).arg(s_where);
    m_reqData->query = QString("{%1 {%2}}").arg(s_tag).arg(s_fields);
}
void UG_PoolPage::startUpdating(quint16 t)
{
    UG_BasePage::startUpdating(t);
    clearPage();
    m_reqData->query.clear();
    emit signalGetFilterParams(m_reqLimit, m_minTVL);
    m_skip = 0;
}
void UG_PoolPage::clearPage()
{
    m_pools.clear();
    m_tableBox->removeAllRows();
    m_tableBox->resizeByContents();
}
void UG_PoolPage::updateTableData()
{
    QTableWidget *tw = m_tableBox->table();
    int start_index = tw->rowCount();
    if (m_pools.isEmpty())
    {
        qDebug()<<QString("WARNIG pools data is empty");
        return;
    }

    if (m_pools.count() > start_index)
    {
        for (int i=start_index; i<m_pools.count(); i++)
        {
            QStringList row_data;
            m_pools.at(i).toTableRow(row_data);
            LTable::addTableRow(tw, row_data);
        }
        m_tableBox->resizeByContents();
        m_tableBox->searchExec();
    }
    qDebug()<<QString("m_pools size %1, table row count %2").arg(m_pools.count()).arg(tw->rowCount());
}
void UG_PoolPage::slotReqBuzyNow()
{
    if (updatingRunning())
    {
        m_skip -= m_reqLimit;
    }
}
void UG_PoolPage::slotSetTokensFromPage(QHash<QString, QString> &map)
{
    qDebug("UG_PoolPage::slotSetTokensFromPage");
    map.clear();
    foreach (const UG_PoolInfo &p, m_pools)
    {
        map.insert(p.token0_id, p.token0);
        map.insert(p.token1_id, p.token1);
    }
}


#include "ug_daysdatapage.h"
#include "subcommonsettings.h"
#include "lchart.h"
#include "lfile.h"
#include "ltable.h"

#include <QTableWidget>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>



#define TRIM_DATA_SIZE      360
#define FACTOR_TH_COL       5


//UG_DaysDataPage
UG_DaysDataPage::UG_DaysDataPage(QWidget *parent)
    :UG_BasePage(parent, 32, rtDaysData),
      m_skip(0),
      m_poolListWidget(NULL),
      m_historyTable(NULL),
      m_chart(NULL)
{
    setObjectName("ug_daysdata_page");

    initPage();
    initTable();
    initChart();

}
void UG_DaysDataPage::prepareQuery()
{
    QString pool_id(curPoolID());
    if (pool_id.isEmpty())
    {
        emit signalError("You must select pool item.");
        emit signalStopUpdating();
        return;
    }

    emit signalMsg(QString("id=%1").arg(pool_id));

    QString query = QString("pool(id: \"%1\")").arg(pool_id);
    QString s_fields = ("close tvlUSD volumeUSD date feesUSD");
    query = QString("%1 {poolDayData(first: %2, skip: %3) {%4} }").arg(query).arg(m_reqLimit).arg(m_skip).arg(s_fields);
    m_reqData->query = QString("{ %1 }").arg(query);
}
void UG_DaysDataPage::updateDataPage(bool forcibly)
{
    qDebug("UG_DaysDataPage::updateDataPage");

    m_chart->removeChart();
    m_chart->updateAxis();


    qDebug()<<QString("forcibly: %1").arg(forcibly?"true":"false");
    if (!forcibly) return;
}
void UG_DaysDataPage::startUpdating(quint16 t)
{
    UG_BasePage::startUpdating(t);
    clearPage();
    emit signalGetReqLimit(m_reqLimit);
    //prepareQuery();
    m_skip = 0;
}
void UG_DaysDataPage::slotTimer()
{
    UG_BasePage::slotTimer();
    emit signalMsg("try next query ......");
    prepareQuery();
    m_skip += m_reqLimit;

    sendRequest();
}
void UG_DaysDataPage::clearPage()
{
    m_data.clear();
    m_historyTable->removeAllRows();
    m_historyTable->resizeByContents();
    m_reqData->query.clear();

    m_chart->removeChart();
    m_chart->updateAxis();
}
void UG_DaysDataPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;
    qDebug()<<QString("UG_PoolPage::slotJsonReply  req_type=%1, OK!").arg(req_type);


    const QJsonValue &j_data = j_obj.value("data");
    if (j_data.isNull()) {emit signalError("UG_DaysDataPage: result QJsonValue <data> not found"); return;}
    const QJsonValue &j_pdata = j_data.toObject().value("pool");
    if (j_pdata.isNull()) {emit signalError("UG_DaysDataPage: QJsonValue <pool> not found"); return;}
    const QJsonArray &j_arr = j_pdata.toObject().value("poolDayData").toArray();
    if (j_arr.isEmpty()) {emit signalError("UG_DaysDataPage: poolDayData QJsonArray is empty"); return;}


    qDebug()<<QString("j_arr DATA %1").arg(j_arr.count());
    for (int i=0; i<j_arr.count(); i++)
    {
        UG_PoolDayData dd;
        dd.fromJson(j_arr.at(i).toObject());
        if (!dd.invalid())
        {
            m_data.append(dd);
        }
        else
        {
            qWarning("\n WARNING: INVALID DAY DATA!!!");
            qDebug()<<dd.toStr();
        }
    }

    if (j_arr.count() < m_reqLimit)
    {
        qDebug()<<QString("m_reqLimit=%1 < j_arr(%2), need stop updating").arg(m_reqLimit).arg(j_arr.count());
        updateTableData();
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
    m_poolListWidget = new LSearchListWidgetBox(this);
    m_historyTable = new LTableWidgetBox(this);
    m_chart = new LChartWidget(this);

    v_splitter->addWidget(m_historyTable);
    v_splitter->addWidget(m_chart);
    h_splitter->insertWidget(0, m_poolListWidget);
}
void UG_DaysDataPage::initTable()
{
    QStringList headers;
    headers << "Date" << "Price" << "TVL, M" << "Volume" << "Fee size" << "Factor_TH" << "avg Factor_TH (7d)" << "avg Factor_TH (30d)";
    m_historyTable->setHeaderLabels(headers);
    m_historyTable->setTitle("Days data");
    m_historyTable->setObjectName("table_box");
    m_historyTable->setSelectionMode(QAbstractItemView::SelectColumns, QAbstractItemView::ExtendedSelection);
    m_historyTable->resizeByContents();

    connect(m_historyTable->table(), SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
}
QColor UG_DaysDataPage::chartColor(int c_index) const
{
    switch (c_index)
    {
        case 0: return Qt::darkBlue;
        case 1: return Qt::darkGreen;
        case 2: return Qt::darkRed;
        case 3: return Qt::darkYellow;
        case 4: return Qt::darkGray;
        default: break;
    }
    return Qt::black;
}
void UG_DaysDataPage::slotSelectionChanged()
{
    QList<int> sel_cols(LTable::selectedCols(m_historyTable->table()));
    qDebug()<<QString("UG_DaysDataPage::slotSelectionChanged(), sel_cols %1").arg(sel_cols.count());
    if (m_data.isEmpty()) return;

    QList<int> chart_cols;
    if (sel_cols.contains(1)) chart_cols.append(1);
    if (sel_cols.contains(2)) chart_cols.append(2);
    if (sel_cols.contains(5)) chart_cols.append(5);
    if (sel_cols.contains(6)) chart_cols.append(6);
    if (sel_cols.contains(7)) chart_cols.append(7);

    repaintChart(chart_cols);
}
void UG_DaysDataPage::fillPoints(QList<QPointF> &points, int col)
{
    points.clear();

    QTableWidget *tw = m_historyTable->table();
    for (int i=tw->rowCount()-1; i>=0; i--)
    {
        float y = tw->item(i, col)->text().toFloat();
        QDateTime dt = QDateTime::fromString(tw->item(i, 0)->text(),UG_APIReqParams::userDateMask());
        int x = dt.toSecsSinceEpoch();
        points.append(QPointF(x, y));
    }
}
void UG_DaysDataPage::repaintChart(const QList<int> &cols)
{
    m_chart->removeChart();
    if (cols.isEmpty()) return;

    foreach (int col, cols)
    {
        QList<QPointF> points;
        fillPoints(points, col);

        LChartParams p;
        p.lineColor = chartColor(m_chart->chartsCount());
        p.pointsColor = QColor(0, 100, 0);
        p.points.clear();
        p.points.append(points);
        m_chart->addChart(p);

    }
    m_chart->updateAxis();
}
void UG_DaysDataPage::initChart()
{
    m_chart->setCrossColor(QColor(150, 190, 150));
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    m_chart->setCrossXAxisTextViewMode(2);
    m_chart->setPointSize(2);
    m_chart->setAxisPrecision(-1, 2);
    m_chart->updateAxis();
}
void UG_DaysDataPage::trimData()
{
    if (m_data.count() <= TRIM_DATA_SIZE) return;

    while (m_data.count() > TRIM_DATA_SIZE)
    {
        m_data.removeFirst();
    }
}
void UG_DaysDataPage::calcAvgFactorTH(quint16 n_days, int col)
{
    QTableWidget *tw = m_historyTable->table();
    int n = m_data.count();
    if (n_days < 5 || (n-n_days) < 2) return;
    int purpose_i = n_days-1;
    while (purpose_i < n)
    {
        int low_i = purpose_i - n_days + 1;
        float f_avg = 0;
        for (int i=low_i; i<=purpose_i; i++)
        {
            f_avg += tw->item(n-i-1, FACTOR_TH_COL)->text().toFloat();
        }
        f_avg /= float(n_days);
        tw->item(n-purpose_i-1, col)->setText(QString::number(f_avg, 'f', 2));
        purpose_i++;
    }
}
void UG_DaysDataPage::updateTableData()
{
    QTableWidget *tw = m_historyTable->table();
    if (m_data.isEmpty())
    {
        emit signalError(QString("WARNIG pools data is empty"));
        return;
    }
    else trimData();

    int n = m_data.count();
    for (int i=n-1; i>=0; i--)
    {
        QStringList row_data;
        m_data.at(i).toTableRow(row_data);
        LTable::addTableRow(tw, row_data);
    }

    calcAvgFactorTH(7, FACTOR_TH_COL+1);
    calcAvgFactorTH(30, FACTOR_TH_COL+2);
    m_historyTable->resizeByContents();
}
void UG_DaysDataPage::loadChainPools(QString fname, QString icon_path)
{
    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    qDebug()<<QString("fname[%1]  icon_path[%2]").arg(fname).arg(icon_path);
    QListWidget *lw = m_poolListWidget->listWidget();
    foreach (const QString v, fdata)
    {
        if (v.trimmed().isEmpty()) continue;

        UG_PoolInfo pool;
        pool.fromFileLine(v);
        if (!pool.invalid())
        {
            bool need_add = true;
            if (sub_commonSettings.only_prefer_tokens)
            {
                if (!sub_commonSettings.prefer_tokens.contains(pool.token0) ||
                        !sub_commonSettings.prefer_tokens.contains(pool.token1)) need_add = false;
            }

            if (need_add)
            {
                QString item_text = QString("%1/%2").arg(pool.token0).arg(pool.token1);
                item_text = QString("%1 (%2%)").arg(item_text).arg(QString::number(pool.fee, 'f', 2));
                m_poolListWidget->addItem(item_text, icon_path);
                lw->item(lw->count()-1)->setData(Qt::UserRole, pool.id);
            }
        }
    }
    emit signalMsg(QString("loaded %1 records").arg(m_poolListWidget->listWidget()->count()));
}
void UG_DaysDataPage::loadData()
{
    m_poolListWidget->listWidget()->clear();
    m_data.clear();

    foreach (const SubGraph_CommonSettings::SGFactory &v, sub_commonSettings.factories)
    {
        QString fname("pools");
        QString chain = v.chain.toLower().trimmed();

        //test
        //if (chain.contains("bnb") || chain.contains("eth") || chain.contains("base")) continue;
        if (chain.contains("eth")) continue;

        fname += QString("_%1").arg((chain.length()>4) ? chain.left(3) : chain);
        fname += QString(".txt");

        emit signalMsg(QString("try load pools list [%1] ........").arg(fname));
        fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(fname);

        loadChainPools(fname, v.iconPath());
        QApplication::processEvents();
    }

    m_poolListWidget->setTitle(QString("Pools (%1)").arg(m_poolListWidget->listWidget()->count()));
    m_poolListWidget->searchExec();
}
void UG_DaysDataPage::slotReqBuzyNow()
{
    if (updatingRunning())
    {
        m_skip -= m_reqLimit;
    }
}



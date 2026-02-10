#include "apicandlespage.h"
#include "lfile.h"
#include "ltable.h"
#include "lstring.h"
#include "apicommonsettings.h"
#include "instrument.h"
#include "lchart.h"
#include "moexbondhistorydownloader.h"
#include "ltime.h"


#include <QDir>
#include <QDebug>
#include <QTableWidget>
#include <QSplitter>
#include <QSettings>


#define FAVOR_LIST_FILE     QString("favorite.txt")
#define HISTORY_FOLDER      QString("candles")


//APICandlesPage
APICandlesPage::APICandlesPage(QWidget *parent)
    :LSimpleWidget(parent, 31),
    m_assetTable(NULL),
    m_candleTable(NULL),
    m_chart(NULL),
    m_downloader(NULL)
{
    setObjectName("api_candles_page");
    m_userSign = aptCandles;

    reinitWidgets();
    initDownloader();

    // init popup
    initPopupMenu();


    /*

    connect(m_tableBox, SIGNAL(signalSearched()), this, SLOT(slotFilter()));
    */
}
void APICandlesPage::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    loadDataFile();
    reloadAssetTable();
}
QString APICandlesPage::dataFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(FAVOR_LIST_FILE);
}
QString APICandlesPage::historyPath()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(HISTORY_FOLDER);
}
void APICandlesPage::resetPage()
{


}
void APICandlesPage::slotRemoveFavorAsset(const QString &ticker)
{
    qDebug("APICandlesPage::slotRemoveFavorAsset()");
    int pos = recIndexByTicker(ticker);
    if (pos < 0)
    {
        emit signalError(QString("Asset [%1] not found in favor list").arg(ticker));
        return;
    }

    m_data.removeAt(pos);
    reloadAssetTable();
    rewriteDataFile();
}
void APICandlesPage::slotAddFavorAsset(const AssetFavorRecord &rec)
{
    qDebug("APICandlesPage::slotAddFavorAsset()");
    if (recIndexByTicker(rec.ticker) >= 0)
    {
        emit signalError(QString("Asset [%1] already exist in favor list").arg(rec.ticker));
        return;
    }

    m_data.append(rec);
    reloadAssetTable();
    rewriteDataFile();
}
void APICandlesPage::reloadAssetTable()
{
    qDebug("APICandlesPage::reloadAssetTable()");
    if (!m_assetTable) return;

    QTableWidget *t = m_assetTable->table();
    LTable::removeAllRowsTable(t);
    if (m_data.isEmpty()) return;

    QStringList row_data;
    foreach (const AssetFavorRecord &rec, m_data)
    {
        row_data.clear();
        row_data << rec.name << rec.ticker << rec.finish_date;
        row_data << QString::number(rec.d_coupon) << rec.rating;
        LTable::addTableRow(t, row_data);
    }
    //m_assetTable->resizeByContents();

    qDebug("APICandlesPage::reloadAssetTable() end");

    m_assetTable->searchExec();
}
void APICandlesPage::initDownloader()
{
    m_downloader = new MoexBondHistoryDownloader(this);
    m_downloader->setPath(historyPath());

    connect(m_downloader, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_downloader, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_downloader, SIGNAL(finished(QString)), this, SLOT(slotDownloadlerFinished(QString)));


}
void APICandlesPage::reinitWidgets()
{
    // m_candleTable
    m_candleTable = new LSearchTableWidgetBox(this);
    m_candleTable->setTitle("Candles history");

    QStringList headers;
    headers << "Open date" << "Open" << "Close" << "Low" << "High" << "Volume";
    m_candleTable->setHeaderLabels(headers);
    m_candleTable->resizeByContents();
    m_candleTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_candleTable->setSelectionColor("#BDFF4F");
    h_splitter->addWidget(m_candleTable);


    // m_assetTable
    m_assetTable = new LSearchTableWidgetBox(this);
    m_assetTable->setTitle("Favorite assets");
    m_assetTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_assetTable->setSelectionColor("#DDF4FF");
    m_assetTable->vHeaderHide();

    headers.clear();
    headers << "Company" << "Ticker" << "Finish date" << "Coupon, d" << "Rating";
    m_assetTable->setHeaderLabels(headers);
    m_assetTable->resizeByContents();
    h_splitter->addWidget(m_assetTable);


    // chart
    m_chart = new LChartWidget(this);
    v_splitter->addWidget(m_chart);


}
void APICandlesPage::loadDataFile()
{
    qDebug("APICandlesPage::loadDataFile()");

    QString fname(dataFile().trimmed());
    emit signalMsg(QString("try loading favorite assets list [%1]").arg(fname));

    QStringList list;
    QString err = LFile::readFileSL(fname, list);
    if (!err.isEmpty())  { emit signalError(err);  return; }

    LString::removeEmptyStrings(list);
    if (list.isEmpty()) { emit signalError("file data is empty");  return; }

    foreach (const QString &v, list)
    {
        loadFileLine(v);
    }

    emit signalMsg(QString("loaded %1 favorite asset records").arg(m_data.count()));
    qDebug()<<QString("loaded %1 recs").arg(m_data.count());
}
void APICandlesPage::loadFileLine(const QString &fline)
{
    QStringList list = LString::trimSplitList(fline, "/");
    if (list.count() != 5) {qWarning()<<QString("APICandlesPage::loadFileLine WARNING invalid fline [%1]").arg(fline); return;}

    bool ok;
    AssetFavorRecord rec;
    rec.name = list.at(0);
    rec.ticker = list.at(1);
    rec.finish_date = list.at(2);
    rec.d_coupon = list.at(3).toFloat(&ok);
    rec.rating = list.at(4);

    if (!ok)
    {
        qWarning()<<QString("APICandlesPage::loadFileLine WARNING can't parse d_coupon valie [%1]").arg(list.at(3));
        rec.d_coupon = -1;
    }
    m_data.append(rec);
}
void APICandlesPage::rewriteDataFile()
{
    qDebug("APICandlesPage::rewriteDataFile()");
    QString fname(dataFile().trimmed());
    emit signalMsg(QString("try rewrite favorite assets file [%1]").arg(fname));
    if (m_data.isEmpty()) {signalError("records list is empty"); return;}

    QStringList list;
    foreach (const AssetFavorRecord &rec, m_data)
    {
        QString fline = QString("%1 / %2 / %3").arg(rec.name).arg(rec.ticker).arg(rec.finish_date);
        fline = QString("%1 / %2 / %3").arg(fline).arg(QString::number(rec.d_coupon, 'f', 2)).arg(rec.rating);
        list.append(fline);
    }

    QString err = LFile::writeFileSL(fname, list);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg(QString("done, wrote %1 records").arg(m_data.count()));

}
int APICandlesPage::recIndexByTicker(const QString &ticker) const
{
    if (m_data.isEmpty()) return -1;

    int  n = m_data.count();
    for (int i=0; i<n; i++)
    {
        if (m_data.at(i).ticker == ticker) return i;
    }
    return -1;
}
void APICandlesPage::initPopupMenu()
{
    QString i_path = QString(":/icons/images");


    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Update chart", QString("%1/view-refresh.svg").arg(i_path));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Download history", QString("%1/candle.png").arg(i_path));
    act_list.append(pair2);

    //init popup menu actions
    m_assetTable->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_assetTable->connectSlotToPopupAction(i_menu, this, SLOT(slotRepaintChart())); i_menu++;
    m_assetTable->connectSlotToPopupAction(i_menu, this, SLOT(slotDownloadCandles())); i_menu++;

}
void APICandlesPage::slotRepaintChart()
{
    qDebug("APICandlesPage::slotRepaintChart()");
    if (!m_downloader) return;

}
void APICandlesPage::slotDownloadCandles()
{
    qDebug("APICandlesPage::slotDownloadCandles()");
    if (!m_downloader) return;

    int row = LTable::selectedRows(m_assetTable->table()).first();

    enableControls(false);
    m_downloader->setTicker(m_data.at(row).ticker);
    qDebug()<<QString("m_downloader ticker %1, path %2").arg(m_downloader->ticker()).arg(m_downloader->path());

    m_downloader->run();
}
void APICandlesPage::slotDownloadlerFinished(QString msg)
{
    if (msg.trimmed().isEmpty()) qWarning("APICandlesPage::slotDownloadlerFinished WARNING can't download history file");
    else qDebug()<<QString("APICandlesPage::slotDownloadlerFinished downloed file OK - %1").arg(msg);

    emit signalMsg(QString("%0 downloader finished").arg(LTime::strCurrentTime()));

    enableControls(true);
}
void APICandlesPage::enableControls(bool b)
{
    m_assetTable->setEnabled(b);
    m_candleTable->setEnabled(b);

}



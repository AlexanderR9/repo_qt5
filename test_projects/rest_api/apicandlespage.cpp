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


#define FAVOR_LIST_FILE         QString("favorite.txt")
#define HISTORY_FOLDER          QString("candles")
#define RSI_LEN                 30
#define PRICE_DEVIATION_COL     6
#define DEVIATION_LIGHT         0.8
#define RSI_COL                 8
#define LOW_COL                 3
#define HIGH_COL                LOW_COL+1
//#define PERCENT_PRECISION       1


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
    m_history.clear();

    reinitWidgets();
    initDownloader();
    initChart();

    // init popup
    initPopupMenu();

}
void APICandlesPage::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    loadFavorDataFile();
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
    m_assetTable->searchExec();
}
void APICandlesPage::reloadHistoryTable()
{
    QTableWidget *t = m_candleTable->table();
    if (m_history.isEmpty()) return;

    QStringList row_data;
    foreach (const HistoryCandle24 &rec, m_history)
    {
        row_data.clear();
        row_data << rec.strTS() << rec.strOpen() << rec.strClose();
        row_data << rec.strLow() << rec.strHigh() << QString::number(rec.volume);
        row_data << QString() << QString() << QString();
        LTable::addTableRow(t, row_data);
    }

    calcDeviationPrice(t);
    calcRSI(t);
    lightUpLow(t);
    lightUpHigh(t);

    //m_candleTable->resizeByContents();
    m_candleTable->searchExec();
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
    headers << "Open date" << "Open" << "Close" << "Low" << "High" << "Volume" <<
                "D Open/High" << "D Low/High" << "RSI, %";
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

}
void APICandlesPage::initChart()
{
    m_chart = new LChartWidget(this);
    v_splitter->addWidget(m_chart);

    m_chart->setCrossColor(QColor(150, 190, 150));
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    m_chart->setCrossXAxisTextViewMode(2);
    m_chart->setPointSize(2);

    LChartParams p;
    p.lineColor = QColor(170, 50, 40);
    p.pointsColor = QColor(0, 100, 0);
    p.points.clear();
    m_chart->addChart(p);
    m_chart->updateAxis();

}
void APICandlesPage::loadFavorDataFile()
{
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
    m_assetTable->connectSlotToPopupAction(i_menu, this, SLOT(slotReloadHistoryFile())); i_menu++;
    m_assetTable->connectSlotToPopupAction(i_menu, this, SLOT(slotDownloadCandles())); i_menu++;

}
void APICandlesPage::slotReloadHistoryFile()
{
    m_chart->clearChartPoints();
    m_chart->updateAxis();
    m_history.clear();
    m_candleTable->removeAllRows();

    int row = LTable::selectedRows(m_assetTable->table()).first();
    emit signalMsg(QString("try loading history file ....."));
    loadHistoryFile(m_data.at(row).ticker);
    emit signalMsg(QString("loaded %1 candles").arg(m_history.count()));

    reloadHistoryTable();
    repaintChart();
}
void APICandlesPage::slotDownloadCandles()
{
    if (!m_downloader) return;

    int row = LTable::selectedRows(m_assetTable->table()).first();

    enableControls(false);
    m_downloader->setTicker(m_data.at(row).ticker);
   // qDebug()<<QString("m_downloader ticker %1, path %2").arg(m_downloader->ticker()).arg(m_downloader->path());

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
void APICandlesPage::loadHistoryFile(QString ticker)
{
    ticker = ticker.trimmed();
    if (ticker.isEmpty()) {emit signalError("ticker is empty"); return;}

    QString fname = QString("%1%2/%3.txt").arg(historyPath()).arg(QDir::separator()).arg(ticker);
    emit signalMsg(QString("FILE [%1]").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError(QString("history file not found [%1]").arg(fname));
        return;
    }

    // read file
    QStringList list;
    QString err = LFile::readFileSL(fname, list);
    if (!err.isEmpty()) {emit signalError(err); return;}
    if (list.isEmpty()) {emit signalError("history file data is empty"); return;}


    // find start index of line data
    int start_row = 0;
    while (2 > 1)
    {
        if (start_row >= list.count()) {start_row = -1; break;}
        QString s = list.at(start_row).trimmed().toLower();
        start_row++;
        if (s.contains("open") && s.contains("close")) break;
    }
    if (start_row < 0) {emit signalError("invalid history file data"); return;}

    // parse candles lines
    for (int i=start_row; i<list.count(); i++)
    {
        HistoryCandle24 candle;
        candle.loadFileLine(list.at(i));
        if (candle.invalid()) qWarning()<<QString("candle line is invalid [%1]").arg(list.at(i));
        else m_history.insert(0, candle);
    }
    if (m_history.count() > 5) m_history.removeFirst();
}
void APICandlesPage::repaintChart()
{
    if (m_history.isEmpty()) return;

    // prepare points
    QList<QPointF> points;
    for (int i=m_history.count()-1; i>=0; i--)
    {
        QDateTime dt(m_history.at(i).ts_open);
        QPointF p(dt.toSecsSinceEpoch(), m_history.at(i).open);
        points.append(p);
    }

    //update chart widget
    float a = 0;
    foreach (const QPointF &p, points) a += p.y();
    a /= points.count(); // calc average y-value

    m_chart->addChartPoints(points, 0);
    emit signalMsg(m_chart->strMinMax());
    m_chart->setAxisPrecision(-1, 2);
    m_chart->updateAxis();

}
void APICandlesPage::calcDeviationPrice(QTableWidget *t)
{
    if (m_history.isEmpty() || !t) return;

    for (int i=0; i<m_history.count(); i++)
    {
        const HistoryCandle24 &rec = m_history.at(i);

        float d = rec.high - rec.open;
        t->item(i, PRICE_DEVIATION_COL)->setText(QString("%1 %").arg(QString::number(d, 'f', 2)));
        if (d > DEVIATION_LIGHT) t->item(i, PRICE_DEVIATION_COL)->setTextColor("#0000dd");

        d = rec.high - rec.low;
        t->item(i, PRICE_DEVIATION_COL+1)->setText(QString("%1 %").arg(QString::number(d, 'f', 2)));
        if (d > DEVIATION_LIGHT) t->item(i, PRICE_DEVIATION_COL+1)->setTextColor("#0000dd");
    }
}
void APICandlesPage::calcRSI(QTableWidget *t)
{
    if (m_history.isEmpty() || !t) return;

    for (int i=0; i<m_history.count(); i++)
    {
        float i_rsi = calcRSI_ps(i);

        t->item(i, RSI_COL)->setText(QString::number(i_rsi, 'f', 1));
        if (i_rsi < 0) t->item(i, RSI_COL)->setTextColor("#dddddd");
        if (i_rsi > 70) t->item(i, RSI_COL)->setTextColor("#aa0000");
    }
}
float APICandlesPage::calcRSI_ps(int i) const
{
    float i_close = m_history.at(i).close; // current point
    QPair<float, float> min_max;
    min_max.first = min_max.second = 0;

    int n = 0;
    while (2 > 1)
    {
        i++;
        if (m_history.count() <= i) return -1;

        const HistoryCandle24 &rec = m_history.at(i);
        if (rec.high > min_max.second) min_max.second = rec.high;
        if (rec.low < min_max.first || n == 0) min_max.first = rec.low;

        n++;
        if (n == RSI_LEN) break;
    }



    float band_size = min_max.second - min_max.first;
    //qDebug()<<QString("min_max  %1/%2,  cur %3,  band_size=%4").arg(min_max.first).arg(min_max.second).arg(i_close).arg(band_size);

    float cur_v = i_close - min_max.first;
    if (band_size <= 0) return -1;

    return float(100)*(cur_v/band_size);
}
void APICandlesPage::lightUpLow(QTableWidget *t)
{
    const int n = 15;
    if (m_history.count() < n || !t) return;

    const int nm = 3;
    QList<int> min_idexes;
    for (int i=0; i<nm; i++)
    {
        float min = 1000000;
        int i_min = -1;
        for (int j=0; j<n; j++)
        {
            if (min_idexes.contains(j)) continue;

            const HistoryCandle24 &rec = m_history.at(j);
            if (rec.low < min) {min = rec.low; i_min = j;}
        }
        min_idexes.append(i_min);
    }

    for (int i=0; i<nm; i++)
    {
        int row = min_idexes.at(i);
        t->item(min_idexes.at(i), LOW_COL)->setTextColor(Qt::red);
        float p_low = m_history.at(row).low * float(10);
        t->item(row, LOW_COL)->setToolTip(QString::number(p_low, 'f', 1));

    }
}
void APICandlesPage::lightUpHigh(QTableWidget *t)
{
    const int n = 15;
    if (m_history.count() < n || !t) return;

    const int nm = 3;
    QList<int> max_idexes;
    for (int i=0; i<nm; i++)
    {
        float max = -1;
        int i_max = -1;
        for (int j=0; j<n; j++)
        {
            if (max_idexes.contains(j)) continue;

            const HistoryCandle24 &rec = m_history.at(j);
            if (rec.high > max) {max = rec.high; i_max = j;}
        }
        max_idexes.append(i_max);
    }

    for (int i=0; i<nm; i++)
    {
        int row = max_idexes.at(i);
        t->item(row, HIGH_COL)->setTextColor(Qt::darkGreen);
        float p_high = m_history.at(row).high * float(10);
        t->item(row, HIGH_COL)->setToolTip(QString::number(p_high, 'f', 1));
    }
}



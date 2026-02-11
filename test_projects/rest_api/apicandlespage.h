#ifndef APICANDLESPAGE_H
#define APICANDLESPAGE_H


#include "apipages.h"
#include "instrument.h"


struct AssetFavorRecord;
class LChartWidget;
class MoexBondHistoryDownloader;


// APICandlesPage
class APICandlesPage : public LSimpleWidget
{
    Q_OBJECT
public:
    APICandlesPage(QWidget*);
    virtual ~APICandlesPage() {}

    void resetPage();

    QString iconPath() const {return QString(":/icons/images/chart.svg");}
    QString caption() const {return QString("Candles");}

    //сохранение/восстановление сплитеров
    void load(QSettings&);
    //void save(QSettings&) {}


    static QString dataFile();
    static QString historyPath();

protected:
    LSearchTableWidgetBox       *m_assetTable;
    LSearchTableWidgetBox       *m_candleTable;
    LChartWidget                *m_chart;
    MoexBondHistoryDownloader   *m_downloader;

    QList<AssetFavorRecord> m_data;
    QList<HistoryCandle24> m_history;


    void reinitWidgets();
    void initDownloader();
    void initChart();

    void loadFavorDataFile();
    void loadFileLine(const QString&);
    void rewriteDataFile();
    void reloadAssetTable();
    void reloadHistoryTable();
    int recIndexByTicker(const QString&) const;
    void enableControls(bool);
    void repaintChart();

    void loadHistoryFile(QString);

    void initPopupMenu(); //инициализировать элементы всплывающего меню

public slots:
    void slotRemoveFavorAsset(const QString&);
    void slotAddFavorAsset(const AssetFavorRecord&);

protected slots:
    void slotReloadHistoryFile();
    void slotDownloadCandles();
    void slotDownloadlerFinished(QString);


};



#endif // APICANDLESPAGE_H




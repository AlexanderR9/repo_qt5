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
    LSearchTableWidgetBox   *m_assetTable;
    LSearchTableWidgetBox   *m_candleTable;
    LChartWidget            *m_chart;
    MoexBondHistoryDownloader   *m_downloader;

    QList<AssetFavorRecord> m_data;

    void reinitWidgets();
    void initDownloader();

    void loadDataFile();
    void loadFileLine(const QString&);
    void rewriteDataFile();
    void reloadAssetTable();
    int recIndexByTicker(const QString&) const;
    void enableControls(bool);

    void initPopupMenu(); //инициализировать элементы всплывающего меню

public slots:
    void slotRemoveFavorAsset(const QString&);
    void slotAddFavorAsset(const AssetFavorRecord&);

protected slots:
    void slotRepaintChart();
    void slotDownloadCandles();
    void slotDownloadlerFinished(QString);


};



#endif // APICANDLESPAGE_H




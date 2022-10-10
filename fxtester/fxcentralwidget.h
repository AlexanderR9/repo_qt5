#ifndef FX_CENTRAL_WIDGET_H
#define FX_CENTRAL_WIDGET_H

#include "lsimplewidget.h"

#include <QMap>


class FXDataLoader;
class QStackedWidget;
class FXDataLoaderWidget;
class QListWidget;
class QSettings;
struct FXCoupleDataParams;
class FXBarContainer;
struct FXChartSettings;

// FXCentralWidget
class FXCentralWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    enum FXPageType {fxptChart = 170, fxptTester, fxptQualData};

    FXCentralWidget(QWidget *parent = 0);
    virtual ~FXCentralWidget() {clearPages();}

    void loaderDataUpdate(const FXDataLoader*); //обновить поле с информацией о загруженных данных
    void setChartSettings(const FXChartSettings&);
    void load(QSettings&);
    void save(QSettings&);
    int currentPageType() const; //вернет тип открытой страницы из множества FXPageType
    void checkQualData(); //запуск проверки качества данных
    void startTesting(); //запуск тестирования по всем загруженным инструментам


protected:
    QStackedWidget          *m_stackedWidget;
    FXDataLoaderWidget      *m_loaderWidget;
    QListWidget             *m_pagesListWidget;

    QMap<int, LSimpleWidget*> m_pages;

    void initWidgets();
    void createPages();
    void clearPages();
    void clearStackedWidget();

protected slots:
    void slotSelectionDataChanged(); //выполняется когда изменился выделенный набор строк в m_loaderWidget
    void slotSelectedPageChanged();  //выполняется когда в m_pagesListWidget выбрали другую страницу

signals:
    void signalGetLoadedDataByReq(const QList<FXCoupleDataParams>&, QList<const FXBarContainer*>&);


};




#endif //FX_CENTRAL_WIDGET_H


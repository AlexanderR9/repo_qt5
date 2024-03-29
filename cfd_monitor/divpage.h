#ifndef DIVPAGE_H
#define DIVPAGE_H

#include "basepage.h"
#include "ui_chartpage.h"

#include <QDate>
#include <QMap>
#include <QDateTime>
#include <QTableWidget>

class LSearch;
class QListWidgetItem;
class QMouseEvent;
class QCheckBox;


//DivRecord
struct DivRecord
{
    DivRecord() {reset();}
    DivRecord(const DivRecord&);

    QString ticker;
    QDate ex_date;
    double size;
    double size_p; //%
    double price;

    static quint8 fieldsCount() {return 5;}

    void reset() {size = size_p = price = -1; ticker.clear();}
    bool invalid() const {return (size < 0.1 || size_p < 0.1 || ticker.isEmpty() || !ex_date.isValid());}
    QString toFileLine() const;
    void fromFileLine(const QString&);
    bool isEqual(const DivRecord &rec) const {return (ex_date == rec.ex_date && ticker == rec.ticker);}
    QString toStr() const;

};


//DivTable
class DivTable : public QTableWidget
{
    Q_OBJECT
public:
    DivTable(QWidget*);
    virtual ~DivTable() {}

    void addRecord(const DivRecord&);
    inline void setLightValues(const double &limit_div, const double &limit_price) {m_lightDivSize = limit_div; m_lightPrice = limit_price;}
    void getTableTitle(QString&);
    void updateRowVisible(int, const QStringList&, bool);
    void showAllRows(bool);
    void updatePrice(int, const double&);
    bool isInstaRow(int) const;

protected:
    double m_lightDivSize; //значения дивов (в %), выше которого необходимо подсвечивать
    double m_lightPrice; //значения цены, выше которого необходимо подсвечивать


    void mouseDoubleClickEvent(QMouseEvent*);

    void updateColors(const DivRecord&);
    int daysTo(const QDate&) const;

signals:
    void signalDoubleClicked();
    void signalGetInstaPtr(const QString&, bool&);

};


//DivPage
class DivPage : public BasePage, Ui::ChartPage
{
    Q_OBJECT
public:
    DivPage(QWidget*);
    virtual ~DivPage() {}

    QString iconPath() const {return QString(":/icons/images/list-add");}
    QString caption() const {return QString("Divs");}

    static QString divFile() {return QString("div_data.txt");}

    void updatePage() {}
    void initSource();
    void setReqParams(const QString&, int, quint16);
    void setTickTimerInterval(int);
    void tickTimerStart();
    void tickTimerStop();

    inline void setLightValues(const double &limit_div, const double &limit_price) {if (m_table) m_table->setLightValues(limit_div, limit_price);}
    inline void setShownHistory(quint16 n) {m_shownHistory = n;}


protected:
    LSearch     *m_search;
    DivTable    *m_table;
    QCheckBox   *m_onlyInstaCheckBox;
    QDateTime   m_lastDT;
    QTimer      *m_timer; //таймер стартует и работает сразу при запуске
    QString     m_url;
    int         m_interval; //интервал опроса m_url, сек
    quint16     m_shownHistory;
    quint16     m_lookDays; //за сколько дней вперед просматривать инфу о дивах
    quint16     m_tablePriceIndex; //индекс строки таблицы в которой будет обновлена цена в очередном цикле tick_timer

    void initSearch();
    void initTable();
    void searchExec();
    void sendLog(const QString&, int);
    inline bool invalidParams() const {return(m_url.isEmpty() || m_interval < 0);}
    void parseDate(const QString&, QDate&); //если строка является датой, то в QDate запишеться считанная дата, инача QDate будет invalid
    void parseDivSize(const QString&, DivRecord&);
    void divDataReceived(const QList<DivRecord>&); //синхронизировать только что полученные данные с текущими хранящамися в файле-divFile()
    void updateLastPrices(QList<DivRecord>&); //получить текущие цены для только что полученных тикеров из данных с дивами и записать их в соответствующие записи
    void loadDivFile(QList<DivRecord>&); //загрузить данные из файла в контейнер
    void updateFileByReceivedData(const QList<DivRecord>&); //добавить при необходимости новые данные в файл, если таких еще нет
    void addRecToFile(const DivRecord&);
    void reloadTable(const QList<DivRecord>&);
    void updateTable();
    void sortData(QList<DivRecord>&);
    void replaceRecords(int, int, QList<DivRecord>&); //меняет местами две записи на указанных позициях в указанном контейнере
    void updateTableNextPrice(); //обновление цены в строке m_tablePriceIndex


protected slots:
    void slotSelectionChanged();
    void slotTimer();

public slots:
    void slotDivDataReceived(const QString&); //получены текстовые данные со страницы html, который необходимо распарсить и извлель данные по дивам

signals:
    void signalGetSource(QStringList&);
    void signalGetDivData(const QString&);
    void signalGetCurrentPrices(QMap<QString, double>&); //key - ticker,  value - cur_price
    void signalGetInstaPtr(const QString&, bool&);
    void signalGetLastPrice(const QString&, double&, int&); //получение последней цены для заданного тикера

private:
    void testDivDataFromFile(); //имитация запроса дивов, вместо ответа html данные считаваются из статического файла


};


#endif //DIVPAGE_H



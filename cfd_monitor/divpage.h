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

protected:
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
    void setReqParams(const QString&, int);
    inline void setShownHistory(quint16 n) {m_shownHistory = n;}

protected:
    LSearch     *m_search;
    DivTable    *m_table;
    QDateTime   m_lastDT;
    QTimer      *m_timer;
    QString     m_url;
    int         m_interval; //интервал опроса m_url, сек
    quint16     m_shownHistory;

    void initSearch();
    void initTable();
    void searchExec();
    void sendLog(const QString&, int);
    inline bool invalidParams() const {return(m_url.isEmpty() || m_interval < 0);}
    void parseDate(const QString&, QDate&); //если строка является датой, то в QDate запишеться считанная дата, инача QDate будет invalid
    void parseDivSize(const QString&, DivRecord&);
    void divDataReceived(const QList<DivRecord>&);
    void updateLastPrices(QList<DivRecord>&);
    void loadDivFile(QList<DivRecord>&); //загрузить данные из файла в контейнер
    void updateFileByReceivedData(const QList<DivRecord>&); //добавить при необходимости новые данные в файл, если таких еще нет
    void addRecToFile(const DivRecord&);
    void reloadTable(const QList<DivRecord>&);
    void sortData(QList<DivRecord>&);
    void replaceRecords(int, int, QList<DivRecord>&); //меняет местами две записи



protected slots:
    void slotSelectionChanged();
    void slotTimer();

public slots:
    void slotDivDataReceived(const QString&);

signals:
    void signalGetSource(QStringList&);
    void signalGetDivData(const QString&);
    void signalGetCurrentPrices(QMap<QString, double>&); //key - ticker,  value - cur_price
    void signalGetInstaPtr(const QString&, bool&);

private:
    void testDivDataFromFile(); //имитация запроса дивов, вместо ответа html данные считаваются из статического файла


};


#endif //DIVPAGE_H



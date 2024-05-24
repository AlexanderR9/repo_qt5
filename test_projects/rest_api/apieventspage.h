#ifndef APIEVENTSPAGE_H
#define APIEVENTSPAGE_H

#include "apipages.h"
#include "instrument.h"


class QJsonObject;
class LSearchTableWidgetBox;
class QComboBox;
class QLineEdit;

//statistic operations
struct StatTotalSums
{
    StatTotalSums() {reset();}

    float coupon;
    float div;
    float commission;
    float tax;
    float input;
    float out;

    quint16 n_records;
    quint16 n_repayments;
    quint16 n_coupons;


    void reset() {coupon = div = commission = tax = input = out = 0;
                    n_records = n_repayments = n_coupons = 0;}

};


// APIEventsPage
class APIEventsPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIEventsPage(QWidget*);
    virtual ~APIEventsPage() {}

    void resetPage();

    QString iconPath() const {return QString(":/icons/images/event.png");}
    QString caption() const {return QString("Events");}

    //сохранение/восстановление сплитеров
    void load(QSettings&);
    void save(QSettings&);

    static QString dataFile();


protected:
    QList<EventOperation>    m_events;
    LTableWidgetBox         *m_statBox;
    QComboBox               *m_paperTypeFilterControl;
    QComboBox               *m_kindFilterControl;
    QComboBox               *m_dateFilterControl;
    QLineEdit               *m_paperResultEdit;
    StatTotalSums           m_stat;

    void reloadTableByData();
    void addRowRecord(const EventOperation&, const QPair<QString, QString>&, QColor);
    void reinitWidgets();
    void initFilterBox();
    void recalcStat();
    void updateStatStruct(const EventOperation&);
    void updateStatTable();
    void initPaperResultWidget();
    void recalcPaperResult();
    void loadDataFromFile(); //загрузка текущих записей из файла
    void syncRecByFile(const EventOperation&); //синхронизировать запись с текущими данными, если запись не найдена, то добавить в конец файла и в m_events
    void sortByDate();
    void checkCloneUid(EventOperation&); //проверить uid пришедшего события в ответе на предмена клона, если это клон, то подменить его на нормальный uid из базы  активов
    void findUnknownUID(); //после загрузки событий в таблицу выполнить поиск строк с пустыми тикерами (т.е. неизвестными UID) и подсветить их

private:
    void paperTypeFilter(int, bool&);
    void kindFilter(int, bool&);
    void dateFilter(int, bool&);
    void updateSearchLabel();

public slots:
    void slotLoadEvents(const QJsonObject&); //выполняется после успешного запроса получения истории операций
    void slotGetEventsHistoryByTicker(const QString&, QStringList&);

protected slots:
    void slotFilter();
    void slotSearched();

signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void signalGetTickerByFigi(const QString&, QString&);

};

#endif // APIEVENTSPAGE_H

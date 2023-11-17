#ifndef APIEVENTSPAGE_H
#define APIEVENTSPAGE_H

#include "apipages.h"
#include "instrument.h"


class QJsonObject;
class LSearchTableWidgetBox;
class QComboBox;

struct StatTotalSums
{
    StatTotalSums() {reset();}

    float coupon;
    float div;
    float commission;
    float tax;
    float input;
    float out;

    void reset() {coupon = div = commission = tax = input = out = 0;}

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


protected:
    QList<EventOperation>    m_events;
    LTableWidgetBox         *m_statBox;
    QComboBox               *m_paperTypeFilterControl;
    QComboBox               *m_kindFilterControl;
    QComboBox               *m_dateFilterControl;
    StatTotalSums           m_stat;

    void reloadTableByData();
    void addRowRecord(const EventOperation&, const QPair<QString, QString>&, QColor);
    void reinitWidgets();
    void initFilterBox();
    void recalcStat();
    void updateStatStruct(const EventOperation&);
    void updateStatTable();


private:
    void paperTypeFilter(int, bool&);
    void kindFilter(int, bool&);
    void dateFilter(int, bool&);

public slots:
    void slotLoadEvents(const QJsonObject&);

protected slots:
    void slotFilter();



signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void signalGetTickerByFigi(const QString&, QString&);

};

#endif // APIEVENTSPAGE_H

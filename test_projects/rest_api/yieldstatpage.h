#ifndef YIELDSTATPAGE_H
#define YIELDSTATPAGE_H

#include "apipages.h"
#include "instrument.h"

class QLineEdit;

// YieldStatPage
class YieldStatPage : public APITablePageBase
{
    Q_OBJECT
public:
    struct YieldRowData
    {
        YieldRowData() {reset();}

        QDate last_date;
        QString name;
        QString ticker;
        QString paper_type;
        int n_papers; //оборот по этой бумаге за всю истории
        int n_events; //сколько всего событий есть в истории по этой бумаге
        float yield; //итоговый полный доход по этой бумаге (включая дивы, купоны и комиссии)
        float coupon; //вспомогательная переменная
        float div; //вспомогательная переменная

        void reset() {n_papers=n_events=0; yield=coupon=div=0; name="?"; ticker="?";}
        inline bool isStock() const {return (paper_type == "share");}
        inline bool isBond() const {return (paper_type == "bond");}
    };

    YieldStatPage(QWidget*);
    virtual ~YieldStatPage() {}

    void resetPage();

    QString iconPath() const {return QString(":/icons/images/coin.svg");}
    QString caption() const {return QString("Yield");}

    inline float totalYield_k() const {return ((m_bondYield + m_stockYield)/float(1000));}

    //сохранение/восстановление сплитеров
   // void load(QSettings&);
   // void save(QSettings&);

protected:
    QList<QLineEdit*> m_editList;
    float m_bondYield;
    float m_stockYield;

    void initFilterBox();
    void findStartDate(const QList<EventOperation>&);
    void setChildEditText(QString, QString);
    void addYieldRowData(const YieldRowData&);
    void recolorTable(); //применить раскраску для ячеек таблицы
    QDate bagStartDate() const; //вернет дату 1-й сделки портфеля
    void sortTableByDate(); //сортировка строк таблицы по дате, 1-я самая старая
    void recalcTotalYield();

    //расчет дохода по каждой бумаге, на каждую бумагу всего одно значение (сумма всех опрераций в истории)
    //расчет производится только по тем бумагам, которые полностью проданы и неучаствуют в портфеле в текущий момент.
    void recalcYield(const QList<EventOperation>&);
    void recalcYieldByUID(const QString&, const QList<EventOperation>&);
    void parseNextRec(const EventOperation&, YieldRowData&, int&, int&);


public slots:
    void slotReceivedEvents(const QList<EventOperation>&); //выполняется после успешного запроса получения истории операций

signals:
    void signalGetPaperInfo(QStringList&);


};




#endif // YIELDSTATPAGE_H

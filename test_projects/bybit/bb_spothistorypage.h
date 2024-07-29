#ifndef BB_SPOTHISTORYPAGE_H
#define BB_SPOTHISTORYPAGE_H

#include "bb_historypage.h"



//BB_SpotHistoryPage
class BB_SpotHistoryPage : public BB_HistoryPage
{
    Q_OBJECT
public:
    BB_SpotHistoryPage(QWidget*);
    virtual ~BB_SpotHistoryPage() {}

    virtual void load(QSettings&);

    QString caption() const {return QString("History (SPOT)");}

protected:
    QList<BB_HistorySpot> m_spotOrders;
    bool m_needRewriteFile;
    QLineEdit *m_amountEdit;

    virtual void loadTablesByContainers(); //by start program once
    virtual void goExchange(const QJsonObject &jresult_obj);
    virtual void loadContainers(); //load m_posList, m_orderList from files by start program
    virtual quint8 daysSeparator() const {return 6;}

    void sendOrdersReq();
    void sendNextOrdersReq();
    void parseOrders(const QJsonObject&);
    void insertSpotEvent(const BB_HistorySpot&);
    bool hasSpotEvent(const BB_HistorySpot&) const;
    void finishReqScenario();


    //make after finish ALL requests
    //void bringTogetherEvents(); //свести в одну позиции с одинаковыми ордерами
    void getNextPartIndexes(const QString&, int, QList<int>&); //найти по указанному orderId, остальные части разбитого ордера и записать их индексы в контейнер
    void sortByExecTime(); //сортировка m_spotOrders по времени срабатывания ордеров, 1-я самая свежая запись
    void rewriteHistoryFile(); //после окончания выполнения сценария запросов переписать файл полностью

    void reinitWidgets();
    void fillTable(const QJsonArray&);
    void updateLastRowColors(const BB_HistorySpot&);
    //void prepareLoadedDates();

private:
    QString hasDoubleUid() const;

protected slots:
    void slotSearched();


};




#endif // BB_SPOTHISTORYPAGE_H



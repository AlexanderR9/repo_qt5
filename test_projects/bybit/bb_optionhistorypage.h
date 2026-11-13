#ifndef BB_OPTION_HISTORYPAGE_H
#define BB_OPTION_HISTORYPAGE_H

#include "bb_historypage.h"

class QLineEdit;



//BB_OptionHistoryPage
class BB_OptionHistoryPage : public BB_HistoryPage
{
    Q_OBJECT
public:
    BB_OptionHistoryPage(QWidget*);
    virtual ~BB_OptionHistoryPage() {}

    virtual void load(QSettings&);

    QString caption() const {return QString("History (OPTION)");}

protected:
    QList<BB_HistoryOption> m_positions;
    bool m_needRewriteFile;
    QLineEdit *m_totalResultEdit;

    virtual void loadTablesByContainers(); //by start program once
    virtual void goExchange(const QJsonObject &jresult_obj);
    virtual void loadContainers(); //load m_posList, m_orderList from files by start program
    virtual quint8 daysSeparator() const {return 3;}

    void sendOrdersReq();
    void sendNextOrdersReq();
    void parseOrders(const QJsonObject&);
    bool hasSpotEvent(const BB_HistoryOption&) const;
    void reinitWidgets();
    void fillTable(const QJsonArray&);
    void updateLastRowColors(const BB_HistoryOption&);
    void recalcTotalResult();


    //make after finish ALL requests
    void sortByCloseTime(); //сортировка m_spotOrders по времени закрытия, 1-я самая свежая запись
    void rewriteHistoryFile(); //после окончания выполнения сценария запросов переписать файл полностью
    void finishReqScenario();

protected slots:
    void slotSearched();


};




#endif // BB_OPTIONHISTORYPAGE_H



#ifndef BB_OPTIONPAGE_H
#define BB_OPTIONPAGE_H

#include "bb_basepage.h"
#include "bb_apistruct.h"


class QJsonObject;
class QJsonArray;
class QJsonValue;
class QSettings;
struct TradeOperationData;


// BB_OptionPage
class BB_OptionPage : public BB_BasePage
{
    Q_OBJECT
public:
    BB_OptionPage(QWidget*);
    virtual ~BB_OptionPage() {}

    QString iconPath() const {return QString(":/icons/images/coins.png");}
    QString caption() const {return QString("Options");}


    void updateDataPage(bool force = false);

protected:
    LSearchTableWidgetBox     *m_table;
    LSearchTableWidgetBox  *m_monitTable;
    QList<BB_Option>        m_container;
    int                     m_polledDays;

    void initPopupMenu(); //инициализировать элементы всплывающего меню


    void init();
    void getTSNextInterval(qint64&, qint64&); // for getting history
    void setTSNextInterval(); // for getting history
    void parseInfoRecord(const QJsonObject&);
    void parsePriceRecord(const QJsonObject&);
    void sortPricesTable();
    void sortDayStrikes();
    void setColorInMoney(); // пометить строки которые сейчас в деньгах

    void updateInfoTable(); // переписать таблицу m_monitTable по m_container
    void rewriteFile();
    void loadContainerByFile();
    int findRecByTicker(const QString&) const;

    bool hasContainerTicker(const QString&) const;


private:
    void sendTradeReq(const TradeOperationData&);

public slots:
    void slotJsonReply(int, const QJsonObject&);


protected slots:
    void slotUpdateOptionsPrices();
    void slotOptionBuy();
    void slotOptionSell();


};







#endif // BB_OPTIONPAGE_H

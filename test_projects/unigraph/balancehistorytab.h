#ifndef BALANCEHISTORYTAB_H
#define BALANCEHISTORYTAB_H

#include "ug_basepage.h"

class LSearchTableWidgetBox;



//BalanceHistoryTab
class BalanceHistoryTab : public LSimpleWidget
{
    Q_OBJECT
public:
    BalanceHistoryTab(QWidget*);
    virtual ~BalanceHistoryTab() {}

    void reloadHistory(QString); //from file
    void setAssets(const QMap<QString, QString>&);

protected:
    LSearchTableWidgetBox     *m_table;
    QMap<QString, QString> m_walletAssets; //key - token_address, value -  ticker (only current chain)

    void initTable();
    void parseFileData(const QStringList&);
    void calcDeviationColumn(); // посчитать столбец с отклонениями баланса
    void calcDeviationByAddr(QString); // посчитать отклонения баланса для указанного актива

public slots:
    void slotBalancesUpdated(QString); //выполняется каждый раз при запросе текущих балансов из сети


};



#endif // BALANCEHISTORYTAB_H

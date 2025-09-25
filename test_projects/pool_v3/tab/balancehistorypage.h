#ifndef BALANCEHISTORY_TABPAGE_H
#define BALANCEHISTORY_TABPAGE_H

#include "basetabpage_v3.h"

class QJsonObject;
class WalletBalanceLogger;



//BalanceHistoryPage
class BalanceHistoryPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    BalanceHistoryPage(QWidget*);
    virtual ~BalanceHistoryPage() {}

    virtual void sendUpdateDataRequest() {};
    virtual void setChain(int);

protected:
    QStringList m_tokenList;
    WalletBalanceLogger *m_balanceObj;

    void initTable();
    void initTokenList(int); // загрузить список токенов из конфигурации для указанной сети
    void startSearch();
    void checkCurrentBalances(const QJsonObject&); // проанализировать значения текущих балансов и внести новые записи при необходимости
    void reloadLogToTable(); // внести весь журнал в таблицу
    void recalcDeviationCol();
    void updateTableColors(); // раскрасить таблицу


public slots:
    virtual void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs

protected slots:
    void slotAddNewRecord(float); //выполняется когда добавляется новая запись по приходу балансов из сети

};



#endif // BALANCEHISTORY_TABPAGE_H

#ifndef POOLS_TABPAGE_H
#define POOLS_TABPAGE_H

#include "basetabpage_v3.h"

class QJsonObject;


//DefiPoolsTabPage
class DefiPoolsTabPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiPoolsTabPage(QWidget*);
    virtual ~DefiPoolsTabPage() {}

    virtual void sendUpdateDataRequest() {};

    virtual void setChain(int);
    /*
    QString tickerByAddress(const QString&) const; // получить тикер по адресу из таблицы кошелька текущей сети
    virtual void updatePageBack(QString extra_data); // выполняется после анализа результа последней транзакции, происходит переход обратно на страницу и обновление ее.
    */

protected:
    void initTable();
    void initPoolList(int); // загрузить список пулов из конфигурации для указанной сети
    void initPopupMenu(); //инициализировать элементы всплывающего меню



    void updatePoolStateRow(const QJsonObject&);

    //check nodejs reply after TX
    void checkTxResult(QString, const QJsonObject&); // проанализировать ответ после попытки отправить очередную транзакцию
   // void logTxRecord(QString, const QJsonObject&); // отправить в журнал новую транзакцию

protected slots:
    void slotGetPoolState();
    void slotTxSwap();

public slots:
    virtual void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs



};



#endif // POOLS_TABPAGE_H


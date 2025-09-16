#ifndef APPROVE_TABPAGE_H
#define APPROVE_TABPAGE_H

#include "basetabpage_v3.h"

//class LTableWidgetBox;
class QJsonObject;
//struct JSTxLogRecord;



//DefiApproveTabPage
class DefiApproveTabPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiApproveTabPage(QWidget*);
    virtual ~DefiApproveTabPage() {}

    virtual void sendUpdateDataRequest() {};
    virtual void setChain(int);
    QString tickerByAddress(const QString&) const; // получить тикер по адресу из таблицы кошелька текущей сети

protected:
    void initTable();
    void initTokenList(int); // загрузить список токенов из конфигурации для указанной сети

    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void updateApprovedAmounts(const QJsonObject&);

    //check nodejs reply after TX
    void checkTxResult(QString, const QJsonObject&); // проанализировать ответ после попытки отправить очередную транзакцию
    void logTxRecord(QString, const QJsonObject&); // отправить в журнал новую транзакцию


protected slots:
    void slotGetApproved();
    void slotTxApprove();


public slots:
    virtual void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs


};



#endif // APPROVE_TABPAGE_H


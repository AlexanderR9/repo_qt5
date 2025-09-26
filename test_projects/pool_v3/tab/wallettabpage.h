#ifndef WALLETTABPAGE_H
#define WALLETTABPAGE_H

#include "basetabpage_v3.h"

class LTableWidgetBox;
class QJsonObject;
class LHttpApiRequester;
class WalletBalanceHistory;
struct JSTxLogRecord;



//DefiWalletTabPage
class DefiWalletTabPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiWalletTabPage(QWidget*);
    virtual ~DefiWalletTabPage() {}

    virtual void setChain(int);
    virtual void sendUpdateDataRequest(); //срабатывает по нажатию пользователем кнопки в тулбаре
    void updatePrices() const;

    QString tickerByAddress(const QString&) const; // получить тикер по адресу из таблицы кошелька текущей сети
    virtual void updatePageBack(QString); // выполняется после анализа результа последней транзакции, происходит переход обратно на страницу и обновление ее.


protected:
    void initTable();
    void updateAmounts(const QJsonObject&);
    void updateIntegratedTable(QString, const QJsonObject&);

    void updateBalance(int) const;
    void updateTotalBalance() const;
    void initTokenList(int); // загрузить список токенов из конфигурации для указанной сети
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    bool hasBalances() const; // проверка что балансы кошелька были получены

    //check nodejs reply after TX
    void checkTxResult(QString, const QJsonObject&); // проанализировать ответ после попытки отправить очередную транзакцию
    void logTxRecord(QString, const QJsonObject&); // отправить в журнал новую транзакцию

public slots:
    void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs
    void slotSetTokenBalance(QString, float&) const;  //запись по запросу другой страницы текущего баланса токена в кошельке по его тикеру.


protected slots:
    void slotWrap();
    void slotUnwrap();
    void slotTransfer();
    void slotGetTxCount();
    void slotGetGasPrice();
    void slotGetChainID();
    void slotGetAssetPrices() {emit signalGetPrices();}

signals:
    void signalGetPrices();

};



#endif // WALLETTABPAGE_H

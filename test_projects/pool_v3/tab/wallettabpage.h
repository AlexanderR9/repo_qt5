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

protected:
    void initTable();
    void updateAmounts(const QJsonObject&);
    void updateIntegratedTable(QString, const QJsonObject&);

    void updateBalance(int) const;
    void updateTotalBalance() const;
    void initTokenList(int); // загрузить список токенов из конфигурации для указанной сети
    void initPopupMenu(); //инициализировать элементы всплывающего меню

public slots:
    void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs

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

    /*
    WalletBalanceHistory    *m_balanceHistory;

    void addTokenToTable(const QStringList&);
    void loadAssetIcons();
    void updateTokenBalance(const QString&, const QString&);
    void updateBalances(const QJsonObject&);
    void lookTxAnswer(const QJsonObject&);
    void initHttpRequester();
    void sendHttpReq();
    void parseHttpResponse(const QJsonObject&);
    void initBalanceHistoryObj();
    void loadAssetsFromFile(); //загрузить список активов из файла
    void sendTxRecordToLog(const QJsonObject&); //подготовить и отправить запись о выполненной транзакции в JSTxLogger для добавления в журнал


private:
    void initNativeToken();
    void prepareCoingeckoParams(QString&);
    void updatePriceItem(QString, double);
    float findTokenPrice(QString) const; //ищет текущую цену токена в таблице, параметр может быть как имя токена так и адрес, в случае ошибки вернет -1



public slots:
    void slotScriptBroken();
    void slotGetTokenPrice(const QString&, float&) const;
    void slotGetChainName(QString&);

signals:
    void signalWalletTx(const QStringList&);
    void signalBalancesUpdated(QString);
    void signalChainUpdated();
    void signalSendTxLog(const JSTxLogRecord&);
    //void signalStartTXDelay();
    */


};



#endif // WALLETTABPAGE_H

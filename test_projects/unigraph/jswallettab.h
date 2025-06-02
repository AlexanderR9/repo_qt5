#ifndef JSWALLETTAB_H
#define JSWALLETTAB_H

#include "ug_basepage.h"

class LTableWidgetBox;
class QJsonObject;
class LHttpApiRequester;
class WalletBalanceHistory;
struct JSTxLogRecord;



//JSWalletTab
class JSWalletTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSWalletTab(QWidget*);
    virtual ~JSWalletTab() {}

    void getBalacesArgs(QStringList&); // список аргументв для получения балансов активов
    int assetsCount() const;
    QMap<QString, QString> assetsTokens() const; //key - ticker, value - token_address
    QMap<QString, float> assetsBalances() const; //key - token_address, value - cur_balance
    void parseJSResult(const QJsonObject&);
    void updateChain(); //вызвать скрипт, который вернет название текущей сети

    inline QString chainName() const {return m_currentChain;}

protected:
    LTableWidgetBox     *m_table;
    LHttpApiRequester   *m_priceRequester;
    WalletBalanceHistory    *m_balanceHistory;
    QString m_currentChain; //название сети в которой идет работа

    void initTable();
    void addTokenToTable(const QStringList&);
    void loadAssetIcons();
    void updateTokenBalance(const QString&, const QString&);
    void initPopupMenu(); //инициализировать элементы всплывающего меню
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


protected slots:
    void slotWrap();
    void slotUnwrap();
    void slotTransfer();
    void slotHttpReqFinished(int);

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


};



#endif // JSWALLETTAB_H

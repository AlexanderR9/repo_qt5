#ifndef APPCOMMONSETTINGS_H
#define APPCOMMONSETTINGS_H

#include <QString>

// типы страниц таба для одной сети
enum DefiPageKind {dpkWallet = 730, dpkBalance, dpkApproved, dpkPool, dpkPositions, dpkTx};


//описание всех возможных запросов к сети на чтение
enum NodejsReadCommand {nrcBalance = 830, nrcTXCount, nrcApproved, nrcGasPrice, nrcChainID,
                        nrcTXStatus, nrcPositions, nrcPoolState, nrcInvalid = -1};

//TX Orepation Type
enum NodejsTxCommand {txWrap = 870, txUnwrap, txApprove, txTransfer, txSwap,
                       txMint, txIncrease, txDecrease, txCollect, txBurn, txInvalid = -1};



// AppCommonSettings
class AppCommonSettings
{
public:
    static QString appDataPath();
    static double tickKwant() {return 1.0001;}
    static QString nativeTokenAddress() {return QString("0xeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");}
    static quint8 nativeTokenDecimal() {return 18;}
    static QString commonIconsPath() {return QString(":/icons/images");} //базовый путь к иконкам проекта
    static QString tabPageTitle(int); //заголовок страницы таба по ее типу
    static QString bbGetPricesUri() {return QString("v5/market/tickers?category=linear");}
    static quint8 interfacePricePrecision(float);
    static QString txListFile() {return "tx_list.txt";}
    static QString txStateFile() {return "tx_state.txt";}
    static QString txDetailsFile() {return "tx_details.txt";}
    static QString priceLogFile() {return "prices_history.log";}
    static QString balanceLogFile() {return "balance_history.log";}
    static QString timeUserMask() {return "hh:mm:ss";}
    static QString dateUserMask() {return "dd.MM.yyyy";}

    // nodejs user vars
    static QString nodejsPath() {return m_nodejsPath;}
    static QString curChainNodeJSFile() {return "current_chain.json";}
    static QString walletAssetsNodeJSFile() {return "tokens.json";}
    static QString readParamsNodeJSFile() {return "read_params.json";}
    static QString txParamsNodeJSFile() {return "tx_params.json";}
    static QString nodejsReqFieldName() {return "req_name";} //название основного поля в параметрах nodejs скрипта
    static QString nodejsTxSimulateFieldName() {return "simulate_mode";} //название поля в параметрах nodejs для указания режима симуляции TX
    static QString nodejsTxHashFieldName() {return "tx_hash";} //название поля в ответе nodejs которое содержит хеш отправленной TX

    //setters
    static void setNodejsPath(QString p) {m_nodejsPath = p.trimmed();}

protected:
    static QString m_nodejsPath;


};



#endif // APPCOMMONSETTINGS_H



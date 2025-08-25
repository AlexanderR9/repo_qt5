#ifndef APPCOMMONSETTINGS_H
#define APPCOMMONSETTINGS_H

#include <QString>

// типы страниц таба для одной сети
enum DefiPageKind {dpkWallet = 730, dpkApproved, dpkPool, dpkTx};


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
    static QString nodejsPath() {return m_nodejsPath;}
    static QString curChainNodeJSFile() {return "current_chain.json";}
    static QString walletAssetsNodeJSFile() {return "tokens.json";}
    static QString readParamsNodeJSFile() {return "read_params.json";}
    static QString bbGetPricesUri() {return QString("v5/market/tickers?category=linear");}
    static quint8 interfacePricePrecision(float);
    static QString nodejsReqFieldName() {return "req_name";} //название основного поля в параметрах nodejs скрипта

    //setters
    static void setNodejsPath(QString p) {m_nodejsPath = p.trimmed();}

protected:
    static QString m_nodejsPath;


};



#endif // APPCOMMONSETTINGS_H



#ifndef SUB_COMMONSETTINGS_H
#define SUB_COMMONSETTINGS_H

#include <QMap>
#include <QStringList>


class QDomNode;
class QSettings;


//SubGraph_CommonSettings
struct SubGraph_CommonSettings
{
    SubGraph_CommonSettings() {reset();}

    struct SGFactory
    {
        SGFactory() {reset();}
        QString sub_id;
        QString chain; //blockchain name
        QString swap_place; //resource (site)
        void reset() {sub_id.clear(); chain.clear(); swap_place.clear();}
        bool invalid() const {return (sub_id.isEmpty() || chain.isEmpty());}
        QString iconPath() const;
        QString toStr() const {return QString("CHAID[%1]  ID[%2]  ICON[%3]").arg(chain).arg(sub_id).arg(iconPath());}
    };



    QList<SGFactory> factories;
    QStringList prefer_tokens;
    QMap<QString, quint8> token_decimals;
    int cur_factory;
    bool only_prefer_tokens;
    QString wallet;
    QString nodejs_path;

    static QString appDataPath();
    static QString tickerByChainName(QString);
    static double tickKwant() {return 1.0001;}
    static QString nativeTokenAddress() {return QString("0xeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");}
    static QString nativeTokenByChain(QString);
    static QString iconPathByChain(QString); //icon file path by chain name
    static QString commonIconsPath() {return QString(":/icons/images");} //базовый путь к иконкам проекта

    void reset();
    void loadConfig(QString&);
    void parseFactoriesNode(const QDomNode&);
    void parsePreferTokensNode(const QDomNode&);
    void setCurFactory(QString sub_id);
    QString curChain() const; //cur chain name by cur_factory or ?
    int tokenDecimal(QString, QString) const; //params 1-token sumb, 2-chain

    QStringList factoryTitles() const;
    bool invalid() const {return (factories.isEmpty());}

};

extern SubGraph_CommonSettings sub_commonSettings;



#endif // SUB_COMMONSETTINGS_H



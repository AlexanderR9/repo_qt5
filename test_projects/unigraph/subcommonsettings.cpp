#include "subcommonsettings.h"
#include "lfile.h"
#include "lstring.h"
#include "lstaticxml.h"

#include <QDir>
#include <QDebug>
#include <QSettings>

#define APP_DATA_FOLDER     QString("data")
#define APP_CONFIG_FILE     QString("config.xml")
#define NODEJS_PATH         QString("/home/roman/work/my/github_repos/repo_html/nodejs/crypto")


//global var
SubGraph_CommonSettings sub_commonSettings;


//API_CommonSettings CPP desc
QString SubGraph_CommonSettings::appDataPath()
{
    return QString("%1%2%3").arg(LFile::appPath()).arg(QDir::separator()).arg(APP_DATA_FOLDER);
}
QStringList SubGraph_CommonSettings::factoryTitles() const
{
    QStringList list;
    foreach (const SGFactory &f, factories)
    {
        QString s = f.chain;
        if (!f.swap_place.isEmpty()) s = QString("%1 (%2)").arg(s).arg(f.swap_place);
        list << s;
    }
    return list;
}
void SubGraph_CommonSettings::setCurFactory(QString sub_id)
{
    for (int i=0; i<factories.count(); i++)
        if (factories.at(i).sub_id == sub_id) {cur_factory=i; return;}

    cur_factory = -1;
}
QString SubGraph_CommonSettings::curChain() const
{
    if (cur_factory < 0 || factories.isEmpty()) return "?";
    return factories.at(cur_factory).chain;
}
int SubGraph_CommonSettings::tokenDecimal(QString symb, QString ch) const
{
    symb = symb.trimmed().toUpper();
    if (!token_decimals.contains(symb)) return -1;
    if (ch.trimmed().toUpper() == "BNB") return 18;
    return token_decimals.value(symb);
}
void SubGraph_CommonSettings::reset()
{
    cur_factory = -1;
    factories.clear();
    prefer_tokens.clear();
    only_prefer_tokens = false;
    wallet.clear();
    token_decimals.clear();
    nodejs_path = NODEJS_PATH;
}
QString SubGraph_CommonSettings::tickerByChainName(QString chain_name)
{
    chain_name = chain_name.trimmed().toLower();
    if (chain_name == "polygon") return "POL";
    if (chain_name == "bnb") return "BNB";
    if (chain_name == "base") return "BASE";
    if (chain_name == "optimism") return "OP";
    if (chain_name == "arbitrum") return "ARB";
    if (chain_name == "etherium") return "ETH";
    return QString();
}
bool SubGraph_CommonSettings::isStableToken(QString token)
{
    token = token.trimmed().toUpper();
    if (token == "DAI") return true;
    if (token.length() < 4) return false;
    return (token.left(4) == "USDT" || token.left(4) == "USDC");
}
bool SubGraph_CommonSettings::isWrapedToken(QString token)
{
    token = token.trimmed().toUpper();
    if (token == "WPOL") return true;
    if (token == "WETH") return true;
    if (token == "WBNB") return true;
    return false;
}
QString SubGraph_CommonSettings::tokenIdCoingecko(QString token)
{
    token = token.trimmed().toUpper();

    if (token == "POL" || token == "WPOL")     return "polygon-ecosystem-token";
    if (token == "ETH" || token == "WETH")     return "ethereum";
    if (token == "BNB" || token == "WBNB")     return "binancecoin";

    if (token == "ARB")     return "arbitrum";
    if (token == "OP")      return "optimism";
    if (token == "AAVE")    return "aave";
    if (token == "SOL")     return "solana";
    if (token == "LINK")    return "chainlink";
    if (token == "XRP")     return "ripple";
    if (token == "TRX")     return "tron";
    if (token == "ZRO")     return "layerzero";
    if (token == "UNI")     return "uniswap";
    if (token == "LDO")     return "lido-dao";
    if (token == "CRV")     return "curve-dao-token";
    if (token == "CAKE")    return "pancakeswap-token";
    if (token == "TWT")     return "trust-wallet-token";
    if (token == "NEAR")    return "near";

    return QString();
}
QString SubGraph_CommonSettings::nativeCoinByChain(QString chain_name)
{
    chain_name = chain_name.trimmed().toLower();
    if (chain_name == "polygon") return "POL";
    if (chain_name == "bnb") return "BNB";
    //if (chain_name == "base") return "BASE";
    if (chain_name == "optimism") return "ETH";
    if (chain_name == "arbitrum") return "ETH";
    if (chain_name == "etherium") return "ETH";
    return QString();
}
QString SubGraph_CommonSettings::iconPathByChain(QString chain_name)
{
    QString icon_name = QString("%1.png").arg(SubGraph_CommonSettings::tickerByChainName(chain_name)).trimmed();
    if (icon_name.isEmpty()) return QString();
    return QString("%1/crypto/%2").arg(commonIconsPath()).arg(icon_name);
}
void SubGraph_CommonSettings::loadConfig(QString &err)
{
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(APP_CONFIG_FILE);
    if (!LFile::fileExists(fname))
    {
        err = QString("SubGraph_CommonSettings::loadConfig - config file not found [%1]").arg(fname);
        return;
    }

    QFile f(fname.trimmed());
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        err = QString("SubGraph_CommonSettings::loadConfig - config filename [%1] can't load to DomDocument").arg(fname);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    QDomNode root_node = dom.namedItem("params");
    if (root_node.isNull())
    {
        err = QString("SubGraph_CommonSettings::loadConfig: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg("params");
        return;
    }

    QDomNode factories_node = root_node.namedItem("factories");
    if (!factories_node.isNull()) parseFactoriesNode(factories_node);

    QDomNode prefer_tokens_node = root_node.namedItem("prefer_tokens");
    if (!prefer_tokens_node.isNull()) parsePreferTokensNode(prefer_tokens_node);
}
void SubGraph_CommonSettings::parseFactoriesNode(const QDomNode &node)
{
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "factory")
        {
            SGFactory f;
            f.sub_id = LStaticXML::getStringAttrValue("subgraph_id", child_node).trimmed();
            f.chain = LStaticXML::getStringAttrValue("chain", child_node).trimmed();
            f.swap_place = LStaticXML::getStringAttrValue("source", child_node).trimmed();
            if (!f.invalid())
            {
                factories.append(f);
                qDebug()<<QString("LOADED FACOTRY: %1").arg(f.toStr());
            }
        }
        child_node = child_node.nextSibling();
    }
}
void SubGraph_CommonSettings::parsePreferTokensNode(const QDomNode &node)
{
    prefer_tokens.clear();
    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == "token")
        {
            QString t = LStaticXML::getStringAttrValue("name", child_node).trimmed();
            if (!t.isEmpty())
            {
                prefer_tokens.append(t);
                int dec = LStaticXML::getIntAttrValue("decimal", child_node, -1);
                if (dec >= 0) token_decimals.insert(t, dec);
            }
        }
        child_node = child_node.nextSibling();
    }
}




//API_CommonSettings::SGFactory
QString SubGraph_CommonSettings::SGFactory::iconPath() const
{
    QString icon_name = QString("%1.png").arg(SubGraph_CommonSettings::tickerByChainName(chain));
    return QString("%1/crypto/%2").arg(commonIconsPath()).arg(icon_name);
}






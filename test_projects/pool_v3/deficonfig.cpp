#include "deficonfig.h"
#include "lfile.h"
#include "appcommonsettings.h"
#include "lstring.h"


#include <QDir>
#include <QDebug>
#include <QSettings>


//global var
DefiConfiguration defi_config;


///////////////////DefiConfiguration//////////////////////
void DefiConfiguration::reset()
{
    chains.clear();
    tokens.clear();
    pools.clear();
    prioritet_data.clear();

    delayAfterTX = 8;
    target_wallet = "0x";
}
int DefiConfiguration::DefiConfiguration::chainIndexOf(int cid) const
{
    if (chains.isEmpty()) return -1;
    for (int i=0; i<chains.count(); i++)
        if (chains.at(i).chain_id == cid) return i;
    return -1;
}
int DefiConfiguration::getChainID(QString chain_name) const
{
    chain_name = chain_name.trimmed().toLower();
    foreach (const DefiChain &v, chains)
        if (v.name == chain_name) return v.chain_id;
    return -1;
}
QString DefiConfiguration::nativeTokenName(QString chain_name) const
{
    chain_name = chain_name.trimmed().toLower();
    foreach (const DefiChain &v, chains)
        if (v.name == chain_name) return v.coin;
    return "?";
}
float DefiConfiguration::lastPriceByTokenName(QString t_name) const
{
    t_name = t_name.trimmed();
    if (t_name.length() < 2) return -1;

    foreach (const DefiToken &v, tokens)
    {
        if (v.name == t_name) return v.last_price;
        if (v.name == QString("W%1").arg(t_name)) return v.last_price;
    }
    return -1;
}
int DefiConfiguration::getTokenIndex(QString t_addr, int cid) const
{
    for (int i=0; i<tokens.count(); i++)
    {
        if (tokens.at(i).address == t_addr && tokens.at(i).chain_id == cid)
            return i;
    }
    return -1;
}
int DefiConfiguration::getPoolIndex(QString p_addr) const
{
    for (int i=0; i<pools.count(); i++)
    {
        if (pools.at(i).address == p_addr) return i;
    }
    return -1;
}
int DefiConfiguration::getPoolTokenPriceIndex(QString pair) const
{
    QStringList token_names = LString::trimSplitList(pair, "/");
    if (token_names.count() != 2)
    {
        qWarning()<<QString("DefiConfiguration::getPoolTokenPriceIndex  WARNING can't get pair tokens from [%1]").arg(pair);
        return 0;
    }

    foreach (const PoolTokenPrioritet &v, prioritet_data)
    {
        if (v.pair == pair || v.pair == QString("%1/%2").arg(token_names.last()).arg(token_names.first()))
        {
            if (v.price_token == token_names.first()) return 0;
            if (v.price_token == token_names.last()) return 1;
            break;
        }
    }
    return 0;
}
void DefiConfiguration::findPoolTokenAddresses(DefiPoolV3 &pool)
{
   // qDebug("DefiConfiguration::findPoolTokenAddresses");
   // qDebug()<<pool.toStr();
    QStringList token_names = LString::trimSplitList(pool.name, "/");
    if (token_names.count() != 2)
    {
        qWarning()<<QString("DefiConfiguration::findPoolTokenAddresses  WARNING can't get pair tokens from [%1]").arg(pool.name);
        return;
    }

  //  qDebug()<<QString("tokens %1").arg(tokens.count());
    foreach (const DefiToken &v, tokens)
    {
        if (v.chain_id != pool.chain_id) continue;

        if (v.name == token_names.first()) pool.token0_addr = v.address;
        else if (v.name == token_names.last()) pool.token1_addr = v.address;
    }
}

///////////////////DefiChain//////////////////////
DefiChain::DefiChain()
    :DefiEntityBase(),
    title(QString()),
    coin(QString()),
    icon_file(QString())
{

}
bool DefiChain::invalid() const
{
    if (chain_id <= 0) return true;
    if (name.length() < 3) return true;
    if (coin.length() < 3 || coin.length() > 4) return true;
    return false;
}
QString DefiChain::toStr() const
{
    QString s("DefiChain: ");
    if (invalid()) return QString("%1 invalid").arg(s);
    s = QString("%1 name[%2] ID[%3] coin[%4]").arg(s).arg(name).arg(chain_id).arg(coin);
    s = QString("%1 title[%2] icon[%3]").arg(s).arg(title).arg(icon_file);
    return s;
}
QString DefiChain::fullIconPath() const
{
    if (invalid()) return QString();
    if (icon_file.trimmed().isEmpty()) return QString();
    return QString("%1%2%3").arg(AppCommonSettings::commonIconsPath()).arg(QDir::separator()).arg(icon_file.trimmed());
}



///////////////////DefiToken//////////////////////
DefiToken::DefiToken()
    :DefiEntityBase(),
    decimal(18),
    address(QString("0x0")),
    icon_file(QString()),
    is_stable(false),
    last_price(-1)
{

}
bool DefiToken::invalid() const
{
    if (chain_id <= 0 || decimal == 0 || decimal > 18) return true;
    if (name.length() < 2 || name.length() > 4) return true;
    if (address.length() < 30 || address.left(2) != "0x")  return true;
    return false;
}
QString DefiToken::toStr() const
{
    QString s("DefiToken: ");
    if (invalid()) return QString("%1 invalid").arg(s);
    s = QString("%1 name[%2] CID[%3] address[%4]").arg(s).arg(name).arg(chain_id).arg(address);
    s = QString("%1 decimal[%2] icon[%3]").arg(s).arg(decimal).arg(icon_file);
    return s;
}
QString DefiToken::fullIconPath() const
{
    if (invalid()) return QString();
    if (icon_file.trimmed().isEmpty()) return QString();
    return QString("%1%2%3").arg(AppCommonSettings::commonIconsPath()).arg(QDir::separator()).arg(icon_file.trimmed());
}
bool DefiToken::isWraped() const
{
    return (name == "WPOL" || name == "WBNB" || name == "WETH");
}

///////////////////DefiPoolV3//////////////////////
DefiPoolV3::DefiPoolV3()
    :DefiEntityBase(),
    fee(0),
    address(QString("0x0")),
    token0_addr(QString()),
    token1_addr(QString()),
    is_stable(false)
{



}
bool DefiPoolV3::invalid() const
{
    if (chain_id <= 0 || fee == 0) return true;
    if (name.length() < 6 || name.length() > 10) return true;
    if (address.length() < 30 || address.left(2) != "0x")  return true;
    if (token0_addr.length() < 30 || token0_addr.left(2) != "0x")  return true;
    if (token1_addr.length() < 30 || token1_addr.left(2) != "0x")  return true;
    return false;
}
QString DefiPoolV3::toStr() const
{
    QString s("DefiPoolV3: ");
    s = QString("%1 name[%2] CID[%3] address[%4]").arg(s).arg(name).arg(chain_id).arg(address);
    s = QString("%1 token0_addr[%2] token1_addr[%3]").arg(s).arg(token0_addr).arg(token1_addr);
    if (invalid()) return QString("%1 invalid").arg(s);
    return s;
}



///////////////////BybitSettings//////////////////////
BybitSettings::BybitSettings()
    :public_key(QString()),
    private_key(QString()),
    api_server(QString()),
    timeout(3000)
{

}
bool BybitSettings::invalid() const
{
    if (public_key.length() < 10 || private_key.length() < 20) return true;
    if (api_server.length() < 5 || !api_server.contains(".com")) return true;
    return false;
}


///////////////////PoolTokenPrioritet//////////////////////
PoolTokenPrioritet::PoolTokenPrioritet()
    :pair(QString()),
    price_token(QString()),
    desired_token(QString())
{

}
bool PoolTokenPrioritet::invalid() const
{
    if (pair.length() < 6 || pair.length() > 10 || !pair.contains("/")) return true;
    if (price_token.length() < 2 || desired_token.length() < 2 ) return true;
    if (!pair.contains(price_token) || !pair.contains(desired_token))  return true;
    return false;
}
QString PoolTokenPrioritet::toStr() const
{
    QString s("PoolTokenPrioritet: ");
    s = QString("%1 pair[%2] price_token[%3] address[%4]").arg(s).arg(pair).arg(price_token).arg(desired_token);
    return s;
}


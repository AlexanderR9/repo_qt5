#include "deficonfig.h"
#include "lfile.h"
#include "appcommonsettings.h"

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
}
int DefiConfiguration::DefiConfiguration::chainIndexOf(int cid) const
{
    if (chains.isEmpty()) return -1;
    for (int i=0; i<chains.count(); i++)
        if (chains.at(i).chain_id == cid) return i;
    return -1;
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
    address(QString("0x0"))
{

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




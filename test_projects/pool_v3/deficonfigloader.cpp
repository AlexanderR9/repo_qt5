#include "deficonfigloader.h"
#include "deficonfig.h"
#include "lfile.h"
#include "lstaticxml.h"

#include <QDebug>
#include <QDomDocument>



//DefiConfigLoader
DefiConfigLoader::DefiConfigLoader(QObject *parent)
    :LSimpleObject(parent)
{
    setObjectName("config_loader_obj");


}
void DefiConfigLoader::loadDefiConfiguration(QString fconfig)
{
    defi_config.reset();

    // check file
    fconfig = fconfig.trimmed();
    if (fconfig.isEmpty()) {emit signalError("DEFI config file is empty"); return;}
    if (!LFile::isXmlFile(fconfig))
    {
        emit signalError(QString("DEFI config file [%1] is not XML").arg(fconfig));
        return;
    }
    if (!LFile::fileExists(fconfig))
    {
        emit signalError(QString("DEFI config file [%1] not found").arg(fconfig));
        return;
    }

    // try load to QDomDocument
    QFile f(fconfig);
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        emit signalError(QString("can't load DEFI config file [%1] to QDomDocument").arg(fconfig));
        if (f.isOpen()) f.close();
        return;
    }
    f.close();
    QDomNode root_node = dom.namedItem("config");
    if (root_node.isNull())
    {
        emit signalError(QString("invalid struct of  DEFI config file [%1], node <config> not found.").arg(fconfig));
        return;
    }


    //read defi configuration
    emit signalMsg(QString("DEFI config file [%1] success loaded to QDomDocument!").arg(fconfig));
    readChainsNode(root_node);
    readTokensNode(root_node);
    readPoolsNode(root_node);
    readBBNode(root_node);
    readTargetWalletNode(root_node);

}
void DefiConfigLoader::readChainsNode(const QDomNode &root_node)
{
    emit signalMsg("");
    emit signalMsg("load <chains> section ............");
    QDomNode node = root_node.namedItem("chains");
    if (node.isNull())
    {
        emit signalError(QString("invalid XML struct of DEFI config, node <chains> not found."));
        return;
    }

    QDomNode chain_node = node.firstChild();
    while (!chain_node.isNull())
    {
        if (chain_node.nodeName() == "chain")
        {
            DefiChain chain;
            chain.name = LStaticXML::getStringAttrValue("name", chain_node).trimmed();
            chain.chain_id = LStaticXML::getIntAttrValue("id", chain_node, -1);
            chain.title = LStaticXML::getStringAttrValue("title", chain_node).trimmed();
            chain.coin = LStaticXML::getStringAttrValue("coin", chain_node).trimmed().toUpper();
            chain.icon_file = LStaticXML::getStringAttrValue("icon", chain_node).trimmed();

            if (chain.invalid()) emit signalError("find invalid <chain> node");
            else {defi_config.chains.append(chain); emit signalMsg(chain.toStr());}
        }
        chain_node = chain_node.nextSibling();
    }
}
void DefiConfigLoader::readTokensNode(const QDomNode &root_node)
{
    emit signalMsg("");
    emit signalMsg("load <tokens> section ............");
    QDomNode node = root_node.namedItem("tokens");
    if (node.isNull())
    {
        emit signalError(QString("invalid XML struct of DEFI config, node <tokens> not found."));
        return;
    }

    QDomNode token_node = node.firstChild();
    while (!token_node.isNull())
    {
        if (token_node.nodeName() == "token")
        {

            DefiToken token;
            token.name = LStaticXML::getStringAttrValue("name", token_node).trimmed().toUpper();
            token.chain_id = LStaticXML::getIntAttrValue("cid", token_node, -1);
            token.decimal = LStaticXML::getIntAttrValue("decimal", token_node, 18);
            token.address = LStaticXML::getStringAttrValue("address", token_node, "0x0").trimmed().toLower();
            token.icon_file = LStaticXML::getStringAttrValue("icon", token_node).trimmed();
            token.is_stable = (LStaticXML::getStringAttrValue("stable", token_node).trimmed() == "yes");

            if (token.is_stable) token.last_price = 1;
            if (token.invalid()) emit signalError("find invalid <token> node");
            else {defi_config.tokens.append(token); emit signalMsg(token.toStr());}
        }
        token_node = token_node.nextSibling();
    }
}
void DefiConfigLoader::readPoolsNode(const QDomNode &root_node)
{
    //qDebug("");
   // qDebug("DefiConfigLoader::readPoolsNode");

    emit signalMsg("");
    emit signalMsg("load <pools> section ............");
    QDomNode node = root_node.namedItem("pools");
    if (node.isNull())
    {
        emit signalError(QString("invalid XML struct of DEFI config, node <pools> not found."));
        return;
    }

    qDebug("<pools> node ok");
    QDomNode pool_node = node.firstChild();
    while (!pool_node.isNull())
    {
        if (pool_node.nodeName() == "pool")
        {
            //qDebug("find pool node");
            DefiPoolV3 pool;
            pool.chain_id = LStaticXML::getIntAttrValue("cid", pool_node, -1);
            pool.address = LStaticXML::getStringAttrValue("address", pool_node, "?").trimmed().toLower();

            QString token0 = LStaticXML::getStringAttrValue("token0", pool_node, "?").trimmed().toUpper();
            QString token1 = LStaticXML::getStringAttrValue("token1", pool_node, "?").trimmed().toUpper();
            pool.name = QString("%1/%2").arg(token0).arg(token1);
            pool.fee = LStaticXML::getIntAttrValue("fee", pool_node, 0);
            pool.is_stable = (LStaticXML::getStringAttrValue("stable", pool_node).trimmed() == "yes");

            defi_config.findPoolTokenAddresses(pool);
           // qDebug()<<pool.toStr();

            if (pool.invalid()) emit signalError("find invalid <pool> node");
            else {defi_config.pools.append(pool); emit signalMsg(pool.toStr());}
        }
        pool_node = pool_node.nextSibling();
    }

    readPoolPrioritetNode(node);
}
void DefiConfigLoader::readPoolPrioritetNode(const QDomNode &pools_node)
{
    emit signalMsg("");
    emit signalMsg("load <pools_prioritet> section ............");
    QDomNode node = pools_node.namedItem("prioritet");
    if (node.isNull())
    {
        emit signalError(QString("invalid XML struct of DEFI config, node <pools>/<prioritet> not found."));
        return;
    }

    QDomNode pair_node = node.firstChild();
    while (!pair_node.isNull())
    {
        if (pair_node.nodeName() == "pool")
        {
            PoolTokenPrioritet pool;
            pool.pair = LStaticXML::getStringAttrValue("pair", pair_node, "?").trimmed().toUpper();
            pool.price_token = LStaticXML::getStringAttrValue("price_token", pair_node, "?").trimmed().toUpper();
            pool.desired_token = LStaticXML::getStringAttrValue("desired_token", pair_node, "?").trimmed().toUpper();

            if (pool.invalid()) emit signalError("find invalid <pools>/<prioritet> node");
            else {defi_config.prioritet_data.append(pool); emit signalMsg(pool.toStr());}
        }
        pair_node = pair_node.nextSibling();
    }
}
void DefiConfigLoader::readBBNode(const QDomNode &root_node)
{
    emit signalMsg("");
    emit signalMsg("load bybit section ............");
    QDomNode node = root_node.namedItem("bb");
    if (node.isNull())
    {
        emit signalError(QString("invalid XML struct of DEFI config, node <bb> not found."));
        return;
    }

    defi_config.bb_settings.api_server = LStaticXML::getStringAttrValue("api_server", node).trimmed();
    defi_config.bb_settings.public_key = LStaticXML::getStringAttrValue("key", node).trimmed();
    defi_config.bb_settings.private_key = LStaticXML::getStringAttrValue("pkey", node).trimmed();
    defi_config.bb_settings.timeout = LStaticXML::getIntAttrValue("timeout", node, 4900);

}
void DefiConfigLoader::readTargetWalletNode(const QDomNode &root_node)
{
    emit signalMsg("load target_wallet section ............");
    QDomNode node = root_node.namedItem("target_wallet");
    if (node.isNull())
    {
        emit signalError(QString("DefiConfigLoader: node <target_wallet> not found, default value - 0x."));
        return;
    }

    defi_config.target_wallet = LStaticXML::getStringAttrValue("address", node).trimmed();
}




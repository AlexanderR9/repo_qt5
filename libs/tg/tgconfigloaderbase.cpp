#include "tgconfigloaderbase.h"
#include "tgabstractbot.h"
#include "lstatic.h"

#include <QFile>
#include <QDomDocument>


//LTGConfigLoaderBase
LTGConfigLoaderBase::LTGConfigLoaderBase(const QString &fname)
    :m_config(fname)
{

}
void LTGConfigLoaderBase::loadBotParams(LTGParamsBot &p, QString &err)
{
    err.clear();
    if (m_config.trimmed().isEmpty())
    {
        err = QString("TGConfigLoader: config filename is empty ");
        return;
    }
    QFile f(m_config.trimmed());
    if (!f.exists())
    {
        err = QString("TGConfigLoader: config filename [%1] not found").arg(m_config);
        return;
    }
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        err = QString("TGConfigLoader: config filename [%1] can't load to DomDocument").arg(m_config);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    parseDom(dom, p, err);
}
void LTGConfigLoaderBase::parseDom(const QDomDocument &dom, LTGParamsBot &p, QString &err)
{
    QString root_node_name("config");
    QDomNode root_node = dom.namedItem(root_node_name);
    if (root_node.isNull())
    {
        err = QString("TGConfigLoader: invalid struct XML document [%1], node <%2> not found.").arg(m_config).arg(root_node_name);
        return;
    }

    QString botparams_node_name("bot_params");
    QDomNode botparams_node = root_node.namedItem(botparams_node_name);
    if (botparams_node.isNull())
    {
        err = QString("TGConfigLoader: invalid struct XML document [%1], node <%2> not found.").arg(m_config).arg(botparams_node_name);
        return;
    }

    parseBotParamsNode(botparams_node, p, err);
}
void LTGConfigLoaderBase::parseBotParamsNode(const QDomNode &node, LTGParamsBot &p, QString &err)
{
    //load main params
    QString token_node_name("token");
    QDomNode token_node = node.namedItem(token_node_name);
    if (token_node.isNull())
    {
        err = QString("TGConfigLoader: invalid struct bot_params [%1], node <%2> not found.").arg(m_config).arg(token_node_name);
        return;
    }
    QString chat_id_node_name("chat_id");
    QDomNode chat_id_node = node.namedItem(chat_id_node_name);
    if (chat_id_node.isNull())
    {
        err = QString("TGConfigLoader: invalid struct bot_params [%1], node <%2> not found.").arg(m_config).arg(chat_id_node_name);
        return;
    }

    p.token = LStatic::getStringAttrValue("value", token_node);
    p.chatID = LStatic::getIntAttrValue("value", chat_id_node);

    //load other params
    QDomNode limit_msg_node = node.namedItem("limit_msg");
    if (!limit_msg_node.isNull()) p.limit_msg = LStatic::getIntAttrValue("value", limit_msg_node);

    QDomNode req_timeout_node = node.namedItem("req_timeout");
    if (!req_timeout_node.isNull()) p.req_timeout = LStatic::getIntAttrValue("value", req_timeout_node);

    QDomNode updates_interval_node = node.namedItem("updates_interval");
    if (!updates_interval_node.isNull()) p.updates_interval = LStatic::getIntAttrValue("value", updates_interval_node, 0);

}

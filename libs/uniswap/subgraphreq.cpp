#include "subgraphreq.h"
#include "lstring.h"

SubGraphReq::SubGraphReq(QString api, QString g_id)
    :api_key_dirty(api),
      subrgaph_id(g_id)
{


}
QString SubGraphReq::graphUri() const
{
    if (api_key_dirty.isEmpty() || subrgaph_id.isEmpty()) return "??";
    QString api = LString::strTrimLeft(api_key_dirty, 4);
    api = LString::strTrimRight(api, 4);
    api.prepend(api_key_dirty.right(4));
    api.append(api_key_dirty.left(4));
    return QString("api/%1/subgraphs/id/%2").arg(api).arg(subrgaph_id);
}
void SubGraphReq::setApiKeys(QString akey, QString sub_id)
{
    api_key_dirty = akey.trimmed();
    subrgaph_id = sub_id.trimmed();
}


#ifndef SUBGRAPHREQ_H
#define SUBGRAPHREQ_H

#include <QString>

//SubGraphReq
//класс для формирования содержимого запроса для subgraph uniswap
//после создания объекта ему необходимо задать методом setApiKeys api_ключ и ID subgraph к которому планируется посылать запросы
//метод graphUri возвращает кусок URL вида api/{api-key}/subgraphs/id/{subgraph_id} (т.е. все что после базового адреса сервера)
class SubGraphReq
{
public:
    SubGraphReq() :api_key_dirty(QString()), subrgaph_id(QString()) {}
    SubGraphReq(QString, QString);
    virtual ~SubGraphReq() {}

    QString graphUri() const;
    void setApiKeys(QString, QString);

protected:
    QString api_key_dirty;
    QString subrgaph_id;

};

#endif // SUBGRAPHREQ_H



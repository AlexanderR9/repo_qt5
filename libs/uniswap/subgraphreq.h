#ifndef SUBGRAPHREQ_H
#define SUBGRAPHREQ_H

#include <QString>

//SubGraphReq
class SubGraphReq
{
public:
    SubGraphReq() :api_key_dirty(QString()), subrgaph_id(QString()) {}
    SubGraphReq(QString, QString);

    QString graphUri() const;
    void setApiKeys(QString, QString);


protected:
    QString api_key_dirty;
    QString subrgaph_id;


};

#endif // SUBGRAPHREQ_H



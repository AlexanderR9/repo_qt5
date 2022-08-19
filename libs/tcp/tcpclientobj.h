#ifndef TCP_CLIENT_OBJECT_H
#define TCP_CLIENT_OBJECT_H


#include "lsimpleobj.h"


//LTcpClientObj
class LTcpClientObj : public LSimpleObject
{
    Q_OBJECT
public:
    LTcpClientObj(QObject *parent = NULL);
    virtual ~LTcpClientObj() {}



};



#endif //TCP_CLIENT_OBJECT_H



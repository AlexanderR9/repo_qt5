#ifndef CFDREQUESTER_H
#define CFDREQUESTER_H

#include "lsimpleobj.h"


class LHTMLPageRequester;

//CFDRequester
class CFDRequester : public LSimpleObject
{
    Q_OBJECT
public:
    CFDRequester(QObject*);
    virtual ~CFDRequester() {}

protected:
    LHTMLPageRequester  *m_htmlRequester;


};


#endif //CFDREQUESTER_H



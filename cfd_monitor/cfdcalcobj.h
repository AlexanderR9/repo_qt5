#ifndef CFDCALCOBJ_H
#define CFDCALCOBJ_H

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


#endif //CFDCALCOBJ_H



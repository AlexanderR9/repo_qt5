#ifndef CFDCONFIGOBJ_H
#define CFDCONFIGOBJ_H

#include "lsimpleobj.h"



//CFDConfigObject
class CFDConfigObject : public LSimpleObject
{
    Q_OBJECT
public:
    CFDConfigObject(const QString&, QObject*);
    virtual ~CFDConfigObject() {}

    QString name() const {return QString("cfd_config_obj");}

    void tryLoadConfig();


protected:
    QString m_configFile;

};


#endif //CFDCONFIGOBJ_H



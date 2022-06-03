#include "cfdconfigobj.h"

CFDConfigObject::CFDConfigObject(const QString &fname, QObject *parent)
    :LSimpleObject(parent),
    m_configFile(fname.trimmed())
{

}
void CFDConfigObject::tryLoadConfig()
{

}

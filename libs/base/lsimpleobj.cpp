#include "lsimpleobj.h"


LSimpleObject::LSimpleObject(QObject *parent)
    :QObject(parent),
      m_debugLevel(0),
      m_userSign(-1)
{

}


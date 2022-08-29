#include "fxenums.h"


QList<int> FXEnumStaticObj::timeFrames()
{
    QList<int> list;
    list << tf5m << tf15m << tf1h <<  tf4h << tf1d <<  tf1w;
    return list;
}









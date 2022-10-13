#include "fxtest_spring.h"
#include "fxbarcontainer.h"
#include "fxenums.h"



//FXTest_Spring
FXTest_Spring::FXTest_Spring(const FXBarContainer *data, QObject *parent)
    :FXAbstractTest(data, parent)
{

}
void FXTest_Spring::checkInputParams()
{

}
QList<int> FXTest_Spring::inputParamsTypes() const
{
    QList<int> list;
    list << ipStartSum << ipStartLot << ipNextLotFactor << ipStopPips << ipStopFactor << ipTralingPips << ipDist;
    return list;
}
QList<int> FXTest_Spring::outputParamsTypes() const
{
    QList<int> list;
    list << spSum << spLineNumber << spFullLossNumber << spMinSum << spMaxSum << spLotSize;
    return list;
}
int FXTest_Spring::type() const
{
    return ttSpring;
}




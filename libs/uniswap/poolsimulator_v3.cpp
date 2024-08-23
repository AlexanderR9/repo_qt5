#include "poolsimulator_v3.h"


// LPoolSimulatorObj
LPoolSimulatorObj::LPoolSimulatorObj(QObject *parent)
    :LSimpleObject(parent)
{
    setObjectName("simulator_v3_obj");
    reset();
}

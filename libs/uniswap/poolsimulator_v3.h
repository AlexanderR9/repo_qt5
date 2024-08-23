#ifndef POOLSIMULATOR_V3_H
#define POOLSIMULATOR_V3_H

#include "lsimpleobj.h"


//класс для расчета изменения параметров пула ликвидности при изменении цены,
//нужен для имитации реального реального результата на некотором промежутке времени
class LPoolSimulatorObj : public LSimpleObject
{
    Q_OBJECT
public:
    LPoolSimulatorObj(QObject *parent = NULL);
    virtual ~LPoolSimulatorObj() {}

    QString name() const {return QString("PoolSimulator_v3");}

protected:
    void reset() {}

};




#endif // POOLSIMULATOR_V3_H

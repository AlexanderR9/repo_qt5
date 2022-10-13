#ifndef FX_TEST_SPRING_H
#define FX_TEST_SPRING_H


#include "fxabstracttest.h"

#include <QList>

class FXBarContainer;


//FXTest_Spring
class FXTest_Spring : public FXAbstractTest
{
    Q_OBJECT
public:
    FXTest_Spring(const FXBarContainer*, QObject*);
    virtual ~FXTest_Spring() {}

    virtual QString name() const {return QString("Spring standard");}

    //pure virtual funcs
    virtual QList<int> inputParamsTypes() const; //список входных параметров
    virtual QList<int> outputParamsTypes() const; //список выходных параметров в результирующей таблице
    virtual int type() const; //тип теста, элемент множества FXTestType


protected:
    virtual void checkInputParams();
    //virtual void finish() {} //номальное завершение тестирования, (происходит при завершении временного интервала)
    //virtual void breakLoss() {} //завершение тестирования, (происходит при достижении максимальной заданной просадки или т.п.)
    //virtual void makeIt() {} //выполнить итерацию расчетов тестирования при очередной поступившей свече


};



#endif //FX_TEST_SPRING_H



#ifndef FX_TEST_H
#define FX_TEST_H


#include "fxabstracttest.h"

class FXBarContainer;

//класс для проведения тестирования


//FXTest
class FXTest : public FXAbstractTest
{
    Q_OBJECT
public:
    FXTest(const FXBarContainer*, QObject*);
    virtual ~FXTest() {}

protected:
    virtual void checkInputParams();
    //virtual void finish() {} //номальное завершение тестирования, (происходит при завершении временного интервала)
    //virtual void breakLoss() {} //завершение тестирования, (происходит при достижении максимальной заданной просадки или т.п.)
    //virtual void makeIt() {} //выполнить итерацию расчетов тестирования при очередной поступившей свече


};



#endif //FX_TEST_H



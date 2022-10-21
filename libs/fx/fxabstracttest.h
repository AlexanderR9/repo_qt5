#ifndef FX_ABSTRACT_TEST_H
#define FX_ABSTRACT_TEST_H


#include "lsimpleobj.h"

#include <QMap>
#include <QDateTime>

class FXBarContainer;

//абстрактный класс для проведения тестирования,
//для реализация конкретной стратегии необходимо унаследоваться от этого класса и реализовать ее.
//тестирование проводится для оного инструмента m_data, (в некоторых стратегиях возможно присутстствие доп. инструментов, но это надо и реализовывать в классе-наследнике).
//перед запуском тестирования необходимо задать границы временного отрезка и входные параметры тестирования.
//после завершения теста результаты тестирования должны лежать в контейнере m_stateParams.


//FXAbstractTest
class FXAbstractTest : public LSimpleObject
{
    Q_OBJECT
public:
    FXAbstractTest(const FXBarContainer*, QObject*);
    virtual ~FXAbstractTest() {}


    virtual void tryStart(); //попытка запуска теста
    virtual QString name() const {return QString("FX abstract test");}

    inline bool hasErr() const {return (!m_err.isEmpty());} //наличие ошибки
    inline void setTimeInterval(const QDate &d1, const QDate &d2) {m_startTime = d1; m_finishTime = d2;} //установить временной интервал тестирования
    inline void setInputParam(int key, double v) {m_inputParams.insert(key, v);} //задать значение одного входного параметра

    static QString testDateMask() {return QString("dd.MM.yyyy");}

    //pure virtual funcs
    virtual QList<int> inputParamsTypes() const = 0; //список входных параметров
    virtual QList<int> outputParamsTypes() const = 0; //список выходных параметров (в результирующей таблице интерфейса)
    virtual int type() const = 0; //тип теста, элемент множества FXTestType


protected:
    const FXBarContainer    *m_data; //данные для тестирования
    QMap<int, double>       m_inputParams; //входные параметры тестирования, key - элемент множества FXInputParam
    QDate                   m_startTime; //начальная дата отрезка тестирования
    QDate                   m_finishTime; //конечная дата отрезка тестирования
    QString                 m_err; //признак ошибки при некорректных входных данных
    int                     m_curBarIndex; //текущй индекс свечи, меняется в процессе тестирования


    //текущие параметры результатов тестирования (они же выходные),
    //перед началом тестирования контейнер пустой, его запонять не надо,
    //он должен заполниться сам в процессе тестирования,
    //после завершения в нем будут лежать конечные результаты теста
    QMap<int, double> m_stateParams; // key - элемент множества FXStateParam

    void resetAll(); //сбросить все в начальное состояние

    virtual void resetTest(); //сброс параметров перед началом (первого/очередного) тестирования
    virtual void exec(); //выполнить тестирование, подразумевается что всех входные данные валидны
    virtual void checkData(); //проверить сам контейнер с данными тестирования m_data
    virtual void checkDateInterval(); //проверить корректность временного интервала тестирования

    //pure virtual funcs
    virtual void checkInputParams() = 0; //проверить корректность и достаточность входных параметров
    virtual void finish() = 0; //номальное завершение тестирования, (происходит при завершении временного интервала)
    virtual void breakLoss() = 0; //завершение тестирования, (происходит при достижении максимальной заданной просадки или т.п.)
    virtual void makeIt() = 0; //выполнить итерацию расчетов тестирования при очередной поступившей свече



};


#endif //FX_ABSTRACT_TEST_H



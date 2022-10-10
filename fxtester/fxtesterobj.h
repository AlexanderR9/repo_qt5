#ifndef FX_TESTEROBJ_H
#define FX_TESTEROBJ_H


#include "lsimpleobj.h"

#include <QList>


class FXAbstractTest;
class FXBarContainer;

//класс для проведения тестирования
//FXTesterObj
class FXTesterObj : public LSimpleObject
{
    Q_OBJECT
public:
    FXTesterObj(QObject*);
    virtual ~FXTesterObj() {reset();}

    void addTest(int, const FXBarContainer*);
    void reset();

protected:
    QList<FXAbstractTest*>  m_tests; //таблица объектов тестов (n_test_types * n_couples), инициализируется при загрузке данных


};


#endif //FX_TESTEROBJ_H



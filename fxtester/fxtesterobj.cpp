#include "fxtesterobj.h"
#include "fxbarcontainer.h"
#include "fxabstracttest.h"
#include "fxenums.h"


//FXTesterObj
FXTesterObj::FXTesterObj(QObject *parent)
    :LSimpleObject(parent)
{

}
void FXTesterObj::reset()
{
    qDeleteAll(m_tests);
    m_tests.clear();
}
void FXTesterObj::addTest(int t_type, const FXBarContainer *data)
{
    if (!data) return;

    switch (t_type)
    {
        case ttSpring:
        {
            break;
        }
        default: break;
    }
}





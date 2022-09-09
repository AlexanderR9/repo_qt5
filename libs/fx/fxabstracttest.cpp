#include "fxabstracttest.h"
#include "fxbarcontainer.h"
#include "fxbar.h"



//FXAbstractTest
FXAbstractTest::FXAbstractTest(const FXBarContainer *data, QObject *parent)
    :LSimpleObject(parent),
    m_data(data)
{
    resetAll();

}
void FXAbstractTest::tryStart()
{
    //reset
    resetTest();

    //checking
    checkDateInterval();
    if (hasErr())
    {
        emit signalError(QString("%1: %2").arg(name()).arg(m_err));
        return;
    }
    checkInputParams();
    if (hasErr())
    {
        emit signalError(QString("%1: %2").arg(name()).arg(m_err));
        return;
    }
    checkData();
    if (hasErr())
    {
        emit signalError(QString("%1: %2").arg(name()).arg(m_err));
        return;
    }

    //run test
    exec();
}
void FXAbstractTest::checkData()
{
    if (!m_data)
    {
        m_err = "data container is NULL";
        return;
    }
    if (m_data->invalid())
    {
        m_err = QString("data container is invalid: (%1)").arg(m_data->toStr());
        return;
    }
    if (m_data->dataEmpty())
    {
        m_err = QString("data container is empty");
        return;
    }
}
void FXAbstractTest::checkDateInterval()
{
    if (!m_startTime.isValid() || !m_finishTime.isValid())
    {
        m_err = QString("date values interval is invalid");
        return;
    }
    if (m_startTime > m_finishTime)
    {
        m_err = QString("start_date(%1) >  finish_date(%2)").arg(m_startTime.toString(testDateMask())).arg(m_finishTime.toString(testDateMask()));
        return;
    }
}
void FXAbstractTest::exec()
{
    int n = m_data->barCount();
    for (int i=0; i<n; i++)
    {
        const FXBar *bar = m_data->barAt(i);
        if (!bar) continue;
        if (bar->time().date() < m_startTime) continue;
        if (bar->time().date() > m_finishTime) {finish(); break;}

        m_curBarIndex = i;
        makeIt();
    }
}
void FXAbstractTest::resetAll()
{
    m_inputParams.clear();
    m_stateParams.clear();
    m_err.clear();
    m_startTime = m_finishTime = QDate();
    m_curBarIndex = -1;
}
void FXAbstractTest::resetTest()
{
    m_curBarIndex = 0;
    m_err.clear();
    m_stateParams.clear();

}




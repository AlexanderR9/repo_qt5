#include "processobj.h"


#include <QProcess>

/////////////// LProcessObj ///////////////////
LProcessObj::LProcessObj(QObject *parent)
    :LSimpleObject(parent),
    m_process(NULL),
    is_running(false),
    m_needSudo(false)
{
    m_process = new QProcess(this);


}
void LProcessObj::startCommand()
{
    is_running = true;


}



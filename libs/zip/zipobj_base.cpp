#include "zipobj_base.h"
#include "processobj.h"



/////////////// LZipObj_base ///////////////////
LZipObj_base::LZipObj_base(QObject *parent)
    :LSimpleObject(parent),
      m_zipType(ztQt),
      m_workingFolder(QString()),
      m_proc(NULL)
{
    setObjectName("zip_obj_base");

    initProcessor();

}
void LZipObj_base::initProcessor()
{
    m_proc = new LProcessObj(this);
    connect(m_proc, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_proc, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_proc, SIGNAL(signalFinished()), this, SLOT(slotProcFinished()));

}



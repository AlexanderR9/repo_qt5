#include "unzipobj.h"
#include "processobj.h"
//#include "lstring.h"


//#include <QProcess>
//#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QDateTime>



/////////////// LUnzipObj ///////////////////
LUnzipObj::LUnzipObj(QObject *parent)
    :LZipObj_base(parent)
{
    setObjectName("unzip_obj");

}

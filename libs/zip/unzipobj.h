#ifndef UNZIPOBJ_H
#define UNZIPOBJ_H


#include "zipobj_base.h"


//LUnzipObj
// обьект предназначен для накопления данных в QByteArray (вызов addBufferData).
// периодическая запись текущего содержимого буфера во временный файл.
// возможность заархивировать файл методом tryZipFile, файл будет лежать в m_workingFolder.
// вся работа ведется в рабочей папке m_workingFolder, которую нужно задать сразу после создания объекта.
class LUnzipObj : public LZipObj_base
{
    Q_OBJECT
public:
    LUnzipObj(QObject *parent = NULL);
    virtual ~LUnzipObj() {}

    virtual QString name() const {return QString("unzip_obj");}


};



#endif // UNZIPOBJ_H

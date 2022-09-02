#ifndef FXDATA_LOADER_H
#define FXDATA_LOADER_H

#include "lsimpleobj.h"
#include <QMap>

class FXBarContainer;


//FXDataLoader
class FXDataLoader : public LSimpleObject
{
    Q_OBJECT
public:
    FXDataLoader(QObject*);
    virtual ~FXDataLoader() {clearData();}

    inline void setDataDir(const QString &s) {m_sourceDir = s.trimmed();}
    inline int count() const {return m_data.count();}
    inline bool dataEmpty() const {return m_data.isEmpty();}

    void reloadData(); //загрузить все данные из папки m_sourceDir

protected:
    QString     m_sourceDir;

    //данные по всем инструментам и периодам лежащим в m_sourceDir,
    //key - название инструмента, vaulue - данные из одного файла
    QMap<QString, FXBarContainer*> m_data;

    void clearData(); //очистить полностью контейнер m_data
    void tryLoadFile(const QString&); //загрузить очередной файл данных

};



#endif


#ifndef FXDATA_LOADER_H
#define FXDATA_LOADER_H

#include "lsimpleobj.h"
#include <QMap>
#include <QStringList>

class FXBarContainer;

//класс реализует загрузку данных из файлов для различных инструментов и периодов,
//сначала задается папка (полный путь) методом setDataDir,
//затем выполняется метод reloadData() для загрузки всех валидных файлов-данных из этой папки


//FXDataLoader
class FXDataLoader : public LSimpleObject
{
    Q_OBJECT
public:
    FXDataLoader(QObject*);
    virtual ~FXDataLoader() {clearData();}

    inline void setDataDir(const QString &s) {m_sourceDir = s.trimmed();} //установить директорию - источник данных
    inline int count() const {return m_data.count();} //размер контейнера m_data
    inline bool dataEmpty() const {return m_data.isEmpty();}

    void reloadData(); //перезагрузить все данные из папки m_sourceDir (старые данные стираются)
    //QStringList getLoadedCouples() const; //возвращает список имен загруженных инструментов
    const FXBarContainer* containerAt(int) const; //возвращает элемент из m_data по индексу или NULL
    const FXBarContainer* container(const QString&, int) const; //возвращает элемент из m_data по имени инструмента и периоду или NULL



protected:
    QString m_sourceDir; //полный путь директории - источника данных
    QList<FXBarContainer*> m_data; //успешно загруженные данные по всем инструментам и периодам из m_sourceDir

    void clearData(); //очистить полностью контейнер m_data
    void tryLoadFile(const QString&); //загрузить очередной файл данных

};



#endif //FXDATA_LOADER_H


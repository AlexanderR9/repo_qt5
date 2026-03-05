#ifndef ZIPOBJ_BASE_H
#define ZIPOBJ_BASE_H



#include "lsimpleobj.h"

//#include <QByteArray>

//class QFile;
class LProcessObj;


class LZipObj_base : public LSimpleObject
{
    Q_OBJECT
public:
    enum ZipType {ztQt = 71, ztGZip, ztTar};  // тип архиватора

    LZipObj_base(QObject *parent = NULL);
    virtual ~LZipObj_base() {}

    inline void setWorkingFolder(QString s) {m_workingFolder = s.trimmed();}
    inline void setCompressMetod(int m) {m_zipType = m;}


protected:
    int m_zipType; // метод архивирования/извлечения, элемент ZipType
    LProcessObj *m_proc; // объект для запуска процесса архивации

    // полный путь к рабочей папке с  файлами-архивами, если папка не задана или такой не существует то класс будет выдавать ошибку.
    QString m_workingFolder;


    void initProcessor();

protected slots:
      virtual void slotProcFinished() = 0; // выполняется когда завершает работу объект m_proc

signals:
    void signalZipFinished(int); // сигнал имитится по завершению сжатия файла, параметр - код ошибки, 0 - ОК.


};


#endif // ZIPOBJ_BASE_H

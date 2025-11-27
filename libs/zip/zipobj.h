#ifndef LZIP_OBJECT_H
#define LZIP_OBJECT_H


#include "lsimpleobj.h"

#include <QByteArray>


//LZipObj
class LZipObj : public LSimpleObject
{
    Q_OBJECT
public:
    LZipObj(QObject *parent = NULL);
    virtual ~LZipObj() {}

    virtual QString name() const {return QString("zip_obj");}

    // добавить данные к буферу m_buffer
    void addBufferData(const QByteArray&);

    // сжать m_buffer и результат записать в параметр
    void tryCompress(QByteArray&);

    inline void setCompressLevel(quint8 a) {if (a > 0 && a <= 9) m_compressLevel = a;}
    inline quint8 compressLevel() const {return m_compressLevel;}
    inline const QByteArray& extractedBuffer() const {return m_bufferExtract;}
    inline const QByteArray& buffer() const {return m_buffer;}
    inline void clearBufer() {m_buffer.clear();}
    inline void setWorkingFolder(QString s) {m_workingFolder = s.trimmed();}


protected:
    quint8 m_compressLevel; //степень сжатия от 0 до 9 (0 - без сжатия)
    QByteArray  m_buffer; // данные подверженные сжатию.
    QByteArray  m_bufferExtract; // буфер для записи в него при распаковке архива.
    QString m_workingFolder; // полный путь к рабочей папке с архивами


    void reset();

};


#endif // LZIP_OBJECT_H




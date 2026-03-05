#ifndef LZIP_OBJECT_H
#define LZIP_OBJECT_H


#include "zipobj_base.h"

#include <QByteArray>

class QFile;
//class LProcessObj;



//LZipObj
// обьект предназначен для накопления данных в QByteArray (вызов addBufferData).
// периодическая запись текущего содержимого буфера во временный файл.
// возможность заархивировать файл методом tryZipFile, файл будет лежать в m_workingFolder.
// вся работа ведется в рабочей папке m_workingFolder, которую нужно задать сразу после создания объекта.
class LZipObj : public LZipObj_base
{
    Q_OBJECT
public:
    LZipObj(QObject *parent = NULL);
    virtual ~LZipObj() {closeTmpFile(false);}

    virtual QString name() const {return QString("zip_obj");}

    // добавить данные к буферу m_buffer
    void addBufferData(const QByteArray&);

    // сжать указанный файл, любой, имя задается без полного пути,
    // файл должен лежать в m_workingFolder, запускается отдельный процесс,
    // по окончанию имитится сигнал signalZipFinished(int) с кодом результата.
    // в файле архива убирается рассширение исходного файла и добавляется рассширение архива.
    void tryZipFile(QString);

    void openTmpFile(); // открыть временный файл
    void closeTmpFile(bool add_cur_buff); // закрыть временный файл, параметр указывает необходимость добавить туда текущий буфер перед закрытием
    void reset(); // сбрасывает буфер и временную метку m_lastTs

    inline void setCompressLevel(quint8 a) {if (a > 0 && a <= 9) m_compressLevel = a;}
    inline quint8 compressLevel() const {return m_compressLevel;}

    inline const QByteArray& buffer() const {return m_buffer;}
    inline void clearBufer() {m_buffer.clear();}
    //inline void setWorkingFolder(QString s) {m_workingFolder = s.trimmed();}
    //inline void setCompressMetod(int m) {m_zipType = m;}
    inline void setTmpFile(QString fname) {m_tmpFile = fname.trimmed();}

protected:
    quint8 m_compressLevel; //степень сжатия от 0 до 9 (0 - без сжатия) // на данный момент не задействована
    QByteArray  m_buffer; // динамический буфер для накопления, данные подверженные сжатию.
    //int m_zipType; // метод архивирования/извлечения, элемент ZipType

    // полный путь к рабочей папке с  файлами-архивами, если папка не задана или такой не существует то класс будет выдавать ошибку.
    //QString m_workingFolder;

    // имя временного файла для накопления данных (как правило за сутки), короткое имя без полного пути example: arch_data.dat.
    // файл будет лежать и копиться в m_workingFolder.
    QString m_tmpFile;
    QFile *f_tmp; // дискриптор открытого  m_tmpFile, если null то временный файл  не был открыт

    // интервал добавления буфера в m_tmpFile, сек., -1 значит данные сразу добавляются в файл без накопления  m_buffer.
    // при успешном добавлении в файл, m_buffer затирается.
    int m_tmpAppendInterval;
    qint64 m_lastTs; // временная метка, последнего добавления в m_tmpFile (вызов addBufferData)

    //LProcessObj *m_proc; // объект для запуска процесса архивации

    void appendTmpFile(); // добавить текущий буфер в f_tmp, f_tmp должен быть открыт
    //void initProcessor();
    void startZipProcess(const QString&); // запустить линуксовую команду архивации файла (m_proc)


protected slots:
    void slotProcFinished(); // выполняется когда завершает работу объект m_proc

private:
    // возвращает имя архивного файла по имени исходного,
    // подменяет рассширение взависимости от m_zipType,
    // если у исходного нет рассширения то просто добавляет к имени.
    // в случае любой ошибки вернет пустую строку.
    QString zipExtentionByInputFile(const QString&) const;


};


#endif // LZIP_OBJECT_H




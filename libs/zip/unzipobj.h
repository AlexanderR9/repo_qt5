#ifndef UNZIPOBJ_H
#define UNZIPOBJ_H


#include "zipobj_base.h"


//LUnzipObj
// обьект предназначен для разархивации файлов-архивов в рабочей папке (вызов tryUnzip).
// предварительно перед вызовом необходимо задать f_list (список файлов-архивов).
// вся работа ведется в рабочей папке m_workingFolder, которую нужно задать сразу после создания объекта.
class LUnzipObj : public LZipObj_base
{
    Q_OBJECT
public:
    LUnzipObj(QObject *parent = NULL);
    virtual ~LUnzipObj() {}

    virtual QString name() const {return QString("unzip_obj");}

    // подготовить список файлов для распаковки, задаются без полного пути.
    // перед добавлением в f_list каждый элемент проверится на наличие файла в m_workingFolder,
    // а так же что этот файл является архивом.
    void prepareFileList(const QStringList&);

    // попытка разархивировать список файлов f_list
    void tryUnzip();

protected:
    QStringList f_list; // список файлов предназначенных для распаковки, задаются без полного пути, должны лежать в m_workingFolder
    QString m_curFile; // текущий распаковываемый файл

    void unzipNextFile(); // приступить к распаковке следующего файла
    void reset(); // сбрасывает состояние объекта

protected slots:
    void slotProcFinished(); // выполняется каждый раз когда завершает работу объект m_proc
    void slotZipTimer(); // выполняется по сигналу m_timer

signals:
    void signalUnzippingFinished(); // имитится по завершению разархивации всего списка f_list, при этом команда выполнилась успешно

    // сигнал имитится когда невозможно выполнить задачу , параметр - код ошибки
    // -1 - процесс уже выполняется
    // -2 - m_state == (zosSettingsInvalid)
    // -3 - f_list is empty
    //void signalInvalidState(int);

};



#endif // UNZIPOBJ_H




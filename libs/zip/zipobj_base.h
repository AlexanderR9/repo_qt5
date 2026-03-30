#ifndef ZIPOBJ_BASE_H
#define ZIPOBJ_BASE_H

#include "lsimpleobj.h"

#include <QMutex>

class LProcessObj;
class QTimer;


// базовый класс для использования архиватора linux
class LZipObj_base : public LSimpleObject
{
    Q_OBJECT
public:
    enum ZipType {ztQt = 71, ztGZip, ztTar};  // тип архиватора
    enum ZipObjState {zosIdle = 81, zosProcessZipping, zosProcessUnzipping, zosSettingsInvalid = -1}; // текущее состояние объекта

    LZipObj_base(QObject *parent = NULL);
    virtual ~LZipObj_base() {}

    inline void setWorkingFolder(QString s) {m_workingFolder = s.trimmed();}
    inline void setCompressMetod(int m) {m_zipType = m;}

    bool processBuzy() const; // признак что объект m_proc что-то выполняет в текущий момент

    static bool isArchiveFile(const QString&); // признак того что указанные файл является архивом, т.е. имя содержит '.gz'

protected:
    int m_zipType; // метод архивирования/извлечения, элемент ZipType
    int m_state; // состояние объекта
    LProcessObj *m_proc; // объект для запуска процесса архивации
    QTimer *m_timer; // штатный тиковый таймер для отслеживания состояния работы m_proc и объекта целиком
    quint32 m_tickCounter; // простой счетчик тиков таймера

    // полный путь к рабочей папке с  файлами-архивами, если папка не задана или такой не существует то класс будет выдавать ошибку.
    // в этой папке копятся файлы данных / архивируются файлы данных / разархивируются файлы данных.
    QString m_workingFolder;

    QMutex m_mutex; // объект для блокировки доступа к буферу данных на время выполнения функции

    void initProcessor();

protected slots:
      virtual void slotProcFinished() = 0; // выполняется когда завершает работу объект m_proc
      virtual void slotZipTimer(); // выполняется по сигналу m_timer

signals:
    void signalProcFinished(int); // сигнал имитится всегда по завершению выполнения m_proc, параметр - код ошибки, 0 - ОК.


};


#endif // ZIPOBJ_BASE_H



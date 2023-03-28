#ifndef LSOCAT_OBJECT_H
#define LSOCAT_OBJECT_H


#include "processobj.h"


//LSocatObj
class LSocatObj : public LProcessObj
{
    Q_OBJECT
public:
    enum SocatObjMode {smChecking = 551, smComCom, smComPipe, smCurrentProcess, smKillProcess, smNone = 0};

    LSocatObj(QObject *parent = NULL);
    virtual ~LSocatObj() {}

    void setTTY(QString, QString);
    void checkUtility(); //проверить наличие socat в ОС
    void createComToCom(); //создать виртуальный канал типа COM-COM (для локальной машины)
    void createComToPipe(); //создать виртуальный канал типа COM-PIPE (для соединения с COM портом виртуальной машины)
    void checkCurrentProcess(); //проверка наличия запущенных процессов socat
    void killProcess(); //срубить все запущенные процессы socat
    void stopChannel(); //остановить утилиту (применяется если текущее состояние smComPipe или smComCom)

    inline bool existUtility() const {return m_exist;}

    virtual QString name() const {return QString("socat_obj");}

protected:
    bool m_exist; //признак того что утитита socat присутствует в ОС
    int m_mode;
    QString m_ownTTY;
    QString m_destinationTTY;
    QList<quint32> m_processID; //идентификаторы текущих запущенных процессов socat, заполняется при выполнении checkCurrentProcess()

    bool isCkeckMode() const;
    void finishCheking();
    void finishComCom();
    void finishComPipe();
    void finishCurrentProcess();
    void finishKillCommand();
    void checkTTY(); //проверить корректность значений переменных m_ownTTY и m_destinationTTY

protected slots:
    void slotParentFinished();


};


#endif // LSOCAT_OBJECT_H

#ifndef LSOCAT_OBJECT_H
#define LSOCAT_OBJECT_H


//#include "lsimpleobj.h"
#include "processobj.h"


//class QProcess;
//class QTimer;


//LSocatObj
class LSocatObj : public LProcessObj
{
    Q_OBJECT
public:
    enum SocatObjMode {smChecking = 551, smComCom, smComPipe, smCurrentProcess, smNone = 0};

    LSocatObj(QObject *parent = NULL);
    virtual ~LSocatObj() {}

    void setTTY(QString, QString);
    void checkUtility();
    void createComToCom();
    void createComToPipe();
    void checkCurrentProcess();

    inline bool existUtility() const {return m_exist;}

    virtual QString name() const {return QString("socat_obj");}

protected:
    bool m_exist;
    int m_mode;
    QString m_ownTTY;
    QString m_destinationTTY;

    bool isCkeckMode() const;
    void finishCheking();
    void finishComCom();
    void finishComPipe();
    void finishCurrentProcess();
    void checkTTY(); //проверить значения m_ownTTY и m_destinationTTY


protected slots:
    void slotParentFinished();


};


#endif // LSOCAT_OBJECT_H

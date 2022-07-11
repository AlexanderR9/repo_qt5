#ifndef LPROCESS_OBJECT_H
#define LPROCESS_OBJECT_H


#include "lsimpleobj.h"

class QProcess;



//LProcessObj
class LProcessObj : public LSimpleObject
{
    Q_OBJECT
public:
    LProcessObj(QObject *parent = NULL);
    virtual ~LProcessObj() {}

    inline void setCommand(QString cmd_name) {m_command = cmd_name.trimmed();}
    inline void setSudo(bool b) {m_needSudo = b;}
    inline QString hasErr() const {return m_err;}

    void startCommand();

protected:
    QProcess	*m_process;
    QString     m_command;
    bool        is_running;
    bool        m_needSudo;
    QString     m_err;


protected slots:



signals:
    void signalFinished();


};


#endif // COMPORT_OBJECT_H

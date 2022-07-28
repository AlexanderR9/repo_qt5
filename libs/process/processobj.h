#ifndef LPROCESS_OBJECT_H
#define LPROCESS_OBJECT_H


#include "lsimpleobj.h"

class QProcess;
class QTimer;


//LProcessObj
class LProcessObj : public LSimpleObject
{
    Q_OBJECT
public:
    LProcessObj(QObject *parent = NULL);
    virtual ~LProcessObj() {}

    inline void setCommand(QString cmd_name) {m_command = cmd_name.trimmed();}
    inline void setSudo(bool b) {m_needSudo = b;}
    inline QString err() const {return m_err;}
    inline bool isRunning() const {return is_running;}
    inline QString buffer() const {return m_buff;}
    inline bool isOk() const {return m_err.isEmpty();}  //команда выполнилась нормально
    inline QString command() const {return m_command;}
    inline const QStringList& args() const {return m_args;}
    inline bool argsEmpty() const {return m_args.isEmpty();}

    void setArgs(const QStringList& args = QStringList());
    QString strProcessState() const;
    QString fullCommand() const;
    QStringList bufferList() const; // тот же buffer, но разбит на строки если там присутствуют разделители '\n'

    void startCommand(); //запуск выполнения команды
    void breakCommand(); //принудительный останов команды

protected:
    QProcess        *m_process;
    QProcess        *m_processChild; //дочерний процесс, подключается только в случае если в аргументах присутствует символ '|', т.е. выполняется двойная команда
    QString         m_command; //всего одно слово, пример "make", "mkdir"
    QStringList     m_args; //арументы команды(без пробелов), все что идет после самой команды, пример "-t", "2&>1", "/home/roman/tmp"
    bool            is_running;
    bool            m_needSudo;
    QString         m_err;
    QTimer          *m_timer; //таймер периодической проверки выхлопа при выполнении команды
    QString         m_buff; //накапливающийся выхлоп выполняемой текущей команды


    void stopProcessChild();
    void activateProcessChild(int);

protected slots:
    void slotTimer();
    void slotProcessError();
    void slotProcessFinished(int);
    void slotProcessStateChanged();


signals:
    void signalFinished();
    void signalReadyRead();


};


#endif // LPROCESS_OBJECT_H

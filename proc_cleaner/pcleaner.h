#ifndef PCLEANER_H_
#define PCLEANER_H_

#include <QtCore/QCoreApplication>
#include <QTime>


class QTimer;
class LProcessObj;

//структура для хранения параметров каждого процесса системы (демона).
struct PInfo
{
    PInfo() {reset();}

    QString name;
    QString user;
    QTime start_time;
    int pid;
    float load_proc;
    float load_mem;
    QString stat;

    void reset() {name = user = QString(); start_time = QTime(); load_proc = load_mem = 0; pid = -1;}
    void fromLine(const QStringList&);
    QString toStr() const;
    bool isSsl() const {return (stat.trimmed().toLower() == QString("ssl"));}
    bool isRoot() const {return (user.trimmed() == QString("root"));}
    bool isUsrStartPoint() const {return (name.contains("/usr"));}
    bool invalid() const {return (pid < 0 || !start_time.isValid());}
    int runnigSecs() const;

};

// PCleaner
class PCleaner : public QCoreApplication
{
    Q_OBJECT
public:
    PCleaner(int& argc, char** argv);
    virtual ~PCleaner() {}

protected:
    QTimer*          m_timer;
    LProcessObj     *m_proc;
    int              t_counter;
    int              m_needKill; //PID
    QString          m_ourUser;

    void parseOut(const QStringList&);
    void checkKill(const QStringList&);
    void addLog(QString);
    inline bool hasKillProcess() const {return (m_needKill > 0);}

protected slots:
    void slotTimerTick();
    void slotProcFinished();

};



#endif



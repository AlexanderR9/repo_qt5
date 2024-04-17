#ifndef LTHREAD_H
#define LTHREAD_H


//#include "lsimpleobj.h"

#include <QTime>
#include <QThread>

//class QThread;
class QTimerEvent;


//LThreadObj
class LThreadObj : public QThread
{
    Q_OBJECT
public:
    enum ThreadResult {trOk = 0, trTimeoutBreaked = 111, trInvalidParams = -2, trUnknown = -1, trCanNotStartTimer = -3};

    LThreadObj(QObject *parent = NULL);
    virtual ~LThreadObj() {stopThread();}

    virtual void startThread(int cheking_interval = -1);
    virtual QString name() const {return QString("lthread_obj");}

    inline void setTimeout(quint32 t) {m_timeout = t;}
    inline void resetCounter() {m_counter = 0;}
    inline int counter() const {return m_counter;}
    inline QString err() const {return m_err;}
    inline bool hasErr() const {return !m_err.isEmpty();}
    inline void setThreadPriority(int p) {m_priority = p;}

    int elapsed() const; //current elapsed running
    int lastElapsed() const; //last elapsed running or -1

    static QString strResult(int);

protected:
    //QTimer      *m_timer; //таймер периодической проверки выхлопа при выполнении команды
    quint32      m_timeout; //допустимое время выполнения нити, после чего произойдет принудительное перрывание, ms
    QTime        m_runningTime; //переменная для отслеживания длительности выполнения нити
    int          m_counter; //счетчик запусков, -1 значит еще не запускался
    int          m_lastElapsed;
    QString      m_err;
    int          m_priority; //элемент множества Qt  QThread::Priority (default LowPriority)
    //int          m_chekingInterval; //timer interval
    int          m_chekingTimerID;
    int          m_resultCode;

    virtual void stopThread();
    virtual void run();
    virtual void prepare(); //prepare params before starting thread

    virtual void checkInputParams() = 0; //здесь нужно проверить вылидность неких входных данных, зависит от задачи, выполняется прямо перед doWork()
    virtual void doWork() = 0; //здесь нужно переопределить дествия которые будут длительно выполнятся в отдельной нити
    virtual void startWaitingTimer(int);
    void timerEvent(QTimerEvent*);

protected slots:
    //virtual void slotTimer();
    virtual void slotThreadFinished();

signals:
    void signalThreadFinished(int);
    void signalError(const QString&);
    void signalMsg(const QString&);

};

#endif // LTHREAD_H



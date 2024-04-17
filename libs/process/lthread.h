#ifndef LTHREAD_H
#define LTHREAD_H


//#include "lsimpleobj.h"

#include <QTime>
#include <QThread>

//class QThread;
class QTimer;


//LThreadObj
class LThreadObj : public QThread
{
    Q_OBJECT
public:
    enum ThreadResult {trOk = 160, trTimeoutBreaked};

    LThreadObj(QObject *parent = NULL);
    virtual ~LThreadObj() {stopThread();}

    virtual void startThread(int cheking_interval = -1);
    virtual QString name() const {return QString("lthread_obj");}

    inline void setTimeout(quint32 t) {m_timeout = t;}
    inline void resetCounter() {m_counter = 0;}
    inline int counter() const {return m_counter;}
    inline QString err() const {return m_err;}
    inline bool hasErr() const {return !m_err.isEmpty();}


    int elapsed() const; //current elapsed running
    int lastElapsed() const; //last elapsed running or -1

protected:
    QTimer      *m_timer; //таймер периодической проверки выхлопа при выполнении команды
    quint32      m_timeout; //допустимое время выполнения нити, после чего произойдет принудительное перрывание, ms
    QTime        m_runningTime; //переменная для отслеживания длительности выполнения нити
    int          m_counter; //счетчик запусков, -1 значит еще не запускался
    int          m_lastElapsed;
    QString      m_err;

    virtual void stopThread();
    virtual void run();
    virtual void prepare(); //prepare params before starting thread

    virtual void doWork() = 0; //здесь нужно переопределить дествия которые будут выполнятся в отдельной нити

protected slots:
    virtual void slotTimer();
    virtual void slotThreadFinished();

signals:
    void signalThreadFinished(int);

};

#endif // LTHREAD_H



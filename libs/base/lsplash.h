#ifndef LSPLASH_H
#define LSPLASH_H

#include <QWidget>

class QLabel;
class QProgressBar;
class QTimer;


///////////LSplash//////////////////////////
class LSplash: public QWidget
{
    Q_OBJECT
public:
    LSplash(QWidget *parent = 0);
    virtual ~LSplash() {}

    void startDelay(const QString &s = QString());
    void stopDelay();

    void initProgress(quint32); //param - timeout seconds
    void startProgress(const QString &s = QString()); //запуск окна с прогрессом, окно после окончания таймаута закроется автоматом

protected:
    QLabel *m_label;
    QProgressBar *m_progressBar;
    quint32 m_progressDelay; //количество секунд, задержка окна и визуальный отчет в прогрессе
    quint16 m_timerCounter; //счетчик тиков m_progressTimer
    QTimer *m_progressTimer; //таймер для обратного отчета, после того как пройдет m_progressDelay окно закроется

protected slots:
    void slotProgressTimer();


};





#endif // LSPLASH_H

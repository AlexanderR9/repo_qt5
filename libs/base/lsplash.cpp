#include "lsplash.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTest>
#include <QTimer>
#include <QProgressBar>

#define PROGRESS_TIMER_INTERVAL        500


/////////////LSplash/////////////////////////
LSplash::LSplash(QWidget *parent)
    :QWidget(parent, Qt::SplashScreen),
     m_label(NULL),
     m_progressBar(NULL),
     m_progressDelay(100),
     m_timerCounter(0)
{
    if (layout()) delete layout();
    setLayout(new QVBoxLayout(0));

    m_label = new QLabel("Loading");
    layout()->addWidget(m_label);

    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    m_label->setFont(font);
    QPalette plt;
    plt.setColor(QPalette::WindowText, Qt::blue);
    m_label->setPalette(plt);

    stopDelay();
}
void LSplash::startDelay(const QString &s)
{
    if (!this->isHidden()) return;

    QString caption = s.isEmpty() ? tr("Loading...........") : s;
    m_label->setText(caption);
    show();
    QTest::qWait(100);
}
void LSplash::startProgress(const QString &s)
{
    if (!m_progressBar) {qWarning("WARNING m_progressBar is NULL"); return;}

    m_timerCounter = 0;
    m_progressBar->setValue(0);
    m_progressTimer->start();
    startDelay(s);
}
void LSplash::stopDelay()
{
    if (this->isHidden()) return;

    hide();
    QTest::qWait(100);
}
void LSplash::initProgress(quint32 sec)
{
    m_progressDelay = sec*1000;
    m_timerCounter = 0;
    m_progressBar = new QProgressBar(this);
    layout()->addWidget(m_progressBar);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setMaximumHeight(10);

    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(PROGRESS_TIMER_INTERVAL);
    m_progressTimer->stop();

    connect(m_progressTimer, SIGNAL(timeout()), SLOT(slotProgressTimer()));
}
void LSplash::slotProgressTimer()
{
    m_timerCounter++;
    qDebug()<<QString("LSplash::slotProgressTimer() m_timerCounter=%1, m_progressDelay=%2").arg(m_timerCounter).arg(m_progressDelay);
    if (m_timerCounter*PROGRESS_TIMER_INTERVAL >= m_progressDelay)
    {
        m_progressBar->setValue(100);
        m_progressTimer->stop();
        stopDelay();
    }

    float a = float(m_timerCounter*PROGRESS_TIMER_INTERVAL)/float(m_progressDelay);
    if (a > 1) a = 1;
    qDebug()<<QString("progress value %1(%2%)").arg(a).arg(qRound(a*100));
    m_progressBar->setValue(qRound(a*100));

}


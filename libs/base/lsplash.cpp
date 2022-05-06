#include "lsplash.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTest>



/////////////LSplash/////////////////////////
LSplash::LSplash(QWidget *parent)
    :QWidget(parent, Qt::SplashScreen),
     m_label(NULL)
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
    QString caption = s.isEmpty() ? tr("Loading...........") : s;
    m_label->setText(caption);
    show();
    QTest::qWait(100);
}
void LSplash::stopDelay()
{
    hide();
    QTest::qWait(100);
}


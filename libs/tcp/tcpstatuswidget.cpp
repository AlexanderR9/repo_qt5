#include "tcpstatuswidget.h"


LTCPStatusWidget::LTCPStatusWidget(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);

    setOffState();
    setTextMode("MODE: ???");

}
void LTCPStatusWidget::setTextMode(const QString &text)
{
    textLabel->setText(text);
}
void LTCPStatusWidget::setOffState()
{
    QPixmap pixmap(QString(":/icons/images/ball_gray.svg"));
    iconLabel->setPixmap(pixmap);
}
void LTCPStatusWidget::setListeningState()
{
    QPixmap pixmap(QString(":/icons/images/ball_yellow.svg"));
    iconLabel->setPixmap(pixmap);
}
void LTCPStatusWidget::setConnectedState()
{
    QPixmap pixmap(QString(":/icons/images/ball_green.svg"));
    iconLabel->setPixmap(pixmap);
}
void LTCPStatusWidget::setConnectingState()
{
    QPixmap pixmap(QString(":/icons/images/down.svg"));
    iconLabel->setPixmap(pixmap);
}




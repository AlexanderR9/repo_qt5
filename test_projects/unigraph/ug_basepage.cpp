#include "ug_basepage.h"
#include "ug_apistruct.h"
#include "lhttp_types.h"

#include <QDebug>
#include <QTimer>


//UG_BasePage
UG_BasePage::UG_BasePage(QWidget *parent, int t, int user_type)
    :LSimpleWidget(parent, t),
      m_reqData(NULL),
      m_minUpdatingInterval(120),
      m_timer(NULL),
      m_timerCounter(0),
      m_reqLimit(70)

{
    m_userSign = user_type;
    setObjectName("ug_base_page");

    m_reqData = new UG_APIReqParams();
    m_reqData->req_type = m_userSign;

    m_updateTime = QTime();

    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    m_timer->stop();

}
void UG_BasePage::slotTimer()
{
    m_timerCounter++;
    qDebug()<<QString("PAGE[%1] slotTimer, counter %2").arg(userSign()).arg(m_timerCounter);
}
void UG_BasePage::startUpdating(quint16 t)
{
    qDebug()<<QString("UG_BasePage::startUpdating t=%1  page=%2").arg(t).arg(userSign());
    m_timerCounter = 0;
    m_timer->start(int(t));
}
bool UG_BasePage::updatingRunning() const
{
    return m_timer->isActive();
}
void UG_BasePage::stopUpdating()
{
    qDebug()<<QString("UG_BasePage::stopUpdating  page=%1").arg(userSign());
    m_timer->stop();
}
void UG_BasePage::reset()
{
    if (m_reqData) {delete m_reqData; m_reqData = NULL;}
}
void UG_BasePage::sendRequest(QString name_extra)
{
    m_reqData->name = UG_APIReqParams::strReqTypeByType(m_reqData->req_type, name_extra);
    //m_reqData->is_running = true;
    emit signalMsg(QString("QUERY: %1").arg(m_reqData->query));
    emit signalSendReq(m_reqData);
}
bool UG_BasePage::updateTimeOver(bool forcibly)
{
    QTime ct = QTime::currentTime();
    if (minUpdatingInterval() < 3) return false;
    if (!m_updateTime.isValid() || forcibly) {m_updateTime = ct; return true;}
    if (m_updateTime.secsTo(ct) > minUpdatingInterval()) {m_updateTime = ct; return true;}
    return false;
}



#include "bb_basepage.h"
#include "bb_apistruct.h"
#include "lhttp_types.h"
//#include "apiconfig.h"

#include <QDebug>

//BB_BasePage
BB_BasePage::BB_BasePage(QWidget *parent, int t, int user_type)
    :LSimpleWidget(parent, t),
      m_reqData(NULL),
      m_minUpdatingInterval(120)
{
    m_userSign = user_type;
    setObjectName("bb_base_page");

    m_reqData = new BB_APIReqParams(BB_APIReqParams::strReqTypeByType(m_userSign), hrmGet);
    m_reqData->req_type = m_userSign;

    m_updateTime = QTime();
}
void BB_BasePage::reset()
{
    if (m_reqData) {delete m_reqData; m_reqData = NULL;}
}
void BB_BasePage::sendRequest(int limit, QString name_extra)
{
    if (limit > 0)
        m_reqData->params.insert("limit", QString::number(limit));

    m_reqData->name = BB_APIReqParams::strReqTypeByType(m_reqData->req_type, name_extra);
    m_reqData->is_running = true;

    emit signalSendReq(m_reqData);
}
bool BB_BasePage::updateTimeOver(bool forcibly)
{
    QTime ct = QTime::currentTime();
    if (minUpdatingInterval() < 3) return false;
    if (!m_updateTime.isValid() || forcibly) {m_updateTime = ct; return true;}
    if (m_updateTime.secsTo(ct) > minUpdatingInterval()) {m_updateTime = ct; return true;}
    return false;
}



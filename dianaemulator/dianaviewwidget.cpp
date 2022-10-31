#include "dianaviewwidget.h"
#include "dianaobj.h"
#include "xmlpackview.h"
#include "xmlpack.h"
#include "xmlpackelement.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QSettings>


//DianaViewWidget
DianaViewWidget::DianaViewWidget(const QString &diana_name, QWidget *parent)
    :LSimpleWidget(parent, 22),
    m_inView(NULL),
    m_outView(NULL),
    m_dianaObj(NULL),
    m_autoUpdatePackValues(false)
{
    m_dianaObj = new DianaObject(diana_name, this);
    setObjectName(QString("%1_view_widget").arg(diana_name));

    initWidget();

    connect(m_dianaObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_dianaObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_dianaObj, SIGNAL(signalSendMsgOk(const QString&)), this, SIGNAL(signalSendMsgOk(const QString&)));
    connect(m_dianaObj, SIGNAL(signalReceiveMsgOk(const QString&)), this, SIGNAL(signalReceiveMsgOk(const QString&)));
    connect(m_dianaObj, SIGNAL(signalSendMsgErr(const QString&)), this, SIGNAL(signalSendMsgErr(const QString&)));
    connect(m_dianaObj, SIGNAL(signalReceiveMsgErr(const QString&)), this, SIGNAL(signalReceiveMsgErr(const QString&)));


}
void DianaViewWidget::initWidget()
{
    m_inView = new LXMLPackView("Input", this);
    m_outView = new LXMLPackView("Output", this);

    h_splitter->addWidget(m_inView);
    h_splitter->addWidget(m_outView);
    m_inView->setReadOnly(false);
    m_outView->setReadOnly(true);
}
void DianaViewWidget::loadMQPacket(const QString &fname)
{
    if (fname.contains(DianaObject::inputType()))
    {
        loadPack(m_inView, fname);
        if (!m_inView->invalid())
            emit signalMQCreated(m_dianaObj->name().toLower(), m_inView->getPacket()->size(),  m_dianaObj->inputQueue());
    }
    else if (fname.contains(DianaObject::outputType()))
    {
        loadPack(m_outView, fname);
        if (!m_outView->invalid())
            emit signalMQCreated(m_dianaObj->name().toLower(), m_outView->getPacket()->size(), m_dianaObj->outputQueue());
    }
    else emit signalError(QString("%1: invalid filename [%2]").arg(name()));
}
void DianaViewWidget::loadPack(LXMLPackView *view, const QString &fname)
{
    qDebug()<<QString("DianaViewWidget::loadPack  %1").arg(fname);
    LXMLPackObj *pack = new LXMLPackObj(fname, this);
    connect(pack, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(pack, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

    bool ok;
    pack->tryLoadPacket(ok);
    if (ok)
    {
        view->setPacket(pack);
        m_dianaObj->addQueue(view->getPacket()->caption());
        emit signalMsg("Ok!");
    }
    else emit signalError("fault");
    emit signalMsg(QString(100, QChar('-')));
}
void DianaViewWidget::setExpandLevel(int level)
{
    m_inView->setExpandLevel(level);
    m_outView->setExpandLevel(level);
}
void DianaViewWidget::setDoublePrecision(quint8 p)
{
    m_inView->setDoublePrecision(p);
    m_inView->updateValues();
    m_outView->setDoublePrecision(p);
    m_outView->updateValues();
}
void DianaViewWidget::updateMQState()
{
    m_dianaObj->updateMQState();
}
void DianaViewWidget::sendMsgToQueue()
{
    if (m_inView->invalid()) return;

    if (m_autoUpdatePackValues)
        m_inView->nextRandValues();

    QByteArray ba;
    m_inView->fromPacket(ba);
    m_dianaObj->sendMsgToQueue(ba);
}




#include "dianaviewwidget.h"
#include "lprotocol.h"
#include "lstatic.h"
#include "dianaobj.h"
#include "xmlpackview.h"
#include "xmlpack.h"
#include "xmlpackelement.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QIcon>
#include <QSettings>


//DianaViewWidget
DianaViewWidget::DianaViewWidget(const QString &diana_name, QWidget *parent)
    :LSimpleWidget(parent, 22),
    m_inView(NULL),
    m_outView(NULL),
    m_dianaObj(NULL)
{
    m_dianaObj = new DianaObject(diana_name, this);
    setObjectName(QString("%1_view_widget").arg(diana_name));

    initWidget();

    connect(m_dianaObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_dianaObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
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
    if (fname.contains("input"))
    {
        loadPack(m_inView, fname);
        if (!m_inView->invalid())
            emit signalMQCreated(m_dianaObj->name().toLower(), m_inView->getPacket()->size(),  m_dianaObj->inputQueue());
    }
    else if (fname.contains("output"))
    {
        loadPack(m_outView, fname);
        if (!m_outView->invalid())
            emit signalMQCreated(m_dianaObj->name().toLower(), m_outView->getPacket()->size(), m_dianaObj->outputQueue());
    }
    else emit signalError(QString("%1: invalid filename [%2]").arg(name()));
}
void DianaViewWidget::loadPack(LXMLPackView *view, const QString &fname)
{
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





/*

void ViewWidget::toLeft(int bytes_order, bool single_floating)
{
    const LXMLPackObj *pack = m_outView->getPacket();
    if (!pack)
    {
        m_protocol->addText("Packet-2 is NULL", LProtocolBox::ttErr);
        return;
    }

    QByteArray ba;
    m_outView->fromPacket(ba, single_floating);
    m_protocol->addText(QString("Out packet size %1 bytes").arg(ba.size()), LProtocolBox::ttData);
    m_protocol->addText(LStatic::baToStr(ba, 4));

    //------------------------------------------
    setPacketData(m_inView, ba, bytes_order, single_floating);
}
void ViewWidget::toRight(int bytes_order, bool single_floating)
{
    const LXMLPackObj *pack = m_inView->getPacket();
    if (!pack)
    {
        m_protocol->addText("Packet-1 is NULL", LProtocolBox::ttErr);
        return;
    }

    QByteArray ba;
    m_inView->fromPacket(ba, single_floating);
    m_protocol->addText(QString("Input packet size %1 bytes").arg(ba.size()), LProtocolBox::ttData);
    m_protocol->addText(LStatic::baToStr(ba, 4));


    //------------------------------------------
    setPacketData(m_outView, ba, bytes_order, single_floating);
}
void ViewWidget::ViewWidget::setPacketData(LXMLPackView *view, const QByteArray &ba, int bytes_order, bool single_floating)
{
    if (!view) return;

    bool ok;
    view->setPacketByteOrder(bytes_order);
    view->setPacketData(ba, ok, single_floating);
    view->updateValues();
    if (!ok) slotError(QString("result fault"));
    else m_protocol->addText("Ok!", LProtocolBox::ttOk);
}

*/




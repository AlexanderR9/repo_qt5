#include "viewwidget.h"
#include "lprotocol.h"
#include "xmlpackview.h"
#include "xmlpack.h"
#include "lstatic.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QIcon>
#include <QSettings>


//ViewWidget
ViewWidget::ViewWidget(QWidget *parent)
    :LSimpleWidget(parent, 31),
      m_inView(NULL),
      m_outView(NULL),
      m_protocol(NULL)
{
    setObjectName("xmlpacket_view_widget");

    initWidget();

}
void ViewWidget::loadInPack(const QString &fname)
{
    slotMessage(QString("loading %1 to input packet ......").arg(fname));
    loadPack(m_inView, fname);
}
void ViewWidget::loadOutPack(const QString &fname)
{
    slotMessage(QString("loading %1 to output packet ......").arg(fname));
    loadPack(m_outView, fname);
}
void ViewWidget::loadPack(LXMLPackView *view, const QString &fname)
{
    LXMLPackObj *pack = new LXMLPackObj(fname, this);
    connect(pack, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(pack, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    bool ok;
    pack->tryLoadPacket(ok);
    if (ok)
    {
        view->setPacket(pack);
        m_protocol->addText("Ok!", LProtocolBox::ttOk);
    }
    else slotError("fault");
}
void ViewWidget::initWidget()
{
    m_inView = new LXMLPackView("Packet 1", this);
    m_outView = new LXMLPackView("Packet 2", this);
    m_protocol = new LProtocolBox(false, this);

    h_splitter->addWidget(m_inView);
    h_splitter->addWidget(m_outView);
    v_splitter->addWidget(m_protocol);

    m_inView->setReadOnly(false);
    m_outView->setReadOnly(true);

}
void ViewWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
}
void ViewWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
}
void ViewWidget::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void ViewWidget::slotMessage(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
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
void ViewWidget::setDoublePrecision(quint8 p)
{
    m_inView->setDoublePrecision(p);
    m_outView->setDoublePrecision(p);
}



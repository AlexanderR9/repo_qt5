#include "viewwidget.h"
#include "lprotocol.h"
#include "xmlpackview.h"
#include "xmlpack.h"

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



 #include "lprotocol.h"

 #include <QHBoxLayout>
 #include <QTime>
 #include <QDebug>


////////////LProtocolBox///////////////////////
LProtocolBox::LProtocolBox(bool nt, QWidget *parent)
    :QGroupBox(tr("Protocol"), parent),
     m_protocol(NULL),
     m_needTime(nt)
{
    if (layout()) delete layout();
    setLayout(new QHBoxLayout(0));

    m_protocol = new QTextEdit(this);
    layout()->addWidget(m_protocol);
    m_protocol->setReadOnly(true);

    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
}
void LProtocolBox::addText(QString text, int type)
{
    if (text.isEmpty())
    {
        addSpace();
        return;
    }

    QString s = QTime::currentTime().toString("hh:mm:ss.zzz");
    s = QString("%1.....%2").arg(s).arg(text);
    if (!m_needTime) s = text;
    QColor color = Qt::black;

    switch (type)
    {
        case ttOk:
        {
            color = Qt::blue;
            break;
        }
        case ttFile:
        {
            color = Qt::darkGreen;
            break;
        }
        case ttData:
        {
            color = Qt::darkYellow;
            break;
        }
        case ttErr:
        {
            s = QString("%1 - [%2]").arg(s).arg(tr("Error"));
            color = Qt::red;
            break;
        }
        case ttWarning:
        {
            s = QString("%1 - [%2]").arg(s).arg(tr("Warning"));
            color = QColor(200, 100, 0);
            break;
        }
        case ttUnknow:
        {
            s = QString("%1             [???]").arg(s);
            color = Qt::gray;
            break;
        }

        default: break;
    }
    m_protocol->setTextColor(color);
    m_protocol->append(s);

}




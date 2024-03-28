#include "lprotocol.h"

#include <QHBoxLayout>
#include <QTime>
#include <QTimer>
#include <QTextEdit>
#include <QDebug>
#include <QTextCursor>


#define CHECK_LINES_INTERVAL        14*1000

////////////LProtocolBox///////////////////////
LProtocolBox::LProtocolBox(bool nt, QWidget *parent)
    :QGroupBox(tr("Protocol"), parent),
     m_protocol(NULL),
     m_checkSizeTimer(NULL),
     m_needTime(nt),
     m_maxLines(-1)
{
    if (layout()) delete layout();
    setLayout(new QHBoxLayout(0));

    m_protocol = new QTextEdit(this);
    layout()->addWidget(m_protocol);
    m_protocol->setReadOnly(true);

    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    m_checkSizeTimer = new QTimer(this);
    m_checkSizeTimer->setInterval(CHECK_LINES_INTERVAL);
}
void LProtocolBox::setMaxLines(int max)
{
    disconnect(m_checkSizeTimer, SIGNAL(timeout()), this, SLOT(slotCheckMaxLines()));
    m_maxLines = max;

    if (m_maxLines > 10)
    {
        connect(m_checkSizeTimer, SIGNAL(timeout()), this, SLOT(slotCheckMaxLines()));
        m_checkSizeTimer->start();
    }
    else m_checkSizeTimer->stop();
}
qint64 LProtocolBox::currentLineCount() const
{
    if (!m_protocol) return -1;
    return m_protocol->document()->lineCount();
}
void LProtocolBox::slotCheckMaxLines()
{
    qint64 over_lines = (currentLineCount() - m_maxLines - 1);
    if (over_lines > 0)
    {
        qDebug()<<QString("NEED CUT PROTOCOL: m_maxLines=%1  over_lines=%2").arg(m_maxLines).arg(over_lines);
        QTextCursor tc = m_protocol->textCursor();
        tc.movePosition(QTextCursor::Start);
        tc.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, over_lines);
        tc.removeSelectedText();
        tc.deletePreviousChar(); // clean up new line
        m_protocol->moveCursor(QTextCursor::End);
    }
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
        case ttNote:
        {
            color = Qt::lightGray;
            m_protocol->setFontItalic(true);
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
    m_protocol->setFontItalic(false);

}
void LProtocolBox::clearProtocol()
{
    m_protocol->clear();
}
void LProtocolBox::addSpace()
{
    m_protocol->append(QString());
}




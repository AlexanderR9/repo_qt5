#include "htmlnode.h"


#include <QDebug>

/*
#include <QTextDocument>
#include <QTextFrame>
#include <QElapsedTimer>

#define SERVICE_SYMBOL1     QString("\t")
#define SERVICE_SYMBOL2     QString("\r")
#define SERVICE_SYMBOL3     QString("\n")
#define SPACE_SYMBOL        QString(" ")
#define START_TAG_SYMBOL    QString("<")
#define END_TAG_SYMBOL      QString(">")
*/



bool MyHTMLScriptNode::invalid() const
{
    if (m_startPos < 0) return true;
    if (m_startPos >= m_endPos) return true;
    return !m_data.isEmpty();
}



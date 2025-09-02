#ifndef LPROTOCOL_H
#define LPROTOCOL_H

#include <QGroupBox>

class QTextEdit;
class QTimer;


///////////LProtocolBox/////////////////////
class LProtocolBox : public QGroupBox
{
    Q_OBJECT
public:
    enum TextType {ttText = 0, ttErr, ttWarning, ttOk, ttFile, ttData, ttNote, ttUnknow = -1};

    LProtocolBox(bool nt, QWidget *parent = 0);
    virtual ~LProtocolBox() {}

    void addText(QString s, int t = 0);
    void setMaxLines(int);
    void clearProtocol();
    void addSpace();
    void moveScrollDown() const;

    qint64 currentLineCount() const;

protected:
    QTextEdit           *m_protocol;
    QTimer              *m_checkSizeTimer;
    bool                m_needTime;
    int                 m_maxLines; //максимальное количество строк

protected slots:
    void slotCheckMaxLines();

};




#endif




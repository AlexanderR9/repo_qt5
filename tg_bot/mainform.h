#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox        *m_protocol;

    QString projectName() const {return "tgbot";}
    QString mainTitle() const {return QString("test tg (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

protected slots:
    void slotAction(int); //virtual slot from parent

};




#endif


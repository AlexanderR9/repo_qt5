#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class LBot;



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    

protected:
    LProtocolBox        *m_protocol;
    LBot                *l_bot;

    QString projectName() const {return "tgbot";}
    QString mainTitle() const {return QString("test tg (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void loadConfig();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);

};




#endif


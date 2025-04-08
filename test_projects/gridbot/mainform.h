#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class GBotPage;
class QSplitter;



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
    virtual void init();

protected:
    LProtocolBox        *m_protocol;
    GBotPage            *m_botPage;
    QSplitter           *v_splitter;

    QString projectName() const {return "gridbot_tester";}
    QString mainTitle() const {return QString("test gridbot (Qt5)!");}
    void save();
    void load();


    void initActions();
    void initWidgets();
    void initCommonSettings();


protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);

};




#endif


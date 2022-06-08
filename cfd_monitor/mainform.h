#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

#include <QMap>

class LProtocolBox;
class LHTMLRequester;
class QSplitter;
class QTextEdit;
class MyHTMLParser;
class QWebEngineView;
class QProgressBar;
class QGroupBox;
class QTimer;
class LHTMLPageRequester;
class QTabWidget;
class BasePage;
class CFDConfigObject;
class TGBot;


// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox            *m_protocol;
    QTabWidget              *m_tab;
    QSplitter               *v_splitter;
    QMap<int, BasePage*>     m_pages;
    CFDConfigObject         *m_configObj;
    TGBot                   *m_bot;

    //QTextEdit           *m_textView;
    QTimer              *m_timer;
    //LHTMLPageRequester  *m_pageRequester;
    //QStringList         m_couples;

    QString projectName() const {return "cfdmonitor";}
    QString mainTitle() const {return QString("CFD monitoring (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void initPages();
    void initTab();
    void initSplitter();
    void initConfigObj();
    void initBotObj();
    void save();
    void load();

    void start();
    void stop();
    void updateActionsEnable(bool);
    void fillConfigPage();

    //QString currentUrl() const;
    //QString viewType() const;
    int reqInterval() const;
    //void tryRequest();
    //void parsePageData();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
    void slotTimer();
    //void slotDataReady();
    //void slotFinished(bool);


};




#endif


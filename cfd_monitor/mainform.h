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
class CFDCalcObj;
struct LogStruct;


// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox            *m_protocol;
    QSplitter               *v_splitter;
    QTabWidget              *m_tab;
    QMap<int, BasePage*>     m_pages;
    CFDConfigObject         *m_configObj;
    TGBot                   *m_bot;
    QTimer                  *m_timer;
    CFDCalcObj              *m_calcObj;

    QString projectName() const {return "cfdmonitor";}
    QString mainTitle() const {return QString("CFD monitoring (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void initPages();
    void initTab();
    void initSplitter();
    void initConfigObj();
    void initCalcObj();
    void initBotObj();
    void save();
    void load();

    void start();
    void stop();
    void updateActionsEnable(bool);
    void fillPages();
    int reqInterval() const;
    quint8 chartPointsSize() const;

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
    void slotTimer();
    void slotAppSettingsChanged(QStringList);

signals:
    void signalSendLog(const LogStruct&);
    void signalPointsSizeChanged(quint8);


};




#endif


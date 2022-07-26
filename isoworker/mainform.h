#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"


class LProtocolBox;
class QSplitter;
class QTextEdit;
class QProgressBar;
class LProcessObj;
class ParamsPage;
class QTimer;



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    enum ISOStages {isoStoped = 270, isoStarting, isoInitSourceDirs, isoFinishedAll, isoMakeNext, isoProcessRun, isoNeedCalcMD5, isoNeedBreak = -1};

    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    ParamsPage              *m_paramsPage;
    LProtocolBox            *m_protocol;
    QSplitter               *v_splitter;
    LProcessObj             *m_processObj;
    QStringList             m_sourceDirISO;
    int                     m_stage;
    QTimer                  *m_timer;
    QString                 m_curISOFile; //full path


    QString projectName() const {return "isoworker";}
    QString mainTitle() const {return QString("ISO worker (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();


    void start();
    void stopBreak();
    void updateActionsEnable(bool);
    void readParamsPage();

    //cd rw
    void prepareSourceDirISO();
    void makeISO();
    void calcMD5();
    void parseMD5();
    QString sourcePath() const;
    void printNextProcessCommand();
    void checkProcessFinishedResult();
    void finishedAll();


    void save();
    void load();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
//    void slotAppSettingsChanged(QStringList);
    void slotReadyRead();
    void slotFinished();
    void slotTimer();


private:
    QString isoFileNameBySourceName(const QString&) const;
    QString isoLabelBySourceName(const QString&) const;
    QStringList makeArgsBySourcePath(const QString&, const QString&) const;


};




#endif


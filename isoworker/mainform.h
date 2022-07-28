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
    enum ISOStages {isoStoped = 270, isoStarting, isoInitSourceDirs, isoFinishedAll, isoMakeNext, isoProcessRun,
                    isoNeedCalcMD5,  isoNeedCalcMD5_CD, isoBurningCD,  isoEraseCD, isoEjectCDROM,
                    isoUmountCD, isoNeedBreak = -1};

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
    int                     m_cdBlockSize;


    QString projectName() const {return "isoworker";}
    QString mainTitle() const {return QString("ISO worker (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void readParamsPage();

    void startMakerISO();
    void stopBreak();
    void stopOk();
    void updateActionsEnable(bool);
    void prepareSourceDirISO();
    void makeISO();
    void calcMD5();
    void calcMD5_CD();
    void parseMD5();
    QString sourcePath() const;
    QString burnSpeed() const;
    QString cdDevice() const;
    void printNextProcessCommand();
    void finishedAll();
    void tryBurn();
    void startBurning(const QString&);
    void eject();
    void umount();
    void tryErase();
    void startErase();
    void runProcess(int);
    void prepareCommand(QString);

    //finished process funcs
    void checkProcessFinishedResult();
    void checkISOProcessFinishedResult();
    void checkBurnProcessFinishedResult();
    void checkEraseProcessFinishedResult();
    void checkEjectProcessFinishedResult();
    void checkMD5CDProcessFinishedResult();
    void checkUmountProcessFinishedResult();


    void save();
    void load();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
    void slotAppSettingsChanged(QStringList);
    void slotReadyRead();
    void slotFinished();
    void slotTimer();


private:
    QString isoFileNameBySourceName(const QString&) const;
    QString isoLabelBySourceName(const QString&) const;
    QStringList makeArgsBySourcePath(const QString&, const QString&) const;


};




#endif


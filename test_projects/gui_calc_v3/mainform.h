#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class QSplitter;
class ParamsWidget;
class LPoolCalcObj;
class LpPutPage;
class QTabWidget;


// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox                *m_protocol;
    ParamsWidget                *m_paramsWidget;
    LpPutPage                   *m_lpWidget;
    QSplitter                   *v_splitter;
    LPoolCalcObj                *m_calcObj;
    QTabWidget                  *w_tab;

    QString projectName() const {return "uni_calc_v3";}
    QString mainTitle() const {return QString("Pool calculator (Qt5)!");}

    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void actStart();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotAppSettingsChanged(QStringList);

};


#endif


#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

#include <QMap>

class LProtocolBox;
class QSplitter;
class ParamsWidget;
class LSimpleWidget;
//class TickObj;
class LPoolCalcObj;




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
    QSplitter                   *v_splitter;
    //TickObj                     *m_tickObj;
    LPoolCalcObj                     *m_calcObj;

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


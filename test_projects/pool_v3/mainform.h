#ifndef TEST_MAINFORM_H
#define TEST_MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class QSplitter;
class CentralWidgetV3;
class DefiConfigLoader;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}

protected:
    LProtocolBox        *m_protocol;
    QSplitter           *v_splitter;
    CentralWidgetV3     *m_centralWidget;
    DefiConfigLoader    *m_configLoader;

    QString projectName() const {return "poolsv3_app";}
    QString mainTitle() const {return QString("Pools_V3 (Qt5)");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void save();
    void load();
    void actStartUpdating(); //send many request by timer

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotAppSettingsChanged(QStringList);
    void slotEnableControls(bool); // выполняется когда либо начинается некий запрос либо заканчивается
    void slotVisibleActionsUpdate(int); // выполняется когда переключаются вкладки на табах

private:
    QString defiConfig() const;
    QString nodejsPath() const;
    bool updateWalletAtStart() const;

};



#endif //TEST_MAINFORM_H


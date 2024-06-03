#ifndef TEST_MAINFORM_H
#define TEST_MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class QSplitter;
class BB_CentralWidget;

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
    BB_CentralWidget    *m_centralWidget;

    QString projectName() const {return "bybit_app";}
    QString mainTitle() const {return QString("ByBit test!");}
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
    void slotActionsApdate(int);
    void slotSetShadowLimitSize(float&);

private:
    int expandLevel() const;
    quint16 pageUpdatingInterval() const;
    float shadowSize() const;

};



#endif //TEST_MAINFORM_H


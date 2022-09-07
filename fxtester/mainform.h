#ifndef FX_MAINFORM_H
#define FX_MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class QSplitter;
class FXCentralWidget;
class FXDataLoader;
class FXBarContainer;
struct FXCoupleDataParams;



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}

protected:
    FXCentralWidget     *m_centralWidget;
    LProtocolBox        *m_protocol;
    QSplitter           *v_splitter;
    FXDataLoader        *m_dataLoader;


    QString projectName() const {return "fxtester";}
    QString mainTitle() const {return QString("FX tester (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();


    void save();
    void load();

    void reloadData();
    void initDataLoader();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
    void slotSetLoadedDataByReq(const QList<FXCoupleDataParams>&, QList<const FXBarContainer*>&);

private:
    QString dataDir() const;


};



#endif


#ifndef TEST_MAINFORM_H
#define TEST_MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class QSplitter;
class UG_CentralWidget;

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
    UG_CentralWidget    *m_centralWidget;

    QString projectName() const {return "unigraph_app";}
    QString mainTitle() const; // {return QString("Unigraph test!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void save();
    void load();

    void actStart(); //send free request (graph_ql query)
    void actStartUpdating(); //send many request by timer (graph_ql query)
    void actStop();
    void actSaveData();
    void actLoadData();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotAppSettingsChanged(QStringList);
    void slotEnableControls(bool);
    void slotVisibleActionsUpdate(int);
    void slotSetFilterParams(quint16&, double&);
    void slotSetSubGraph(QString);

private:
    int expandLevel() const;
    quint16 pageUpdatingInterval() const;
    QString apiKey() const;
    QString walletAddr() const;
    QString graphDomain() const;
    QString subgraphID() const;
    quint8 viewPrecision() const;
    double minTVL() const;
    quint16 reqSize() const;
    quint16 reqInterval() const;
    bool usePreferTokens() const;


};



#endif //TEST_MAINFORM_H


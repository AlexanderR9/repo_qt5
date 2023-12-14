#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

#include <QMap>

class LProtocolBox;
class QSplitter;
class QTabWidget;
class LSimpleWidget;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {qDeleteAll(m_pages); m_pages.clear();}
    
protected:
    LProtocolBox                *m_protocol;
    QSplitter                   *v_splitter;
    QTabWidget                  *m_tab;
    QMap<int, LSimpleWidget*>    m_pages;
    bool                         m_autoStartMakerFinished;

    QString projectName() const {return "rest_api_tester";}
    QString mainTitle() const {return QString("REST API tester (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void initPages();
    void sendAPIRequest();
    void loadData();
    void runAutoStart();
    void autoLoadDataFiles();
    void clear();
    LSimpleWidget* activePage() const;
    void enableActions(bool);
    bool autoStartModeNow() const;
    void updateOrdersList();
    void updateEventsList();
    void updateBagInfo();
    void getVisibleBondPrices();


protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotReqFinished(int);
    void slotAppSettingsChanged(QStringList);
    void slotGetReqParams(QString &s1, QString &s2) {s1 = token(); s2 = baseURI();}
    void slotGetPricesDepth(quint16 &dp) {dp = quint16(depth());}
    void slotGetCandleSize(QString &cp) {cp = candleSize();}
    void slotAutoStart();
    void slotDisableActions(bool disabled) {enableActions(!disabled);}


private:
    QString serverAPI() const;
    QString baseURI() const;
    QString token() const;
    QString bondsPeriod() const;
    int expandLevel() const;
    int depth() const;
    QString candleSize() const;
    bool printHeaders() const;
    bool autoStart() const;

};


#endif


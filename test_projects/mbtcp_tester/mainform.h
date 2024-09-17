#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"


class LProtocolBox;
class ComObj;
class QByteArray;
class QSplitter;
class MBTcpCentralWidget;
class MBTcpObj;


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
    MBTcpCentralWidget      *m_centralWidget;
    MBTcpObj                *m_mbtcpObj;

    QString projectName() const {return "mbtcp_tester";}
    QString mainTitle() const {return QString("Tester MB_TCP apps (Qt5)!");}

    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void startExchange();
    void stopExchange();
    void sendReq();
    void initMBObj();
    void updateToolbar(bool);

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotAppSettingsChanged(QStringList);

private:
    int mode() const; //0-master, 1-slave
    quint16 port() const;
    QString host() const;
    quint32 do_count() const;
    quint32 di_count() const;
    quint32 ao_count() const;
    quint32 ai_count() const;


};



#endif


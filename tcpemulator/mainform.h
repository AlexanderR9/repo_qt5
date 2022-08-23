#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"
#include "lsimpleobj.h"

#include <QDataStream>

class LProtocolBox;
class QSplitter;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class LTcpServerObj;
class LTcpClientObj;


struct w32_SYSTEMTIME
{
    w32_SYSTEMTIME() {reset();}

    qint16 wYear;
    qint16 wMonth;
    qint16 wDayOfWeek;
    qint16 wDay;
    qint16 wHour;
    qint16 wMinute;
    qint16 wSecond;
    qint16 wMilliseconds;

    void reset() {wYear=2022; wMonth=8; wDayOfWeek=1; wDay=22; wHour=15; wMinute=35; wSecond=wMilliseconds=0;}
};

struct PackHeader
{
    PackHeader() {reset();}

    char            subSys;
    char            dataType;
    quint64         len;
    //SYSTEMTIME    time;
    w32_SYSTEMTIME  time;
    quint16         roundCtr;

    void toByteArray(QByteArray *ba)
    {
        QDataStream stream(ba, QIODevice::WriteOnly);
        stream << subSys << len << time.wYear << time.wMonth << time.wDayOfWeek << time.wDay
               << time.wHour << time.wMinute << time.wSecond << time.wMilliseconds << roundCtr;

    }
    void reset() {subSys=dataType=1; len=0; time.reset(); roundCtr=0;}

};




// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    enum EmulatorMode {emServer = 0, emClient};

    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
    inline bool isClient() const {return (m_mode == emClient);}
    inline bool isServer() const {return (m_mode == emServer);}

protected:
    LProtocolBox        *m_protocol;
    LTcpServerObj       *m_server;
    LTcpClientObj       *m_client;
    int                 m_mode;
    QByteArray          ba_header;

    QString projectName() const {return "tcpemulator";}
    QString mainTitle() const {return QString("TCP emulator (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void initTcpObjects();
    void start();
    void stop();
    void sendPack();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotAppSettingsChanged(QStringList); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotServerPackReceived(const QByteArray&);
    void slotClientPackReceived(const QByteArray&);
    void slotTimer();

};




#endif


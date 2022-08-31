#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"
#include "lsimpleobj.h"

#include <QDateTime>

class LProtocolBox;
class QSplitter;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class LTcpServerObj;
class LTcpClientObj;
class LTCPStatusWidget;

struct w32_SYSTEMTIME;
struct w32_FILETIME;



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

    static QDateTime toQDateTime(const w32_SYSTEMTIME&, Qt::TimeSpec ts = Qt::UTC);
    static QDateTime toQDateTime(const w32_FILETIME&, Qt::TimeSpec ts = Qt::UTC);
    static void toSystemTime_w32(const QDateTime&, w32_SYSTEMTIME&);
    static void toFileTime_w32(const QDateTime&, w32_FILETIME&);

protected:
    LProtocolBox        *m_protocol;
    LTCPStatusWidget     *m_statusWidget;
    LTcpServerObj       *m_server;
    LTcpClientObj       *m_client;
    int                 m_mode;
    QByteArray          ba_header;
    quint16 m_counter;

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
    void updateStatusWidget();

    void prepareFloatPacket(QByteArray&);
    void prepareDiscretePacket(QByteArray&);

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotAppSettingsChanged(QStringList); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotServerPackReceived(const QByteArray&);
    void slotClientPackReceived(const QByteArray&);
    void slotTimer();

private:
    bool autoSendPack() const;
    int recsCount() const;
    int byteOrder() const;
    quint16 qualSig() const;

    void DTSTDataReady(const QByteArray &ba);

};




#endif


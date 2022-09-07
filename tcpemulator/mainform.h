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

struct w32_time;
struct w32_system_time;



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
    LTCPStatusWidget     *m_statusWidget;
    LTcpServerObj       *m_server;
    LTcpClientObj       *m_client;
    int                 m_mode;
    quint16             m_counter;

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
    void updateButtonsState();

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
    int floatRecsCount() const;
    int discreteRecsCount() const;
    int byteOrder() const;
    quint16 qualSig() const;

    void DTSTDataReady(const QByteArray &ba);

};




#endif


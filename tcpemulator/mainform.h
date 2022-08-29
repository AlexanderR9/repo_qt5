#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"
#include "lsimpleobj.h"

#include <QDataStream>
#include <QDateTime>

class LProtocolBox;
class QSplitter;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class LTcpServerObj;
class LTcpClientObj;

#define SUCCESS_QUALITY     0


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
    void toByteArray(QDataStream &stream)
    {
        stream << wYear << wMonth << wDayOfWeek << wDay << wHour << wMinute << wSecond << wMilliseconds;
    }
    void setCurrentTime()
    {
        QDateTime dt = QDateTime::currentDateTime();
        wYear = dt.date().year();
        wMonth = dt.date().month();
        wDayOfWeek = dt.date().dayOfWeek();
        wDay = dt.date().day();
        wHour = dt.time().hour();
        wMinute = dt.time().minute();
        wSecond = dt.time().second();
        wMilliseconds = dt.time().msec();
    }

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


struct PortalPackHeader
{
    PortalPackHeader() {reset();}

    qint8           subSys_id;
    qint8           dataType_id;
    qint32          len;
    w32_SYSTEMTIME  time;
    quint16         counter;

    void reset() {subSys_id=dataType_id=1; len=0; time.reset(); counter=0;}
    quint32 size() const {return 24;}
    quint32 size2() const {return sizeof(subSys_id) + sizeof(dataType_id) + sizeof(len) + sizeof(time) + sizeof(counter);}
    void toByteArray(QByteArray *ba)
    {
        ba->clear();
        QDataStream stream(ba, QIODevice::WriteOnly);
        stream << subSys_id << dataType_id << len;
        time.toByteArray(stream);
        stream << counter;
    }
    QString toStr() const
    {
        QString s("PortalPackHeader:");
        s = QString("%1  subSys_id=%2  dataType_id=%3  len=%4  counter=%5").arg(s).arg(subSys_id).arg(dataType_id).arg(len).arg(counter);
        return s;
    }
};
struct ARecord
{
    ARecord() :quality(SUCCESS_QUALITY), sig_value(-1) {}
    ARecord(float v) :quality(SUCCESS_QUALITY), sig_value(v) {}

    quint16     quality;
    float       sig_value;

    void toByteArray(QDataStream &stream) {stream << quality << sig_value;}
    quint32 size2() const {return sizeof(quality) + sizeof(sig_value);}
    static int size() {return 6;}

};
struct	DRecord
{
    DRecord() :quality(SUCCESS_QUALITY), sig_value(0) {}
    DRecord(quint8 v) :quality(SUCCESS_QUALITY), sig_value(v) {}

    quint16  quality;
    quint8   sig_value;

    void toByteArray(QDataStream &stream) {stream << quality << sig_value;}
    quint32 size2() const {return sizeof(quality) + sizeof(sig_value);}
    static int size() {return 3;}
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

};




#endif


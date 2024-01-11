#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"
#include "lsimpleobj.h"

#include <QDateTime>

class LProtocolBox;
class QSplitter;
class QUdpSocket;
struct w32_time;
struct w32_system_time;


//простая утилита для приема/передачи udp датаграмм
//в режиме сервера отправляет раз в заданный интервал тестовый пакет, который был считан из заданного файла,
// либо (если файл не зада) это 8 байт от 0 до 7.
// в режиме клиента принимает данные по указанному порту и выводит в протокол, таже может писать в файл если таковой задан.


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
    QTimer              *m_timer;
    QUdpSocket          *udp_socket;
    int                  m_mode;
    quint16              m_counter;
    QByteArray           m_testPacket;

    QString projectName() const {return "udpemulator";}
    QString mainTitle() const {return QString("UDP emulator (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void start();
    void stop();
    bool started() const;
    void sendPack();
    void updateStatusWidget();
    void updateButtonsState();
    void preparePacket(); //подготовить тестовый пользовательский пакет для отправки его по таймеру
    void writeOutFile(const QByteArray&); //записать принятый пакет в пользовательский файл


protected slots:
    void slotAction(int); //virtual slot from parent
    void slotAppSettingsChanged(QStringList); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotTimer();
    void slotReadyRead();

private:
    int byteOrder() const;
    int timerPeriod() const;
    QString host() const;
    quint16 port() const;
    QString packFile() const;
    QString outFile() const;



};




#endif


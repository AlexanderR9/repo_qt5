#ifndef MBSERVER_H
#define MBSERVER_H

#include "mbslaveserverbase.h"

#include <QDebug>

class QSerialPort;
class QModbusPdu;
struct ComParams;
class LMBAdu;
class MBDeviceEmulator;
class MBConfigLoader;


//MBServer
class MBServer : public LMBSlaveServerBase
{
    Q_OBJECT
public:
    MBServer(QObject *parent = NULL);
    virtual ~MBServer() {}

    QString cmdCounterToStr() const; //info
    QString regPosToStr() const; //info
    inline const MBConfigLoader* configLoader() const {return emul_config_loader;}
    void setEmulConfig(const QString&);

protected:
    void reset();
    void timerEvent(QTimerEvent*); //test
    QModbusResponse exeptionRequest(const QModbusPdu&) const; //обработать исключение запроса

    QMap<quint8, int>   m_cmdCounter; //счетчик команд
    int     m_maxReadingReg;
    int     m_maxWritingReg;
    int     m_invalidPass;

    void parseCurrentBuffer();
    void tryParseAdu(const LMBAdu&);
    bool open();

    //for emulation
    void initRegistersMap();
    void transformPDU(QModbusPdu&, quint8);

    MBConfigLoader *emul_config_loader;
    MBDeviceEmulator *emul_complex;
    void initEmulComplex(); //инициализировать таблицу устройств и сигналов для имитации системы

protected slots:
    void slotSetRegisterValue(int, quint16, quint16); //записать значение в регистр


};


#endif // MBSERVER_H



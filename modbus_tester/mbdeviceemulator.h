#ifndef MB_DEVICE_EMULATOR_H
#define MB_DEVICE_EMULATOR_H


#include <QDebug>

class QSerialPort;
class QModbusPdu;
struct ComParams;
class MBAdu;
class QTimer;


// EmulValueSettings
struct EmulValueSettings
{
    EmulValueSettings() :base_value(0), err(0), factor(1), adder(0) {}
    EmulValueSettings(double bv, double e, double f, double a = 0) :base_value(bv), err(e), factor(f), adder(a) {}

    double base_value;
    double err; //%

    double factor; //множитель для участия конвертации double в значение регистра quint16
    double adder; //добавочное значение для участия конвертации double в значение регистра quint16

    double nextValue() const; //для вещественных сигналов
    double nextBitValue() const; //для дискретных сигналов
    void disableSettings() {base_value = err = -1;}
    bool disabled() const {return ((base_value < 0) && (err < 0));}
    QString toStr() const;

};


// MBRegisterInfo
struct MBRegisterInfo
{
    enum MBEmulSignalType {estBit_1 = 1, estBit_16 = 16};

    MBRegisterInfo() :type(estBit_16), pos(0), bit_index(0) {}
    MBRegisterInfo(int t, quint16 p, quint8 i = 0) :type(t), pos(p), bit_index(i) {}
    MBRegisterInfo(const MBRegisterInfo &info) :type(info.type), pos(info.pos), bit_index(info.bit_index) {}

    int type;       //тип данных регистра
    quint16 pos;    //позиция регистра, которая должна быть в имитируемом устройстве
    quint8 bit_index; //индекс бита в 16-битном значении регистра (актуален только для типа estBit_1)

    QString toStr() const {return QString("RegisterInfo: type=%1 pos=%2 bit=%3").arg(type).arg(pos).arg(bit_index);}
};

// MBEmulSignal
struct MBEmulSignal
{
    MBEmulSignal() {reset();}
    MBEmulSignal(const MBRegisterInfo &info) :reg_info(info) {reset();}

    quint16 value;
    MBRegisterInfo reg_info;
    EmulValueSettings emul_settings;


    //уникальный
    //10-ми значное число
    //1-я цифра 1 или 2, означает тип сигнала estBit_1 или estBit_16 ??????
    //следующие 3 знака означают адрес устройства
    //следующие 4 знака означают позицию регистра, которая должна быть в имитируемом устройстве
    //следующие 2 знака означают номер бита в 16-битном регистре (для типа сигнала estBit_1)
    quint32 id;

    inline bool isBit() const {return (reg_info.type == MBRegisterInfo::estBit_1);}
    inline void reset() {value = id = 0; emul_settings.disableSettings();}
    inline quint32 baseID() const {return quint16(qRound(double(id)/double(100))*100);}

    void calcID(quint8 addr);
    void updateValue(double v, double factor, int adder = 0);
    void setEmuValueSettings(const EmulValueSettings&);
    bool hasEmulSettings() const {return !emul_settings.disabled();}

};

// MBEmulDevice
struct MBEmulDevice
{
    MBEmulDevice() {reset();}
    MBEmulDevice(quint8 addr) :address(addr), internal_index(0) {emul_signals.clear();}

    quint8 address;
    quint8 internal_index;
    QList<MBEmulSignal> emul_signals;
    EmulValueSettings emul_settings;

    inline int signalsCount() const {return emul_signals.count();}
    inline void reset() {address = 200; emul_signals.clear(); internal_index = 0;}
    static quint16 maxSignals() {return 100;} //максимально допустимое количество сигналов для 1-го устройства

    void addSignal(const MBRegisterInfo&);
    void updateSignal(int i, double v, double factor, int adder = 0); //обновить значение сигнала
    quint16 regPos(int i, quint16 def_value = 0) const; //реальная позиция регистра i-го сигнала для этого устройства
    QList<quint16> emulPosList() const; //все значения имитируемых регистров в этом устройстве (для всех сигналов этого устройства)
    void setEmuValueSettings(const EmulValueSettings&); //задать настроки эмуляции значений для всей стойки
    void setEmuValueSettings(int, const EmulValueSettings&); //задать настроки эмуляции значений для конкретного датчика стойки
    void updateSignals(QMap<quint16, quint16>&, bool); // need update registers map: pos, value

};


// MBDeviceEmulator
class MBDeviceEmulator : public QObject
{
    Q_OBJECT
public:
    MBDeviceEmulator(QObject *parent = NULL);
    virtual ~MBDeviceEmulator() {}

    void out();
    int allSignalsCount() const; //количество всех сигналов, всей системы
    int deviceSignalsCount(quint8) const; //количество всех сигналов заданного устройства

    void addDevice(quint8 address); //добавить устройство
    void addDeviceSignal(quint8, const MBRegisterInfo&); //добавить сигнал в устройство
    void processPDU(quint8, QByteArray&); //обработать данные запроса
    void startEmulation(); //активировать эмулятор
    void stopEmulation(); //остановить эмулятор
    bool activated() const;
    void setEmuValueSettings(quint8, const EmulValueSettings&); //установить настройки эмуляции для заданного устройства
    void setEmuValueSettings(quint8, const EmulValueSettings&, int); //установить настройки эмуляции для заданного устройства и заданного сигнала

    inline int deviceCount() const {return m_devices.count();}
    inline quint32 mbRegsCount() const {return (deviceCount()*MBEmulDevice::maxSignals() + 10);} //сколько всего регистров необходимо зарезервировать для имитации данной системы
    inline void setFirstValuesOn() {m_firstValues = true;}
    inline void reset() {m_devices.clear();}

protected:
    QMap<quint8, MBEmulDevice> m_devices;
    QTimer *m_timer;
    bool m_firstValues; //признак того что в этот момент значения задаются в 1-й раз после старта эмуляции.

    //const MBEmulDevice* deviceAt(quint8) const;

protected slots:
    void slotTimer(); //обновить некоторые значения имитируемых сигналов

signals:
    void signalSetRegisterValue(int, quint16, quint16);

};




#endif // MB_DEVICE_EMULATOR_H



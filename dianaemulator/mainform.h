#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"
#include <QMap>

class QSplitter;
class QTabWidget;
class LProtocolBox;
class QTimer;
class DianaViewWidget;
class MQGeneralPage;


// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    enum EmulatorMode {emServer = 0, emClient};

    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
    inline bool isClient() const {return (m_mode == emClient);} //работаем с дианами
    inline bool isServer() const {return (m_mode == emServer);} //имтируем дианы

protected:
    int                 m_mode;
    QSplitter           *v_splitter;
    QTabWidget          *m_tab;
    LProtocolBox        *m_protocol;
    QTimer              *m_exchangeTimer;
    QTimer              *m_stateTimer;


    MQGeneralPage   *m_generalPage;
    QMap<QString, DianaViewWidget*>     m_pages;

    QString projectName() const {return "dianaemulator";}
    QString mainTitle() const {return QString("DIANA emulator (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void initTimers(); //инициализация таймеров
    void loadMQConfig(); //загрузить все конфиги для диан
    void loadMQPacket(const QString&, const QString&); //загрузить указанный пакет для указанной дианы
    void save();
    void load();

    void start();
    void stop();
    void updateButtonsState();
    void tryAddPage(const QString&); //попытка создания новой страницы для указанной дианы, при условии что такой еще нет
    void checkMQLinuxDir(bool&); //проверка наличия примонтированного раздела /dev/mqueue в Linux
    void restartStateTimer();
    void restartExchangeTimer();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotAppSettingsChanged(QStringList); //virtual slot from parent
    void slotError(const QString&);
    void slotMsg(const QString&);
    void slotUpdateMQStateTimer();
    void slotMQExchangeTimer();



private:
    QString configDir() const;
    int byteOrder() const;
    bool autoUpdatePackets() const;
    int viewExpandLevel() const;
    quint8 doublePrecision() const;
    int modeSettings() const;
    void parseConfigName(const QString&, QPair<QString, QString>&); //проверка корректности имени файла-конфига
    int mqStateInterval();
    int mqExchangeInterval();

};




#endif


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
    QTimer              *m_exchangeTimer; //таймер для записи/чтении в/из очередей, включается/выключается пользователем кнопками старт/стоп
    QTimer              *m_stateTimer; //таймер для обновления информации о состоянии очередей, работает все время


    MQGeneralPage   *m_generalPage;
    QMap<QString, DianaViewWidget*>     m_pages;

    QString projectName() const {return "dianaemulator";}
    QString mainTitle() const {return QString("DIANA emulator (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void initModeLabel();

    void initTimers(); //инициализация таймеров
    void loadMQConfig(); //загрузить все конфиги для диан
    void loadMQPacket(const QString&, const QString&); //загрузить указанный пакет для указанной дианы
    void save();
    void load();

    //act funcs
    void start(); //запустить обмен
    void stop(); //остановить обмен
    void recreateAllQueues(); //пересоздать/создать все очереди MQ согласно конфигурации  (only emServer)
    void destroyAllQueues(); //удалить все очереди MQ согласно конфигурации (only emServer)
    void sendMsg(); //отправить одно сообщение очереди, вкладка которой открыта, и которая находится в режиме WRITE (это зависит от m_mode)


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
    void slotSetBytesLineSize(int&);

private:
    QString configDir() const;
    int byteOrder() const;
    int byteLineCount() const;
    bool autoUpdatePackets() const;
    bool autoReadMsg() const;
    int viewExpandLevel() const;
    quint8 doublePrecision() const;
    int modeSettings() const;
    void parseConfigName(const QString&, QPair<QString, QString>&); //проверка корректности имени файла-конфига
    int mqStateInterval();
    int mqExchangeInterval();

};




#endif


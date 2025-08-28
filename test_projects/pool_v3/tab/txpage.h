#ifndef TXTABPAGE_H
#define TXTABPAGE_H

#include "basetabpage_v3.h"

#include <QDateTime>

//class LTableWidgetBox;
class QJsonObject;
//class QTimer;
class DefiTxLogger;


//DefiTxTabPage
//страница дял отображения информации о транзакциях.
// объект DefiTxLogger хранит в себе записи транзакций, а так же загружает/выгружает их в файлы на диске.
class DefiTxTabPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiTxTabPage(QWidget*);
    virtual ~DefiTxTabPage() {}

    virtual void sendUpdateDataRequest() {};
    virtual void setChain(int);

    /*
    void loadTxFromFile(QString); //загрузить список hash из файла
    inline int txCount() const {return tx_data.count();}
    void parseJSResult(const QJsonObject&);
    void getAllWaitingStates(); //запросить состояние у всех записей, которые в режиме ожидания
*/
protected:
    DefiTxLogger    *m_logger; //managet tx records

    void initTable();
    void reloadTables();

    /*
    LTableWidgetBox     *m_table;
    QList<JSTxRecord>    tx_data;
    QTimer              *m_checkStateTimer; //таймер для опроса всех записей, которые в режиме ожидания
    bool                 js_running;


    //информация загруженная из локального файла, содержит информацию только о завершенных транзакциях.
    //вид строки: hash / fee_size / status (OK/FAULT)
    QStringList          m_locData;

    void initTable();
    void reloadTable();
    void applyLocalData();
    void loadLocalData(); //from local defi file
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    JSTxRecord* recordByHash(QString);
    void updateTableRowByRecord(const JSTxRecord*);
    void addRecToLocalFile(const JSTxRecord *rec); //после получения ответа от nodejs добавить информацию в локальный файл
    void selectRowByHash(const QString&);



public slots:
    void slotCheckTxResult(const QString&, bool&); //выполняется для проверки успешности выполнения транзакции
*/
public slots:
    virtual void slotNodejsReply(const QJsonObject&) {}; //получен успешный ответ от скрипта nodejs

/*
protected slots:
    void slotTxStatus();
    void slotCheckStateTimer();

signals:
    void signalCheckTx(const QStringList&);
    void signalEnableControls(bool);
    */



};

#endif // TXTABPAGE_H

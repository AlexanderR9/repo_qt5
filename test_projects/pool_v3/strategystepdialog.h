#ifndef STRATEGYSTEPDIALOG_H
#define STRATEGYSTEPDIALOG_H


#include "lsimpleobj.h"
#include "ui_strategystepdialog.h"

#include <QDialog>
#include <QDateTime>


class QTimer;
class NodejsBridge;
class QJsonObject;



// интерфейс для взаимодействия основного виджета с диалогом StrategyStepDialog
// хранит промежуточные данные по линии, а так же после выполнения действия в нее записываются новые.
struct StrategyStepDialogData
{
    StrategyStepDialogData() {reset();}

    //настройки линии
    QString pool_addr;
    QPair<float, float> line_liq; // текущая ликвидность линии
    float price_width; // ширина диапазона
    int prior_asset_size; // доля приоритетного токена (%) от общей вносимой ликвидности
    quint8 next_step; // номер шага к которому переходим на этом этапе
    quint8 first_token_index; // индекс токена из пары пула, который будет использоваться для внесения ликвидности на 1-м шаге

    QPair<QString, QString> pool_tickers; // определяется по pool_addr при старте
    QPair<QString, QString> pool_token_addrs; // определяется по pool_addr при старте

    //данные полученные из сети в процессе выполнения сценария
    QPair<float, float> wallet_assets_balance; // текущие балансы пары токенов пула в кошельке


    void reset();
    bool invalid() const;
    void out();

};




//StrategyStageManagerObj
class StrategyStageManagerObj : public LSimpleObject
{
    Q_OBJECT
public:
    struct SM_StageState
    {
        SM_StageState() {reset();}
        QDateTime   ts_start; // время для отслеживания таймаута
        QString     result; // динамическая переменная, результат выполнения каждой стадии OK/FAULT
        QString     note; // динамическая вспомогательная переменная, доп инфа после выполнения стадии

        void reset() {ts_start = QDateTime(); result.clear(); note.clear();}
    };

    StrategyStageManagerObj(StrategyStepDialogData&, QObject*);
    virtual ~StrategyStageManagerObj() {}

    void run(); // запуск выполнения сценария
    void stop(); // принудительный останов сценария
    void execCurrentStage(); // выполнить текущую стадию

    static QString captionByStage(int); // надпись в таблице указанной стадии
    static QString kindByStage(int); // надпись в таблице типа операции для указанной стадии

    void fillStagesList(int); // инициализировать список стадий по указанному типу действия
    inline int stagesCount() const {return m_actionStages.count();}
    inline int currentStage() const {return m_userSign;}
    inline QString resultNote() const {return m_stageState.note;}

protected:
    QList<int> m_actionStages; // динамический список стадий которые надо пройти при выполнении текущего действия, причем стадии могут повторятся
    QTimer      *m_timer; // таймер для отслеживания состояния стадий
    SM_StageState m_stageState; // структура для отслеживания состояния текущей стадии
    StrategyStepDialogData &m_data;

    bool isRunning() const;
    void checkTimeout();
    void checkStageState();
    void jsScriptRun();
    bool needToEthersReq() const; // признак что для текущей стадии нужен запрос к ethers_js (read/write)
    void setFaultResult(QString note = "none");

    // check ethers_js reply
    void readWalletAssetsBalance(const QJsonObject&);


private:
    void getWalletTokenAmounts();

protected slots:
    void slotTimerTick();

public slots:
    void slotNodejsReply(const QJsonObject&);
    void slotJSScriptFinished(int);

signals:
    void signalStartStage(int);
    void signalFinishedAll();
    void signalStageFinished(QString);
    void signalRunScriptArgs(const QStringList&);
    void signalRewriteJsonFile(const QJsonObject&, QString);

};





// StrategyStepDialog
class StrategyStepDialog : public QDialog, public Ui::StrategyStepDialog
{
    Q_OBJECT
public:
    StrategyStepDialog(int, StrategyStepDialogData&, QWidget *parent = 0);
    virtual ~StrategyStepDialog();

protected:
    int m_actionType; // element of StrategyStepAction
    StrategyStepDialogData &m_data;
    int m_stage;
    NodejsBridge *js_obj;
    StrategyStageManagerObj *m_stageManager;

    void initTables();
    void initActionEdit();
    void initJsObj();
    void fillParamsTable();
    void startScenario();
    void definePoolAssets(); // заполнить m_data.pool_tickers и  m_data.pool_token_addrs, выпоняется при старте

protected slots:
    void slotStartStage(int);
    void slotFinishedAll();
    void slotStageFinished(QString);

signals:
    void signalRewriteJsonFile(const QJsonObject&, QString);


};



#endif // STRATEGYSTEPDIALOG_H




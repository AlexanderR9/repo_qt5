#ifndef STRATEGY_STAGE_MANAGER_H
#define STRATEGY_STAGE_MANAGER_H


#include "lsimpleobj.h"

#include <QDateTime>


class QTimer;
class QJsonObject;
struct StrategyStepDialogData;



//StrategyStageManagerObj
// объект который выполняет стадии сценария и отслеживает их состояния.
// в процессе выполнения запрашивает необходимые данные из сети, а так же производит расчеты и
// все это записывает в промежуточную структуру m_data.
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
    quint16 txDelay() const; // задержка после отправленной транзакции
    void continueAfterDelay(); // задержка после отправленной транзакции закончилась, необходимо продолжить сценарий

    // check ethers_js reply
    void readWalletAssetsBalanceReply(const QJsonObject&);
    void readPoolStateReply(const QJsonObject&);
    void readSwapTxReply(const QJsonObject&);
    void readSwapTxStatusReply(const QJsonObject&);
    void readWalletAssetsBalanceAfterSwapReply(const QJsonObject&);


private:
    void getWalletTokenAmounts();
    void getPoolState();
    void readLineSettings();
    void calcSwapParts();
    void checkBalancesBeforeSwap();
    void makeSwap_TX();
    void checkTxSwapStatus();
    void getNextPosRange();


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
    void signalAddGasPriceField(QJsonObject&);

    // имитится когда была совершена транзакция, т.е. получена реальная hash (но еще неизвестен результат выполнения)
    // сигнал передается в StrategyTxLogMaker для формирования соответствующего лога и помещения его в файлы логов TX.
    void signalTxWasDone(const QJsonObject&);
    void signalTxStatusDone(const QJsonObject&); // был получен ответ при запросе статуса TX

    void signalDelayAfterTx(int);

};




#endif // STRATEGY_STAGE_MANAGER_H




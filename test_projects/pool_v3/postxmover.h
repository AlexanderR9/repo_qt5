#ifndef POSTXMOVER_H
#define POSTXMOVER_H

#include "postxworker.h"


class QTimer;
class TxMovePositionDialog;
class NodejsBridge;


//класс для выполнения сценария из последовательности действий для перемещения открытой позиции,
//которая вышла за диапазон.

// сценарий:
// 1. проверить состояние текущее позы, на предмет что оно обновлено,
//      а так же поза открыта и находится вне диапазона. запомнить адрес пула.
//      запомнить размер диапазона в тиках.
// 2. т.к. поза вне диапазона значит ее ликвидность полностью перетекла в один из токенов,
//      запомнить количество этого токена(оно пойдет в новую позу) и его индекс в паре пула,
//      так мы будем знать с какой стороны от текущего тика чеканить новый диапазон.
// 3. выполнить транзакцию типа take_away, затем выждать паузу
// 4. запросить в сети состояние выполненой транзакции take_away, проверить на успешность.
// 5. запросить текущее состояние пула, запомнить текущий тик.
// 6. рассчитать новый диапазон с нужной стороны от текущего тика, размер диапазона по умолчанию такой же,
//      но можно указать коэф. увеличения в диалоговом окне, так одна крайняя точка будет рядом с текущим тиком а другая растянется.
//      рассчет должен учитывать tickspacing, начальный тик должен быть не ближайший к текущему
//      а через tickspacing (т.е. ближайший по tickspacing + еще 1 интервал).
//      ИСКЛЮЧЕНИЕ: для пула из стейблов, для этого случая берем ближайший.
// 7. чеканим новую позу, выполнить транзакцию типа mint, затем выждать паузу
// 8. запросить в сети состояние выполненой транзакции mint, проверить на успешность.
// 9. запросить в сети текущий список поз обновить соответственно страницу "Wallet positions"


// PosTxMover
class PosTxMover : public PosTxWorkerBase
{
    Q_OBJECT
public:
    enum MoveStages {msFixAmount = 101, msTakeAway, msTakeAway_res, msPoolState, msCalcNewRange,
                     msMint, msMint_res, msPosList, msStoped};

    struct MoveParams
    {
        MoveParams() :token_index(0), amount(-1), pool_tick(0), pool_fee(0), tick_spacing(0) {reset();}

        quint8 token_index; // 0 or 1, token of pair pool
        float amount; // move position amount of token
        int pool_tick; // current tick of pool

        QString pool_addr;
        QString token0_address;
        QString token1_address;
        int pool_fee;
        quint16 tick_spacing;

        QPair<int, int> tick_range;

        int pid;
        QString liq;
        QString last_hash; // saved last TX hash

        QPair<float, float> taken_asset_amounts;
        QPair<float, float> taken_rewards;


        void reset() {tick_range.first = tick_range.second = -1; pid = 0; liq.clear();
                     taken_asset_amounts.first = taken_asset_amounts.second = 0;
                               taken_rewards.first = taken_rewards.second = 0;
                     }

    };


    PosTxMover(QObject*, int, QTableWidget*);
    virtual ~PosTxMover() {}

    void tryMove(const DefiPosition&);
    bool isRunning() const;
    void poolStateReceived(const QJsonObject&);


protected:
    QTimer *m_timer;
    qint64 m_tsTimeout; // для отслеживания таймаута выполнения любой стадии
    qint64 m_tsDelay; // для выполнения задержки после каждой стадии
    int m_stage; // текущая стадия
    //bool m_runningStage; // признак что стадия выполняется

    MoveParams m_moveParams;
    TxMovePositionDialog *m_moveDialog;
    NodejsBridge *js_obj;


    //int rowTableByPid(int) const;
    void nextStage(); // перейти к следующей стадии
    void breakMoving(const QString&); // произошла ошибка, прервать выполнения сценария
    void runStage(); // выполнить текущую стадию
    void finishMoving(); // сценарий выпонился успешно, завершить выполнение
    QString gasField() const;

    // была отправлена реальная транзакция, теперь необходимо сформировать соответствующую запись и отправить в журнал
    void prepareTakeawayTxLog();
    void prepareMintTxLog(const QJsonObject&);

    // был получен ответ на запрос статуса последней выполненной транзакции, необходимо эту инфу перекинуть на страницу TxPage
    void txStatusDone(const QJsonObject &js_reply);


    inline bool isDelaing() const {return (m_tsDelay > 0);}
    inline bool isRunningStage() const {return (m_tsTimeout > 0);}

private:
    void jsScriptRun();
    void calcNextRange();
    void takeAway();
    void lastTxStatus_res();
    void nextMint();


protected slots:
    void slotStageTimer();
    void slotStartMove();
    void slotNodejsReply(const QJsonObject&);
    void slotJSScriptFinished(int);

signals:
    void signalGetPoolState(const QString&);
    void signalRewriteJsonFile(const QJsonObject&, QString);
    void signalNewTx(const TxLogRecord&); // отправляется странице DefiTxTabPage после успешного получения в ответе tx_hash

    // имитится когда была совершена транзакция, т.е. получена реальная hash (но еще неизвестен результат выполнения)
    // сигнал передается в StrategyTxLogMaker для формирования соответствующего лога и помещения его в файлы логов TX.
    void signalStrategyTxStatus(const QMap<QString, QString>&); // отправляется после успешного получения статуса TX

};



#endif // POSTXWORKER_H

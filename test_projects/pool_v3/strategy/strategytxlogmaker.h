#ifndef STRATEGY_TX_LOG_MAKER_H
#define STRATEGY_TX_LOG_MAKER_H


#include "lsimpleobj.h"

class QJsonObject;
struct StrategyStepDialogData;
struct TxLogRecord;



//StrategyTxLogMaker
// объект для формарирования логов транзакций совершенных при выполнении сценария шага
class StrategyTxLogMaker : public LSimpleObject
{
    Q_OBJECT
public:
    struct SM_TxInfo
    {
        SM_TxInfo() {reset();}

        QString req;
        QString hash;
        float gas_fee; // уплаченная комиссия в нативной монете, -1 значит пока неизвестно
        QString status; // результат выполнения ok/fault, пустая строка значит пока неизвестно

        void reset() {req.clear(); hash.clear(); status.clear(); gas_fee=-1;}
        bool isChecked() const {return (!status.isEmpty());} // проверен статус
    };

    StrategyTxLogMaker(StrategyStepDialogData&, QObject*);
    virtual ~StrategyTxLogMaker() {}

protected:
    StrategyStepDialogData &m_data;
    QList<SM_TxInfo> m_txList; // инфа о транзакциях(реальных) выполненных в процессе выполнения сценария

    void makeSwapLog(const QJsonObject&);

protected slots:
    // выполняется когда была совершена транзакция, т.е. получена реальная hash (но еще неизвестен результат выполнения)
    // сигнал передается от StrategyStageManagerObj для формирования соответствующего лога и помещения его в файлы логов TX.
    void slotTxWasDone(const QJsonObject&);
    void slotTxStatusDone(const QJsonObject&);


signals:
    void signalStrategyTx(const TxLogRecord&); // отправляется после успешного получения в ответе tx_hash
    void signalStrategyTxStatus(const QMap<QString, QString>&); // отправляется после успешного получения статуса TX

};




#endif // STRATEGY_TX_LOG_MAKER_H




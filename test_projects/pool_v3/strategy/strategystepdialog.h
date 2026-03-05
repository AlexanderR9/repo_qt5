#ifndef STRATEGYSTEPDIALOG_H
#define STRATEGYSTEPDIALOG_H


//#include "lsimpleobj.h"
#include "ui_strategystepdialog.h"

#include <QDialog>
//#include <QDateTime>


//class QTimer;
class NodejsBridge;
class QJsonObject;
struct StrategyStepDialogData;
class StrategyStageManagerObj;
class StrategyTxLogMaker;
struct TxLogRecord;


// StrategyStepDialog
// диалоговое окно, которое всплывает при старте совершения сценария выполнения шага.
// отображает в своем интерфейсе состояние стадий и текущую стадию.
class StrategyStepDialog : public QDialog, public Ui::StrategyStepDialog
{
    Q_OBJECT
public:
    StrategyStepDialog(int, StrategyStepDialogData&, QWidget *parent = 0);
    virtual ~StrategyStepDialog();

protected:
    int m_actionType; // element of StrategyStepAction
    StrategyStepDialogData      &m_data;
    int m_stage;
    NodejsBridge                *js_obj;
    StrategyStageManagerObj     *m_stageManager;
    StrategyTxLogMaker          *m_txLogMaker;

    void initTables();
    void initActionEdit();
    void initJsObj();
    void initManagerObj();
    void fillParamsTable();
    void startScenario();
    void definePoolAssets(); // заполнить m_data.pool_tickers и  m_data.pool_token_addrs, выпоняется при старте
    void updateTableAfterStage();

private:
    void updateLineAmountsCell(int);
    void updateSwapInfoCell();

protected slots:
    void slotStartStage(int);
    void slotFinishedAll();
    void slotStageFinished(QString);
    void slotDelayAfterTx(int);


signals:
    void signalRewriteJsonFile(const QJsonObject&, QString);
    void signalAddGasPriceField(QJsonObject&);
    void signalStrategyTx(const TxLogRecord&);
    void signalStrategyTxStatus(const QMap<QString, QString>&);


};



#endif // STRATEGYSTEPDIALOG_H




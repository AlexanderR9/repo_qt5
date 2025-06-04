#ifndef JSPOSHISTORYTAB_H
#define JSPOSHISTORYTAB_H


#include "ug_basepage.h"

#include <QPair>;

//class QJsonArray;
//class QJsonObject;
//struct TxDialogData;
struct JSTxLogRecord;
class LSearchTableWidgetBox;
class JSTxLogger;

//PosLineStep
struct PosLineStep
{
    //состояние позиции в начале/конце
    struct PointState
    {
        PointState() {reset();}
        QDateTime dt;
        float current_price;

        //столько токено либо внесли, либо вывели без учета собранных комиссиий
        float token0_size;
        float token1_size;

        void reset() {current_price=-1; token0_size=token1_size=0; dt=QDateTime();}
        QString strTime() const {return (dt.isValid() ? dt.toString("dd.MM.yyyy (hh:mm)") : "---");}
    };
    //------------------------------
    PosLineStep() {reset();}

    QString tx_kind;
    QPair<float, float> pos_range;
    QPair<float, float> claimed_rewards; //хранит только собранные комиссии, основные внесенные токены сюда не входят
    QString pool_info; //short pool info
    PointState start_state;
    PointState exit_state;

    void reset();

    //алгоритм расчета APR:
    float calcAPR() const;

    //string interface funcs
    QString strPriceRange() const;
    QString strCurrentPrices() const;
    QString strDeposited() const;
    QString strDecreased() const;
    QString strClaimedFees() const;

};

//JSPosHistoryTab
//отображает историю операций с позициями
class JSPosHistoryTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSPosHistoryTab(QWidget*);
    virtual ~JSPosHistoryTab() {}

    void loadTxLogFile(QString);

protected:
    LSearchTableWidgetBox     *m_table;
    JSTxLogger                *m_txHistoryObj;

    QList<PosLineStep>        m_stepData;

    void initTable();
    void reloadTableData();
    void convertLogDataToStepData();
    void addNewRecord(const JSTxLogRecord&);

signals:
    void signalCheckTxResult(const QString&, bool&);
    void signalGetPoolInfo(const QString&, QString&); //запросить краткую информацию о пуле по ее адресу


};



#endif // JSPOSHISTORYTAB_H




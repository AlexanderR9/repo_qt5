#ifndef JSPOSHISTORYTAB_H
#define JSPOSHISTORYTAB_H


#include "ug_basepage.h"

#include <QPair>;

//class QJsonArray;
//class QJsonObject;
//struct TxDialogData;
struct JSTxLogRecord;
class JSPosTableWidgetBox;
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
    QString pool_address;
    QString tick_range;
    PointState start_state;
    PointState exit_state;
    bool wait_collect; // после decrease этот флаг необходимо установить в true  чтобы потом найти эту запись для внесения инфы о claimed_rewards

    void reset();
    bool isClosed() const;
    float lifeTime() const; //days
    bool invalid() const;


    //считает общий объем позиции при внесении ликвидности по текущей цене на момент открытия позы.
    //объем считается в одном из токенов пары пула. если в паре есть стейбл, то расчет производится в нем.
    //иначе в той монете, которая дешевле.
    //если пара состоит из обоих стейблов, то 1-м токене из них.
    float startAssetsSize() const;
    float decreasedAssetsSize() const; //если поза еще в работе (ликвидность еще не выведена), то вернет 0
    float rewardsAssetsSize() const; // только rewards
    float collectedAssetsSize() const; //если поза еще в работе (ликвидность еще не выведена), то вернет 0, сюда же добавляется rewards;
    int totalSizeTokenIndex() const; //индекс токена из пары (0/1) в котором считается общий объем
    QString totalSizeTokenName() const; //название токена из пары  в котором считается общий объем

    //алгоритм расчета APR: считается в однои из токене из пары, токен выбирается также как для расчета startAssetsSize().
    //для расчета APR берется сумарное значение rewards и сумарное стартовое значение внесенной ликвидности.
    //далее берется временная продолжительность позиции и по этим трем значение вычисляется годовой APR (%).
    //в итоге вернет годовую доходность в %
    float calcAPR() const;

    //пул в котором оба токена не стейблы и так же не ETH (example: WPOL/LDO)
    bool customPool() const;

    //ситуация когда была добавлена/создана ликвидность (в одном токене) в позу, которая не была в диапазоне.
    //после длительного времени поза так и не вошла в диапазон и ничего не заработала.
    //в итоге все те же токены были выведены (поза закрыта).
    bool zeroResult() const;

    //string interface funcs
    QString strPriceRange() const;
    QString strCurrentPrices() const;
    QString strDeposited() const;
    QString strDecreased() const;
    QString strClaimedFees() const;
    QString strLasting() const; //продолжительность

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
    JSPosTableWidgetBox     *m_table;
    JSTxLogger                *m_txHistoryObj;

    QList<PosLineStep>        m_stepData;

    void initTable();
    void reloadTableData();
    void convertLogDataToStepData();
    void addNewRecord(const JSTxLogRecord&);
    void decreaseRecord(const JSTxLogRecord&);
    void collectRecord(const JSTxLogRecord&);

public slots:
    void slotPosStateReceived(QString, QString);


signals:
    void signalCheckTxResult(const QString&, bool&);
    void signalGetPoolInfo(const QString&, QString&); //запросить краткую информацию о пуле по ее адресу


};



#endif // JSPOSHISTORYTAB_H




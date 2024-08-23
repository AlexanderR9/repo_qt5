#ifndef CALC_V3_H
#define CALC_V3_H
 
#include "lsimpleobj.h"
#include "poolstructs.h"

#include <QString>

struct mq_attr;
class QByteArray;

 
//класс для расчетов параметров пулов ликвидности,
//при помощи которого можно определить по входным параметрам объемы требуемых токенов и др. параметры.
//а так же имитацию перетикания из одного в другой при изменении цены на указанную дельту.
class LPoolCalcObj : public LSimpleObject
{
    Q_OBJECT
public:
    LPoolCalcObj(QObject *parent = NULL);
    virtual ~LPoolCalcObj() {}

    inline void setPricePrecision(quint8 p) {m_pricePrecision = p;}
    inline quint8 pricePrecision() const {return m_pricePrecision;}


    QString name() const {return QString("PoolCalculation_v3");}   
    void recalc(); //произвести расчет выходных параметров
    void reset(); //сброс всех параметров входных/расчетных

    //set params for calculation
    void setPoolTokens(QString, QString, int fee_type = pfs005); //установить токены и размер коммисии выбираемого пула
    void setPricesRange(double, double); //установить ценовой диапазов в котором будет работать ликвидность
    void setCurPrice(double); //установить текущую цену token0
    void setTokenSize(double, quint8 t_index=0); //установить вносимый объем одного из токенов, и индекс токена (0/1), если текущая цена вне диапазона, то t_index определится автоматом

    //get results params
    void getGuiParams(GuiPoolResults&);
    double assetsSum(const double&) const;


    //static
    static double basePips() {return 1.0001;} //базовый квант изменения цены
    static bool invalidFeeType(int); //вернет true если входной параметр не принадлежит множеству PoolFeeSize
    static quint16 binSize(int); //размер одной корзины в минимальных ценовых тиках при указанной коммисии пула
    static QString captionByFeeType(int fee_type); //строковое обозначение указанной коммисии пула
    static qint64 tickIndexByPrice(double &price); //индекс тика v3, цена которого ближайшая (или равна) снизу для указанной цены
    static qint64 tickBinByTickIndex(qint64, int fee_type = pfs005); //индекс тика корзины v3 соответствующей fee_type, ищется ближайщий (или равный) снизу по указанному тику
    static double tickPrice(qint64); //вернет цену соответствующую i-му тику  v3

protected:
    QString  	m_token0; //базовый токен
    QString  	m_token1; //стабильный токен
    quint8      m_pricePrecision; //точность цен
    
    InputPoolParams in_params; //входные параметры позиции при добавлении ликвидности
    PoolParamsCalculated out_params; //расчетные параметры

    void updateInputParamsValidity(); //обновить признак достоверности in_params
    void calcLiquiditySize(); //рассчитать значение ликвидности по входным параметрам
    void calcTokenSizes(); //рассчитать объемы токенов, подразумевается, что ликвидность уже посчитана
    bool invalidState() const; //признак того что входные параметры некорректны

    //цены диапазона сделать в соответствии с разбиением тиков v3.
    //нижняя цена трансформируется в цену ближаешего тика снизу.
    //верхняя цена трансформируется в цену ближаешего тика снизу, при этом который должен быть кратен размеру корзины для данного пула.
    void nomalizeRangeByBinSize();

public slots:
    //симуляция, текущая цена изменилась и нужно пересчитать объемы токенов.
    //пересчет идет всегда относительно стартового состояния.
    void slotSimPriceChanged(float);

signals:
    void signalSimChangePriceResult(float, float); //имитится после пересчета именениий объемов токенов при изменении цены

};


#endif //CALC_V3_H
 

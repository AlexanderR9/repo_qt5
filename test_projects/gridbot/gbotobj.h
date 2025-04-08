#ifndef GBOT_OBJECT_H
#define GBOT_OBJECT_H

#include "lsimpleobj.h"


struct GBotParams
{
    enum TradeDirection {tdShort = 0, tdLong};
    GBotParams() {reset();}

    float       in_sum;
    quint16     leverage;
    quint16     grid_count;
    float       start_price;
    int         direction;
    QPair<float, float> range; //p1 - p2

    void reset()
    {
        in_sum = start_price = 0;
        leverage = grid_count = 0;
        direction = -1;
        range.first = range.second = -1;
    }
    bool invalid() const {return (in_sum <= 0);}

};

//GBotObj
class GBotObj : public LSimpleObject
{
    Q_OBJECT
public:
    struct CalcData //промежуточные расчетные параметры
    {
        CalcData() {reset();}
        float grid_step;
        float lot_step;
        float lot_start; //сколько актива куплено/продано на старте
        QList<float> grid_values;
        void reset() {grid_values.clear(); grid_step=lot_step=lot_start=0;}
    };

    GBotObj(QObject *parent = NULL);
    virtual ~GBotObj() {}

    virtual QString name() const {return QString("gbot_obj");}
    void setParams(const GBotParams&);
    void resetParams();
    void exec();

    inline bool isShort() const {return (m_params.direction == GBotParams::tdShort);}
    inline bool isLong() const {return (m_params.direction == GBotParams::tdLong);}

protected:
     GBotParams m_params;
     CalcData m_calcData;

     void calcGridValues(); //найти все значения шагов-сеток
     void calcTableLines(); //найти табличный результат
    float stepProfit(bool) const; //абсолютная величина профита на 1-м шаге.

private:
     int upSteps() const;
     int downSteps() const;     

signals:
     void signalTableResult(const QStringList&);

};



#endif // GBOT_OBJECT_H

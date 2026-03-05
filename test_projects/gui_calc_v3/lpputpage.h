#ifndef LPPUTPAGE_H
#define LPPUTPAGE_H


#include <QWidget>
#include "ui_lpputpage.h"

#include <QPair>

class QSettings;

// LpPutPage
class LpPutPage : public QWidget, public Ui::LpPutPage
{
public:
    struct CalcParams
    {
        CalcParams() {reset();}

        // LP
        quint8 steps;
        float P0;
        float r1;
        float e0;
        float yield1;
        float yield_stable;

        // PUT
        float put_price;
        float put_strike;
        float put_lots;
        quint8 expiration_month;
        quint8 expiration_points;
        float point_step;

        void reset();
        bool invalid() const;
        QString toStr() const;

    };
    struct StepState
    {
        quint8 number;
        float nested_stables;
        float nested_persent;
        QPair<float, float> range;
        QPair<float, float> deposited;
        float left_point_asset;
        float loss_stable;
        float loss_persent;
        float avrP;
        float range_yield;
        float integrated_yield;


        float rangeSize() const {return (range.second - range.first);}
        //float averageP() const {return (range.second + range.first)/2;}
        float buyLeftP() const {return ((avrP + range.first)/2);} // средняя цена покупка актива в левой части диапазона если цена пойдет вниз

    };
    struct ExpirationPointResult
    {
        float point_price;
        //float put_cost;
        float put_result; // итоговый результат пута (с учетом его стоимости)
        float lp_reward; // сколько на момент экспирации принесла поза LP
        float lp_loss; // какой текущий убыток позы в точке point_price (без учета lp_reward)
        //float total_result;

        float totalReward() const {return (put_result + lp_reward);}
        float totalResult() const {return (put_result + lp_reward + lp_loss);}

    };
    struct TotalResult
    {
        TotalResult() {reset();}
        float lp_liq; // все средства выделенные на все LP шаги
        float put_cost; // средства потраченные на покупку пута (с учетом лотности)
        //float full_nested; // все задействованные средства в игре
        float put_cost_p; // средства потраченные на покупку пута, % от full_nested

        float average_yield_lp; // итоговая (усредненная) доходность LP поз. %
        float max_drawdown; // итоговая максимальная просадка (с учетом пута)
        float max_drawdown_p; // итоговая максимальная просадка (с учетом пута) %

        float result_lp; // итоговый результат(усредненый), полученный от LP поз за выбранные N мес
        float result_lp_p; // итоговый результат(усредненый), полученный от LP от всех вложенных средств, в том числе и на пут
        float low_lp_price; // самая низкая цена актива до которой она может опустится на последнем LP шаге

        float full_result; // итоговый результат за N мес, с учетом всего (вычисляется как среднее между 3-мя точками с наихудшими результатами)
        float full_result_p; // итоговый результат за N мес, % от full_nested

        void reset();
        float lpYearYield() const {return (lp_liq*average_yield_lp/float(100));} // предполагаемая прибыль за год от LP(абсолютная величина)
        float fullNested() const {return (lp_liq+put_cost);} // все задействованные средства в игре

    };

    LpPutPage(QWidget *parent = 0);
    virtual ~LpPutPage() {}

    void calc();

    void save(QSettings&);
    void load(QSettings&);

protected:
    CalcParams m_params;
    QList<StepState> step_data;
    QList<ExpirationPointResult> put_data;
    TotalResult m_result;
    //float start_liq;
    //float average_yield;


    void initTable();
    void checkParams(QString&);
    void updateTable();
    void updatePutTable();
    void updateTotalTable();
    void calcLP();
    void calcPut();
    float lpLossByPoint(float point_price) const;
    void calcTotal();
    void calcFullResult();

private:
    QString floatToStr(float) const;

};




#endif // LPPUTPAGE_H

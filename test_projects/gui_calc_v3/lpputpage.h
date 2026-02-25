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
        float buyLeftP() const {return ((avrP + range.first)/2);}
    };
    struct ExpirationPointResult
    {
        float point_price;
        float put_cost;
        float put_result; // итоговый результат пута (с учетом его стоимости)
        float lp_reward; // сколько на момент экспирации принесла поза LP
        float lp_loss; // какой текущий убыток позы в точке point_price (без учета lp_reward)
        //float total_result;

        float totalReward() const {return (put_result + lp_reward);}
        float totalResult() const {return (put_result + lp_reward + lp_loss);}

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
    float start_liq;
    float average_yield;


    void initTable();
    void checkParams(QString&);
    void updateTable();
    void updatePutTable();
    void calcLP();
    void calcPut();
    float lpLossByPoint(float point_price) const;

private:
    QString floatToStr(float) const;

};




#endif // LPPUTPAGE_H

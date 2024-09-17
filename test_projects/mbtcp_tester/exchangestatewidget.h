#ifndef EXCHANGESTATEWIDGET_H
#define EXCHANGESTATEWIDGET_H

#include "lsimplewidget.h"


// ExchangeStateWidget
class ExchangeStateWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    ExchangeStateWidget(QWidget *parent = 0);
    virtual ~ExchangeStateWidget() {}

    void updateStatistic(const QStringList&); //обновление информации об обмене
    virtual QString iconPath() const {return QString(":/icons/images/ball_gray.svg");}

protected:
    LTableWidgetBox         *m_tableBox;    //отображение статистики обмена и текущего состояния

    void initWidget();
    QStringList stateTableHeaders() const;


};


#endif // EXCHANGESTATEWIDGET_H

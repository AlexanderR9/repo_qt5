#ifndef PARAMSWIDGET_H
#define PARAMSWIDGET_H


#include <QWidget>
#include "ui_poolparams.h"

class QSettings;
struct PoolParamsStruct;
struct PoolParamsCalculated;


//ParamsWidget
class ParamsWidget : public QWidget, public Ui::PoolParams
{
    Q_OBJECT
public:
    ParamsWidget(QWidget *parent = 0);
    virtual ~ParamsWidget() {}

    void getParams(PoolParamsStruct&);

    void save(QSettings&);
    void load(QSettings&);

protected:
    void init();
    void normalizateValues();
    void normalizateValue(QLineEdit*);


signals:
    void signalError(const QString&);
    void signalMsg(const QString&);


public slots:
    void slotCalcResult(const PoolParamsCalculated&);


};

#endif // PARAMSWIDGET_H

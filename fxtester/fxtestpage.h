#ifndef FX_TESTPAGE_H
#define FX_TESTPAGE_H


#include "lsimplewidget.h"


//страница-интерфес для проведения тестирования

// FXTestPage
class FXTestPage : public LSimpleWidget
{
    Q_OBJECT
public:
    FXTestPage(QWidget *parent = 0);
    virtual ~FXTestPage() {}

    virtual QString caption() const {return QString("Testing page");} //некая надпись соответствующая этому виджету
    virtual QString iconPath() const {return QString(":/icons/images/r_scale.svg");} //некая иконка соответствующая этому виджету


};



#endif //FX_TESTPAGE_H



#ifndef FX_TESTPAGE_H
#define FX_TESTPAGE_H


#include "lsimplewidget.h"


//страница-интерфес для проведения тестирования

class QComboBox;

// FXInputParamsWidget
class FXInputParamsWidget : public LSimpleWidget
{
public:
    FXInputParamsWidget(QWidget *parent = 0);
    virtual ~FXInputParamsWidget() {}

    virtual QString caption() const {return QString("input_widget");} //некая надпись соответствующая этому виджету

protected:
    LTableWidgetBox     *m_tableBox;
    QComboBox           *m_testTypeBox;
    QComboBox           *m_yearStartBox;
    QComboBox           *m_yearEndBox;

    void initWidgets();


};



// FXTestPage
class FXTestPage : public LSimpleWidget
{
    Q_OBJECT
public:
    FXTestPage(QWidget *parent = 0);
    virtual ~FXTestPage() {}

    virtual QString caption() const {return QString("Testing page");} //некая надпись соответствующая этому виджету
    virtual QString iconPath() const {return QString(":/icons/images/r_scale.svg");} //некая иконка соответствующая этому виджету

protected:
    FXInputParamsWidget     *m_inputWidget;
    LTableWidgetBox         *m_resultWidget;
    LTableWidgetBox         *m_historyWidget;

    void initWidgets();

};



#endif //FX_TESTPAGE_H



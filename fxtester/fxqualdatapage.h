#ifndef FX_QUALDATA_PAGE_H
#define FX_QUALDATA_PAGE_H


#include "lsimplewidget.h"

class FXBarContainer;


//страница-интерфес для проверки качества загруженных данных

// FXQualDataPage
class FXQualDataPage : public LSimpleWidget
{
    Q_OBJECT
public:
    FXQualDataPage(QWidget *parent = 0);
    virtual ~FXQualDataPage() {}

    virtual QString caption() const {return QString("Quality data");} //некая надпись соответствующая этому виджету
    virtual QString iconPath() const {return QString(":/icons/images/chain.svg");} //некая иконка соответствующая этому виджету

    void check(const FXBarContainer*);

protected:
    LTableWidgetBox     *m_tableBox;
    LListWidgetBox      *m_listBox;

    void initWidgets();
    void reset();

};



#endif //FX_QUALDATA_PAGE_H



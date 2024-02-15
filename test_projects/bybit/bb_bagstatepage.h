#ifndef BB_BAGSTATEPAGE_H
#define BB_BAGSTATEPAGE_H

#include "lsimplewidget.h"


//BB_BagStatePage
class BB_BagStatePage : public LSimpleWidget
{
    Q_OBJECT
public:
    BB_BagStatePage(QWidget*);
    virtual ~BB_BagStatePage() {}

    QString iconPath() const {return QString(":/icons/images/bag.svg");}
    QString caption() const {return QString("Bag state");}

    //void updateDataPage();

protected:
    LTableWidgetBox     *m_table;

    void init();


};



#endif // BB_BAGSTATEPAGE_H



#ifndef CFDPAGE_H
#define CFDPAGE_H

#include "basepage.h"

//CFDPage
class CFDPage : public BasePage
{
    Q_OBJECT
public:
    CFDPage(QWidget*);
    virtual ~CFDPage() {}

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("CFD stat");}

    void updatePage() {}

};


#endif //CFDPAGE_H



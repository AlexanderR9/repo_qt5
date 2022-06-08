#ifndef LOGPAGE_H
#define LOGPAGE_H

#include "basepage.h"


//LogPage
class LogPage : public BasePage
{
    Q_OBJECT
public:
    LogPage(QWidget*);
    virtual ~LogPage() {}

    QString iconPath() const {return QString(":/icons/images/log.png");}
    QString caption() const {return QString("Log");}

    void updatePage() {}
};


#endif //LOGPAGE_H



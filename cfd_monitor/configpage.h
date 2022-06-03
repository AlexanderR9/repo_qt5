#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "basepage.h"

#include "ui_configpage.h"


//ConfigPage
class ConfigPage : public BasePage, Ui::ConfigPage
{
    Q_OBJECT
public:
    ConfigPage(QWidget*);
    virtual ~ConfigPage() {}

    QString iconPath() const {return QString(":/icons/images/xml.png");}
    QString caption() const {return QString("Configuration");}

};


#endif //CONFIGPAGE_H



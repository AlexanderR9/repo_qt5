#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <QWidget>

class QSettings;

// BasePage
class BasePage : QWidget
{
public:
    enum PageType {ptCFDStat = 81, ptLog};

    BasePage(QWidget*);
    virtual ~BasePage() {}

    virtual QString iconPath() const {return QString();}
    virtual void save(QSettings&) {}
    virtual void load(QSettings&) {}


};


#endif //BASEPAGE_H



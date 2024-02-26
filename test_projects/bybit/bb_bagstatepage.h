#ifndef BB_BAGSTATEPAGE_H
#define BB_BAGSTATEPAGE_H

#include "bb_basepage.h"
#include "bb_apistruct.h"


//BB_BagStatePage
class BB_BagStatePage : public BB_BasePage
{
    Q_OBJECT
public:
    BB_BagStatePage(QWidget*);
    virtual ~BB_BagStatePage() {}

    QString iconPath() const {return QString(":/icons/images/bag.svg");}
    QString caption() const {return QString("Bag state");}

    void updateDataPage(bool force = false);

protected:
    LTableWidgetBox     *m_table;
    BB_BagState          m_state;

    void init();
    void updateTable();

signals:
    void signalGetPosState(BB_BagState&);

};



#endif // BB_BAGSTATEPAGE_H



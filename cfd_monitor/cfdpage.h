#ifndef CFDPAGE_H
#define CFDPAGE_H

#include "basepage.h"

class QTableWidget;

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

protected:
    QTableWidget    *m_table;

    QStringList headerLabels() const;
    void init();
    void removeRowByTicker(const QString&);

public slots:
    void slotNewPrice(const QStringList&);



};


#endif //CFDPAGE_H



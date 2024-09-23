#ifndef REQRESP_WIDGET_H
#define REQRESP_WIDGET_H

#include "lsimplewidget.h"



// ReqRespWidget
class ReqRespWidget : public LTableWidgetBox
{
    Q_OBJECT
public:
    ReqRespWidget(QWidget *parent = 0);
    virtual ~ReqRespWidget() {}

    static int staticRows() {return 5;}
    void updateTable(const QStringList&, const QStringList&);

protected:
    void resetTable();

};



#endif // REQRESP_WIDGET_H



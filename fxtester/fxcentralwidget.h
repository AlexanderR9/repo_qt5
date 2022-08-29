#ifndef FX_CENTRAL_WIDGET_H
#define FX_CENTRAL_WIDGET_H

#include "ui_fxcentralwidget.h"


// MainForm
class FXCentralWidget : public QWidget, public Ui::FXCentralWidget
{
    Q_OBJECT
public:
    FXCentralWidget(QWidget *parent = 0);
    virtual ~FXCentralWidget() {}


};

#endif //FX_CENTRAL_WIDGET_H


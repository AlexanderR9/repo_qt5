#ifndef BB_CENTRALWIDGET_H
#define BB_CENTRALWIDGET_H

#include "lsimplewidget.h"

//class LListWidgetBox;
class QStackedWidget;
class QSettings;


//BB_CentralWIdget
class BB_CentralWidget : public LSimpleWidget
{    
    Q_OBJECT
public:
    BB_CentralWidget(QWidget*);
    virtual ~BB_CentralWidget() {}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

protected:
    LListWidgetBox  *w_list;
    QStackedWidget  *w_stack;

    void init();
    void clearStack();
    void createPages();

};



#endif // BB_CENTRALWIDGET_H

#include "bb_centralwidget.h"
#include "bb_chartpage.h"

#include <QStackedWidget>
#include <QSplitter>
#include <QSettings>
#include <QAction>
#include <QListWidget>



//BB_CentralWidget
BB_CentralWidget::BB_CentralWidget(QWidget *parent)
    :LSimpleWidget(parent, 20),
      w_list(NULL),
      w_stack(NULL)
{
    setObjectName("central_widget");
    init();
    createPages();

}
void BB_CentralWidget::createPages()
{
    BB_ChartPage *p_chart = new BB_ChartPage(this);
    w_list->addItem(p_chart->caption(), p_chart->iconPath());
    w_stack->addWidget(p_chart);

}
void BB_CentralWidget::init()
{
    w_list = new LListWidgetBox(this);
    w_list->setTitle("Pages");
    w_list->setBaseColor("#EFECEC", "#556B2F");

    w_stack = new QStackedWidget(this);
    clearStack();

    h_splitter->addWidget(w_list);
    h_splitter->addWidget(w_stack);
}
void BB_CentralWidget::clearStack()
{
    while (w_stack->count() > 0)
    {
        QWidget *w = w_stack->widget(0);
        w_stack->removeWidget(w);
        delete w;
        w = NULL;
    }
}
void BB_CentralWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->load(settings);
    }

    /*
    QByteArray ba = settings.value(QString("%1/v_spltitter_state").arg(objectName()), QByteArray()).toByteArray();
    if (v_splitter && !ba.isEmpty()) v_splitter->restoreState(ba);

    ba.clear();
    ba = settings.value(QString("%1/h_spltitter_state").arg(objectName()), QByteArray()).toByteArray();
    if (h_splitter && !ba.isEmpty()) h_splitter->restoreState(ba);
    */

}
void BB_CentralWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);

    for (int i=0; i<w_stack->count(); i++)
    {
        LSimpleWidget *w = qobject_cast<LSimpleWidget*>(w_stack->widget(i));
        if (w) w->save(settings);
    }

    /*
    if (v_splitter)
        settings.setValue(QString("%1/v_spltitter_state").arg(objectName()), v_splitter->saveState());

    if (h_splitter)
        settings.setValue(QString("%1/h_spltitter_state").arg(objectName()), h_splitter->saveState());
        */
}



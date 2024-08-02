
#include <QApplication>
#include "mainform.h"

int main(int args, char **argv)
{
    QApplication app(args, argv);
    qDebug("main 1");
    MainForm *form = new MainForm();
    qDebug("main 2");

    form->init();
    qDebug("main 3");
    form->show();
    qDebug("main 4");
    return app.exec();
};



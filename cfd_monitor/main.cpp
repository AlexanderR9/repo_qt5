#include <QApplication>
#include "mainform.h"


int main(int args, char **argv)
{
    QApplication app(args, argv);
    MainForm *form = new MainForm();
    form->init();
    form->show();
    return app.exec();
}



 #include "mainwidget.h"
 #include <QApplication>

int main(int argc, char** argv)
{
    QApplication app( argc, argv );
    MainWidget mw;
    mw.show();
    return app.exec();
}


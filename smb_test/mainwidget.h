#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "ui_mainwidget.h"

class DSNSFileStruct;


////////////// class MainWidget //////////////////////////////////////
class MainWidget : public QWidget, public  Ui::mainWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = NULL);
    virtual ~MainWidget() {}

protected:
    DSNSFileStruct *m_reader;
    QTimer *m_timer;

    void addProtocol(QString s = QString(), bool isErr = false);

protected slots:
    void slotRead();
    void slotStart();
    void slotStop();
    void slotTimer();

};


#endif




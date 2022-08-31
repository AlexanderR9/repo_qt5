#ifndef TCPSTATUSWIDGET_H
#define TCPSTATUSWIDGET_H


#include "ui_tcpstatuswidget.h"


//виджет для отображения состояния сетевого подключения TCP объекта (клиента или сервера)
//предаствляет собой групбокс в котором по горизонтали стоят иконка и надпись
class LTCPStatusWidget : public QWidget, public Ui::TCPStatusWidget
{
    Q_OBJECT
public:
    LTCPStatusWidget(QWidget*);
    virtual ~LTCPStatusWidget() {}

    void setTextMode(const QString&); //установить главную надпись виджета
    void setOffState();
    void setListeningState(); //only server
    void setConnectedState();
    void setConnectingState();   //only client

};


#endif


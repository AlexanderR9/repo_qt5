#ifndef MBTCPCENTRALWIDGET_H
#define MBTCPCENTRALWIDGET_H


#include "lsimplewidget.h"


class ReqRespWidget;
class ExchangeStateWidget;
class SlaveWidget;


//MBTcpCentralWidget
class MBTcpCentralWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    MBTcpCentralWidget(QWidget *parent = 0);
    virtual ~MBTcpCentralWidget() {}

    void reinitWidget();

    inline bool isMaster() const {return (m_mode==0);}

protected:
    SlaveWidget             *m_slaveWidget;
    ReqRespWidget           *m_reqRespWidget;
    ExchangeStateWidget     *m_stateWidget;
    quint8 m_mode; // 0-master, 1-slave

};



//SlaveWidget
class SlaveWidget : public LTabWidgetBox
{
    Q_OBJECT
public:
    SlaveWidget(QWidget *parent = 0);
    virtual ~SlaveWidget() {}

};

//RegDataTable
class RegDataTable : public LTableWidgetBox
{
    Q_OBJECT
public:
    RegDataTable(QWidget *parent = 0);
    virtual ~RegDataTable() {}

protected:

};



#endif // MBTCPCENTRALWIDGET_H


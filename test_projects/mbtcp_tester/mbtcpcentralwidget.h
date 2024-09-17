#ifndef MBTCPCENTRALWIDGET_H
#define MBTCPCENTRALWIDGET_H


#include "lsimplewidget.h"


class ReqRespWidget;
class ExchangeStateWidget;
class SlaveWidget;
class MasterWidget;
class QLineEdit;
class QGridLayout;
class QSettings;
class QModbusRequest;


//MBTcpCentralWidget
class MBTcpCentralWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    MBTcpCentralWidget(QWidget *parent = 0);
    virtual ~MBTcpCentralWidget() {}

    void reinitWidget(int);
    virtual void load(QSettings&);
    virtual void save(QSettings&);


    inline bool isMaster() const {return (m_masterWidget != NULL);}
    inline bool isSlave() const {return (m_slaveWidget != NULL);}

protected:
    SlaveWidget             *m_slaveWidget;
    MasterWidget            *m_masterWidget;
    ReqRespWidget           *m_reqRespWidget;
    ExchangeStateWidget     *m_stateWidget;
    //quint8 m_mode; // 0-master, 1-slave
    QWidget                 *m_mainWidget;

    void resetMainWidget();

public slots:
    void slotUpdateState(const QStringList&);
    void slotFillReq(QModbusRequest&, quint8&, QString&);

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

//MasterWidget
class MasterWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    MasterWidget(QWidget *parent = 0);
    virtual ~MasterWidget() {}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    void fillReq(QModbusRequest&, quint8&, QString&);

protected:
    QLineEdit   *m_transactionIdEdit;
    QLineEdit   *m_devAddrEdit;
    QLineEdit   *m_funcCodeEdit;
    QLineEdit   *m_startRegEdit;
    QLineEdit   *m_regsLenEdit;

    LTableWidgetBox *m_regTable;


    void init();
    void addBoxLineEdit(QGridLayout*);

protected slots:
    void slotRegsCountChanged();

};



#endif // MBTCPCENTRALWIDGET_H


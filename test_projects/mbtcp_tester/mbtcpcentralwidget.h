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
class QModbusResponse;



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
    QWidget                 *m_mainWidget;

    void resetMainWidget();

public slots:
    void slotUpdateState(const QStringList&);
    void slotFillReq(QModbusRequest&, quint8&, QString&);
    void slotUpdateReqRespTable(const QStringList&, const QStringList&);
    void slotUpdateRegTable(const QModbusResponse&);

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

    void regsCountChanged(quint32, quint32);
    quint16 regCount() const;

    //записать в поток инфу о значениях(и) регистров (зависит от кода функции) для отправки запроса
    void fillRegValues(QDataStream&, quint8, quint16);

    //по приходу ответа обновить значения регистров, выполняется только когда была команда на чтение
    void updateRegValueByResponse(quint8, const QByteArray&);

protected:
    quint32 m_startReg;
    QVector<quint8> m_bitSwithers; //значения (8шт) - степени 2, инициализтруются в конструкторе, служат для вкл/выкл битов в байте

    void setPropertyItems();
    quint16 valueAt(quint16 addr) const;
    void initBitSwithers();

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

    //заполнить объект QModbusRequest и адрес устройства для запроса, которые надо взять с виджета
    void fillReq(QModbusRequest&, quint8&, QString&);

    //по приходу ответа обновить значения регистров в таблице если ответ валиден и была команда на чтение
    void updateRegValueByResponse(const QModbusResponse&);

protected:
    QLineEdit   *m_transactionIdEdit;
    QLineEdit   *m_devAddrEdit;
    QLineEdit   *m_funcCodeEdit;
    QLineEdit   *m_startRegEdit;
    QLineEdit   *m_regsLenEdit;

    RegDataTable *m_regTable;


    void init();
    void addBoxLineEdit(QGridLayout*);

    //заполнить объект полезные данные запроса (QByteArray), т.е. инфу о диапазоне регистров, сами значения (зависит от кода функции)
    void fillReqData(QDataStream&, quint8, quint16);


protected slots:
    void slotRegsCountChanged();

};



#endif // MBTCPCENTRALWIDGET_H


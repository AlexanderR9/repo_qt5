#ifndef BB_POSITIONSPAGE_H
#define BB_POSITIONSPAGE_H

#include "bb_basepage.h"


struct BB_APIReqParams;
class QJsonObject;
class QPointF;
class QJsonArray;
struct BB_BagState;
struct TradeOperationData;


//BB_PositionsPage
class BB_PositionsPage : public BB_BasePage
{
    Q_OBJECT
public:
    BB_PositionsPage(QWidget*);
    virtual ~BB_PositionsPage() {}

    QString iconPath() const {return QString(":/icons/images/ball_green.svg");}
    QString caption() const {return QString("Positions");}

    virtual void updateDataPage(bool force = false);

protected:
    LSearchTableWidgetBox     *m_table;
    LSearchTableWidgetBox     *m_orderTable;

    void init();
    QStringList tableHeaders(QString) const;
    virtual void fillPosTable(const QJsonArray&);
    virtual void fillOrdersTable(const QJsonArray&);
    void clearTables(); //remove all rows
    void checkAdjacent(int);

    void initPopupMenu(); //инициализировать элементы всплывающего меню
    virtual void sendTradeReq(const TradeOperationData&);

private:
    void initTable(LSearchTableWidgetBox*);
    QString getTimePoint(const QJsonObject&, bool&) const;


public slots:
    virtual  void slotJsonReply(int, const QJsonObject&);
    void slotGetPosState(BB_BagState&);


protected slots:
    virtual void slotOrderModify();
    virtual void slotOrderCancel();

};


//BB_SpotPositionsPage
class BB_SpotPositionsPage : public BB_PositionsPage
{
    Q_OBJECT
public:
    BB_SpotPositionsPage(QWidget*);
    virtual ~BB_SpotPositionsPage() {}

    QString iconPath() const {return QString(":/icons/images/ball_yellow.svg");}
    QString caption() const {return QString("Positions (SPOT)");}

    virtual void updateDataPage(bool force = false);


protected:
    virtual void fillPosTable(const QJsonArray&);
    virtual void fillOrdersTable(const QJsonArray&);

    void reinitWidgets();
    void getSpotOrders();

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);


};


//BB_OptionPositionsPage
class BB_OptionPositionsPage : public BB_PositionsPage
{
    Q_OBJECT
public:
    BB_OptionPositionsPage(QWidget*);
    virtual ~BB_OptionPositionsPage() {}

    QString iconPath() const {return QString(":/icons/images/timer_option.png");}
    QString caption() const {return QString("Positions (OPTION)");}

    virtual void updateDataPage(bool force = false);


protected:
    virtual void fillPosTable(const QJsonArray&);
    virtual void fillOrdersTable(const QJsonArray&);

    void reinitWidgets();
    void reinitReqData();


public slots:
    virtual void slotJsonReply(int, const QJsonObject&);

protected slots:
    virtual void slotOrderModify();
    virtual void slotOrderCancel();




};


#endif // BB_POSITIONSPAGE_H

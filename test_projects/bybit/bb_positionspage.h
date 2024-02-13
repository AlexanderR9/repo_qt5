#ifndef BB_POSITIONSPAGE_H
#define BB_POSITIONSPAGE_H

#include "lsimplewidget.h"


struct BB_APIReqParams;
class QJsonObject;
class QPointF;
class QJsonArray;





//BB_ChartPage
class BB_PositionsPage : public LSimpleWidget
{
    Q_OBJECT
public:
    BB_PositionsPage(QWidget*);
    virtual ~BB_PositionsPage() {}

    QString iconPath() const {return QString(":/icons/images/ball_green.svg");}
    QString caption() const {return QString("Positions");}

    void updateDataPage();

protected:
    LSearchTableWidgetBox     *m_table;
    LSearchTableWidgetBox     *m_orderTable;
    int cur_stage;

    void init();
    QStringList tableHeaders(QString) const;
    void fillPosTable(const QJsonArray&);
    void fillOrdersTable(const QJsonArray&);

private:
    void initTable(LSearchTableWidgetBox*);

public slots:
    void slotJsonReply(int, const QJsonObject&);

signals:
    void signalSendReq(const BB_APIReqParams&);


};




#endif // BB_POSITIONSPAGE_H

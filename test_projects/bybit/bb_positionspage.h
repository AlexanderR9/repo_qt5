#ifndef BB_POSITIONSPAGE_H
#define BB_POSITIONSPAGE_H

#include "bb_basepage.h"


struct BB_APIReqParams;
class QJsonObject;
class QPointF;
class QJsonArray;
struct BB_BagState;


//BB_PositionsPage
class BB_PositionsPage : public BB_BasePage
{
    Q_OBJECT
public:
    BB_PositionsPage(QWidget*);
    virtual ~BB_PositionsPage() {}

    QString iconPath() const {return QString(":/icons/images/ball_green.svg");}
    QString caption() const {return QString("Positions");}

    void updateDataPage(bool force = false);

protected:
    LSearchTableWidgetBox     *m_table;
    LSearchTableWidgetBox     *m_orderTable;

    void init();
    QStringList tableHeaders(QString) const;
    void fillPosTable(const QJsonArray&);
    void fillOrdersTable(const QJsonArray&);
    void clearTables(); //remove all rows
    void checkAdjacent();

private:
    void initTable(LSearchTableWidgetBox*);
    QString getTimePoint(const QJsonObject&, bool&) const;

public slots:
    void slotJsonReply(int, const QJsonObject&);
    void slotGetPosState(BB_BagState&);

};




#endif // BB_POSITIONSPAGE_H

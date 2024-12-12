#ifndef UG_POSPAGE_H
#define UG_POSPAGE_H


#include "ug_basepage.h"
#include "ug_apistruct.h"

class QJsonObject;


//UG_PosPage
class UG_PosPage : public UG_BasePage
{
    Q_OBJECT
public:
    UG_PosPage(QWidget*);
    virtual ~UG_PosPage() {m_positions.clear();}

    QString iconPath() const {return QString(":/icons/images/bag.svg");}
    QString caption() const {return QString("Positions");}
    virtual void saveData() {}
    virtual void loadData() {}

    virtual void updateDataPage(bool forcibly = false);
    virtual void startUpdating(quint16);

protected:
    QList<UG_PosInfo>       m_positions; //контейнер для хранения записей-позиций
    LSearchTableWidgetBox   *m_tableBox;

    virtual void clearPage();
    void initTable();
    void prepareQuery();
    void parseJArrPos(const QJsonArray&);
    void updateTableData();
    void sortPositions();

private:
    QList<int> openedIndexes() const;


public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    virtual void slotReqBuzyNow();



};


#endif // UG_POSPAGE_H



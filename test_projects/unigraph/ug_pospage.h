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

    virtual QString iconPath() const {return QString(":/icons/images/bag.svg");}
    virtual QString caption() const {return QString("Positions");}
    virtual void saveData() {}
    virtual void loadData() {}

    virtual void updateDataPage(bool forcibly = false);
    virtual void startUpdating(quint16);

protected:
    QList<UG_PosInfo>       m_positions; //контейнер для хранения записей-позиций
    LSearchTableWidgetBox   *m_tableBox;

    virtual void clearPage();
    virtual void initTable();
    void prepareQuery();
    void parseJArrPos(const QJsonArray&);
    virtual void updateTableData();
    virtual void sortPositions();
    virtual void updateLastRowColor(const UG_PosInfo&);

private:
    QList<int> openedIndexes() const;


public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    virtual void slotReqBuzyNow();

};

//UG_ActivePosPage
class UG_ActivePosPage : public UG_PosPage
{
    Q_OBJECT
public:
    UG_ActivePosPage(QWidget*);
    virtual ~UG_ActivePosPage() {}

    virtual QString iconPath() const {return QString(":/icons/images/ball_green.svg");}
    virtual void startUpdating(quint16);

protected:
    int m_chainIndex;
    bool wait_data;

    virtual void initTable();
    virtual void updateTableData();
    virtual void sortPositions();
    void nextChainReq();
    virtual void updateLastRowColor(const UG_PosInfo&);
    void calcStatbleAssetsLastRow(const UG_PosInfo&);

protected slots:
    virtual void slotTimer();

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    virtual void slotReqBuzyNow();

};

#endif // UG_POSPAGE_H



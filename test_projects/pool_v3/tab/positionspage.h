#ifndef POSITIONSPAGE_H
#define POSITIONSPAGE_H

#include "basetabpage_v3.h"
#include "defiposition.h"


class QJsonObject;
class QJsonValue;


//DefiPositionsPage
class DefiPositionsPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiPositionsPage(QWidget*);
    virtual ~DefiPositionsPage() {}

    virtual void sendUpdateDataRequest();

    virtual void setChain(int);

    inline int posCount() const {return m_positions.count();}
    inline bool hasPos() const {return (posCount() > 0);}

protected:
    QList<DefiPosition> m_positions;

    void initTable();
    void updatePositionsData(const QJsonObject&);
    void updatePositionsState(const QJsonObject&);
    void updatePositionState(int, const QJsonValue&);
    void readNodejsPosFile(); // после успешного запроса позиций необходимо считать файл positions.txt
    void reloadTableByRecords(); //перезаписать таблицу после успешного запроса позиций и парсинга positions.txt
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    int posIndexOf(int) const; // поиск позиции по ее PID, вернет индекс в контейнере или -1

private:
    QString poolInfo(const DefiPosition&) const;

public slots:
    virtual void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs

protected slots:
    void slotGetSelectedPosState();
    void slotGetLiquidityPosState();


};




#endif // POSITIONSPAGE_H


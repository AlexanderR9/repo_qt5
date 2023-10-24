#ifndef BAGSTATE_H
#define BAGSTATE_H


#include "lsimpleobj.h"

#include <QList>
#include <QStringList>

class QJsonObject;

//BagPosition
struct BagPosition
{
    BagPosition() {reset();}

    QString uid;
    quint32 count;
    float average_price;
    float current_price;
    QString paper_type;

    void reset() {uid.clear(); count = 0; average_price = current_price = -1; paper_type.clear();}
    bool invalid() const {return (uid.isEmpty() || count == 0);}
    QString strPrice() const {return QString("%1 / %2").arg(QString::number(average_price, 'f', 1)).arg(QString::number(current_price, 'f', 1));}
    float curProfit() const {return (float(count)*(current_price - average_price));}
    QString strProfit() const {return QString::number(curProfit(), 'f', 1);}


};



//BagState
class BagState : public LSimpleObject
{
    Q_OBJECT
public:
    BagState(QObject*);
    virtual ~BagState() {m_positions.clear();}

    QStringList tableHeaders() const;
    virtual QString name() const {return QString("bag_obj");}

    inline QString strBlocked() const {return QString::number(m_blocked, 'f', 1);}
    inline QString strFree() const {return QString::number(m_free, 'f', 1);}
    inline QString strTotal() const {return QString::number(m_total, 'f', 1);}
    inline quint16 posCount() const {return m_positions.count();}
    inline bool hasPositions() const {return !m_positions.isEmpty();}
    inline const BagPosition& posAt(quint16 i) const {return m_positions.at(i);}

protected:
    QList<BagPosition> m_positions;
    float m_total;
    float m_blocked;
    float m_free;

    void reset();
    void parsePositions(const QJsonArray&);

public slots:
    void slotLoadPositions(const QJsonObject&);
    void slotLoadPortfolio(const QJsonObject&);

signals:
    void signalBagUpdate();

};



#endif // BAGSTATE_H




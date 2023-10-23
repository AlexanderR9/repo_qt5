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

    void reset() {uid.clear(); count = 0; average_price = current_price = -1;}

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

protected:
    QList<BagPosition> m_positions;
    float m_total;
    float m_blocked;
    float m_free;

    void reset();

public slots:
    void slotLoadPositions(const QJsonObject&);
    void slotLoadPortfolio(const QJsonObject&);

signals:
    void signalBagUpdate();

};



#endif // BAGSTATE_H




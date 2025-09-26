#ifndef POSITIONSPAGE_H
#define POSITIONSPAGE_H

#include "basetabpage_v3.h"

class QJsonObject;


//DefiPositionsPage
class DefiPositionsPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiPositionsPage(QWidget*);
    virtual ~DefiPositionsPage() {}

    virtual void sendUpdateDataRequest();

    virtual void setChain(int);

protected:
    void initTable();
    void updatePositionsData(const QJsonObject&);

public slots:
    virtual void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs


};




#endif // POSITIONSPAGE_H


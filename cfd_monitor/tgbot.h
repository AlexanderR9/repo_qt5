#ifndef TGBOT_H
#define TGBOT_H


#include "tgabstractbot.h"


// TGBot
class TGBot : public LTGAbstractBot
{
    Q_OBJECT
public:
    TGBot(QObject*);
    virtual ~TGBot() {}

    QString name() const {return QString("MyLBot");}

protected slots:
    void slotJsonReceived(QJsonObject);
    void slotJArrReceived(QJsonArray);

};


#endif //TGBOT_H



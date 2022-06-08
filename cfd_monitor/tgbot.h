#ifndef TGBOT_H
#define TGBOT_H


#include "tgabstractbot.h"

#include <QMap>

// TGBot
class TGBot : public LTGAbstractBot
{
    Q_OBJECT
public:
    TGBot(QObject*);
    virtual ~TGBot() {}

    QString name() const {return QString("MyLBot");}
    QString toStrParams() const;

    QMap<QString, QString> getParams() const;

protected slots:
    void slotJsonReceived(QJsonObject);
    void slotJArrReceived(QJsonArray);

};


#endif //TGBOT_H



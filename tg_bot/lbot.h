#ifndef LBOT_H
#define LBOT_H


#include "tgabstractbot.h"


// LBot
class LBot : public LTGAbstractBot
{
    Q_OBJECT
public:
    LBot(QObject*);
    virtual ~LBot() {}

    void loadConfig(const QString&);
    QString name() const {return QString("MyLBot");}


    //static service funcs
    static void jsonToDebug(const QJsonObject&, quint8 level = 0); //выхлоп объекта json в debug
    static QString jsonValueToStr(const QJsonValue&);

protected:
    void reset() {m_params.reset();}

protected slots:
    void slotJsonReceived(QJsonObject);
    void slotJArrReceived(QJsonArray);

};



#endif // LBOT_H

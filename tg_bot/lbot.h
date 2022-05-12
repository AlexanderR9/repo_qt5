#ifndef LBOT_H
#define LBOT_H


#include <lsimpleobj.h>


class TarnaBot;

// LBot
class LBot : public LSimpleObject
{
    Q_OBJECT
public:
    LBot(QObject*);
    virtual ~LBot() {}

    void loadConfig(const QString&);
    QString name() const {return QString("tgbot_obj");}


protected:
    QString      m_token;
    TarnaBot    *m_botObj;


    void init();


};


#endif // LBOT_H

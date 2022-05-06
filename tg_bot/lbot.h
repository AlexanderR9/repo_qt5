#ifndef LBOT_H
#define LBOT_H


#include <QObject>


class TarnaBot;

// LBot
class LBot : public QObject
{
    Q_OBJECT
public:
    LBot(QObject*);
    virtual ~LBot() {}



protected:
    QString      m_token;
    TarnaBot    *m_botObj;


    void init();


};


#endif // LBOT_H

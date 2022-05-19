#ifndef TG_ABSTRACT_BOT_H
#define TG_ABSTRACT_BOT_H


#include "lsimpleobj.h"


#include <QJsonObject>
#include <QJsonArray>

class LTGSender;

// основное настройки бота
// TGParamsBot
struct LTGParamsBot
{
    LTGParamsBot() {reset();}

    QString     token;  //bot token
    qint64      chatID;
    quint8      req_timeout; //sec
    quint8      limit_msg; //max msg count

    void setParams(const LTGParamsBot&);
    QString toStr() const;
    void reset();
};


//абстрактый класс бота TG,
//для создания своего бота необходимо унаследоваться от этого.
//релиазовать свой алгоритм в слотах slotJsonReceived и slotJArrReceived.
//после создания объекта класса своего бота надо установить параметры бота методом setBotParams.
// TGAbstractBot
class LTGAbstractBot : public LSimpleObject
{
    Q_OBJECT
public:
    LTGAbstractBot(QObject*);
    virtual ~LTGAbstractBot() {}

    virtual QString name() const = 0;
    virtual bool invalid() const;   // валидность параметров бота
    virtual void setBotParams(const LTGParamsBot&);
    virtual void loadConfig(const QString&);


    //tg funcs
    void getMe();
    void getUpdates(qint64 last_update_id = -1);
    void sendMsg(const QString&);

protected:
    LTGParamsBot    m_params;   //основные параметры бота
    LTGSender       *m_sender;  //объект для обмена с сервером TG

    virtual void reinitSender(); //параметры бота изменились, необходимо переинициализировать объект m_sender
    virtual void reset() {m_params.reset();}

protected slots:
    virtual void slotJsonReceived(QJsonObject) = 0; //пришел нормальный ответ в виде QJsonObject, необходимо обработать ответ
    virtual void slotJArrReceived(QJsonArray) = 0; //пришел нормальный ответ в виде QJsonArray, необходимо обработать ответ
    virtual void slotFinishedFault(); //запрос завершился неудачно, описание ошибки в переменной m_err

};



#endif // TG_ABSTRACT_BOT_H


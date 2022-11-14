#ifndef DIANA_VIEW_WIDGET_H
#define DIANA_VIEW_WIDGET_H

#include "lsimplewidget.h"


class QSettings;
class LXMLPackView;
class DianaObject;
class MQ;


// DianaViewWidget
class DianaViewWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    DianaViewWidget(const QString&, bool serv_mode, QWidget *parent = 0);
    virtual ~DianaViewWidget() {}


    void loadMQPacket(const QString&); //загрузить указанный пакет
    QString name() const {return objectName();}
    void setExpandLevel(int);
    void setDoublePrecision(quint8);
    void updateMQState();

    //если режим Client, то функция работает по таймеру и с некоторым интервалом записывает пакет в очередь типа input.
    //в режиме Server функция может разово срабытывать по нажатию кнопки пользувателем для принудительной отправки пакета в очередь типа output.
    void sendMsgToQueue();

    void readLastMsgMQ(); //считать последний пакет из очереди типа input (функция работает в режиме Server(эмулятора дианы)  по таймеру)


    void destroyAllQueues();
    void recreatePosixQueues();

    inline void setAutoRecalcPackValues(bool b) {m_autoUpdatePackValues = b;}
    inline void setAutoReadMsg(bool b) {m_autoUpdateReadMsg = b;}

protected:
    LXMLPackView            *m_inView;
    LXMLPackView            *m_outView;
    DianaObject             *m_dianaObj;
    bool                     m_autoUpdatePackValues; //признак того что значения отправляемого пакета будут автоматом пересчитываться с учетом случайной состовляющей
    bool                     m_autoUpdateReadMsg; //признак того что значения отправляемого пакета будут автоматом пересчитываться с учетом случайной состовляющей
    bool                     is_serv; //признак того что еэмулятор работает в режиме имитатора самой дианы, иначе как клиент для дианы

    void initWidget();

    void loadPack(LXMLPackView*, const QString&);
    void readMsgFromQueue(); //проверить наличие сообщений в очереди (output) и считать его
    void sengMsgFromView(LXMLPackView*);

protected slots:
    void slotReadingTimer();
    void slotSetPacketSize(const QString&, quint32&);

signals:
    void signalMQCreated(const QString&, quint32, const MQ*);
    void signalSendMsgOk(const QString&);
    void signalReceiveMsgOk(const QString&);
    void signalSendMsgErr(const QString&);
    void signalReceiveMsgErr(const QString&);

};




#endif //DIANA_VIEW_WIDGET_H


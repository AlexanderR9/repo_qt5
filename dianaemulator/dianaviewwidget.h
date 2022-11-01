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
    DianaViewWidget(const QString&, QWidget *parent = 0);
    virtual ~DianaViewWidget() {}


    void loadMQPacket(const QString&); //загрузить указанный пакет
    QString name() const {return objectName();}
    void setExpandLevel(int);
    void setDoublePrecision(quint8);
    void updateMQState();
    void sendMsgToQueue(); //записать пакет в очередь (input)

    inline void setAutoRecalcPackValues(bool b) {m_autoUpdatePackValues = b;}

protected:
    LXMLPackView            *m_inView;
    LXMLPackView            *m_outView;
    DianaObject             *m_dianaObj;
    bool                     m_autoUpdatePackValues; //признак того что значения отправляемого пакета будут автоматом пересчитываться с учетом случайной состовляющей

    void initWidget();

    void loadPack(LXMLPackView*, const QString&);
    void readMsgFromQueue(); //проверить наличие сообщений в очереди (output) и считать его

protected slots:
    void slotReadingTimer();

signals:
    void signalMQCreated(const QString&, quint32, const MQ*);
    void signalSendMsgOk(const QString&);
    void signalReceiveMsgOk(const QString&);
    void signalSendMsgErr(const QString&);
    void signalReceiveMsgErr(const QString&);

};




#endif //DIANA_VIEW_WIDGET_H


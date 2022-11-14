#ifndef MQ_GENERAL_PAGE_H
#define MQ_GENERAL_PAGE_H

#include "lsimplewidget.h"

#include <QMap>

class QSettings;
class MQ;
class QTreeWidget;


// MQGeneralPage
class MQGeneralPage : public LSimpleWidget
{
    Q_OBJECT
public:
    MQGeneralPage(QWidget *parent = 0);
    virtual ~MQGeneralPage() {}

    void updateMQState();
    inline void setServerMode(bool b) {is_serv = b;}

protected:
    LTableWidgetBox     *m_tableBox;
    LTreeWidgetBox      *m_viewBox;
    QMap<int, const MQ*>    m_queues;
    bool                     is_serv; //признак того что еэмулятор работает в режиме имитатора самой дианы, иначе как клиент для дианы

    void initWidget();
    void appendDianaToView(const QString&, const QString&);
    QTreeWidget* view() const;

public slots:
    void slotAppendMQ(const QString&, quint32, const MQ*);
    void slotSendMsgOk(const QString&);
    void slotReceiveMsgOk(const QString&);
    void slotSendMsgErr(const QString&);
    void slotReceiveMsgErr(const QString&);

private:
    int viewDianaIndex(const QString&) const; //вернет индекс заглавного итема по имени дианы или -1
    int sendingItemIndexByMode() const; // вернет индекс итема-ребенка дианы (0/1) взависимости от значения is_serv
    int receivingItemIndexByMode() const; // вернет индекс итема-ребенка дианы (0/1) взависимости от значения is_serv

};

#endif //MQ_GENERAL_PAGE_H


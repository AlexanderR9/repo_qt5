#ifndef MQ_GENERAL_PAGE_H
#define MQ_GENERAL_PAGE_H

#include "lsimplewidget.h"

#include <QMap>

class QSettings;
class MQ;


// MQGeneralPage
class MQGeneralPage : public LSimpleWidget
{
    Q_OBJECT
public:
    MQGeneralPage(QWidget *parent = 0);
    virtual ~MQGeneralPage() {}

    void updateMQState();

protected:
    LTableWidgetBox     *m_tableBox;
    LTreeWidgetBox      *m_viewBox;
    QMap<int, const MQ*>    m_queues;

    void initWidget();

public slots:
    void slotAppendMQ(const QString&, quint32, const MQ*);

};

#endif //MQ_GENERAL_PAGE_H


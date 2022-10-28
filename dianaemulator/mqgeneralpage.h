#ifndef MQ_GENERAL_PAGE_H
#define MQ_GENERAL_PAGE_H

#include "lsimplewidget.h"


class QSettings;
class MQ;


// MQGeneralPage
class MQGeneralPage : public LSimpleWidget
{
    Q_OBJECT
public:
    MQGeneralPage(QWidget *parent = 0);
    virtual ~MQGeneralPage() {}


protected:
    LTableWidgetBox     *m_tableBox;
    LTreeWidgetBox      *m_viewBox;

    void initWidget();

public slots:
    void slotAppendMQ(const QString&, quint32, const MQ*);

};

#endif //MQ_GENERAL_PAGE_H


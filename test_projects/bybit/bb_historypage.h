#ifndef BB_HISTORYPAGE_H
#define BB_HISTORYPAGE_H

#include "lsimplewidget.h"


class LTableWidgetBox;
struct BB_APIReqParams;
class QJsonObject;


//BB_HistoryPage
class BB_HistoryPage : public LSimpleWidget
{
    Q_OBJECT
public:
    BB_HistoryPage(QWidget*);
    virtual ~BB_HistoryPage() {}

    QString iconPath() const {return QString(":/icons/images/ball_gray.svg");}
    QString caption() const {return QString("History");}

    void updateDataPage();

protected:
    LTableWidgetBox     *m_table;

    void init();
    QStringList tableHeaders() const;

public slots:
    void slotJsonReply(int, const QJsonObject&);

signals:
    void signalSendReq(const BB_APIReqParams&);

};



#endif // BB_HISTORYPAGE_H

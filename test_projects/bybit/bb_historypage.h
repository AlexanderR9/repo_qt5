#ifndef BB_HISTORYPAGE_H
#define BB_HISTORYPAGE_H

#include "bb_basepage.h"


class LTableWidgetBox;
struct BB_APIReqParams;
class QJsonObject;
class QJsonArray;


//BB_HistoryPage
class BB_HistoryPage : public BB_BasePage
{
    Q_OBJECT
public:
    BB_HistoryPage(QWidget*);
    virtual ~BB_HistoryPage() {}

    QString iconPath() const {return QString(":/icons/images/ball_gray.svg");}
    QString caption() const {return QString("History");}

    void updateDataPage(bool force = false);

protected:
    LSearchTableWidgetBox     *m_table;

    void init();
    QStringList tableHeaders() const;
    void fillOrdersTable(const QJsonArray&);

public slots:
    void slotJsonReply(int, const QJsonObject&);

};



#endif // BB_HISTORYPAGE_H

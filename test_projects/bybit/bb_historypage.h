#ifndef BB_HISTORYPAGE_H
#define BB_HISTORYPAGE_H

#include "bb_basepage.h"


class LTableWidgetBox;
struct BB_APIReqParams;
class QJsonObject;
class QJsonArray;
class QLineEdit;
class QComboBox;
class QJsonValue;


//BB_HistoryPage
class BB_HistoryPage : public BB_BasePage
{
    Q_OBJECT
public:
    enum HStage {hsGetOrders = 75, hsWaitOrders, hsGetPos, hsWaitPos, hsFinished};

    BB_HistoryPage(QWidget*);
    virtual ~BB_HistoryPage() {}

    virtual void load(QSettings&);
    virtual void save(QSettings&);


    QString iconPath() const {return QString(":/icons/images/ball_gray.svg");}
    QString caption() const {return QString("History");}

    void updateDataPage(bool force = false);

protected:
    LSearchTableWidgetBox     *m_tableOrders;
    LSearchTableWidgetBox     *m_tablePos;
    QLineEdit                 *m_startDateEdit;
    QComboBox                 *m_historyDaysCombo;
    int h_stage;

    void init();
    void initOrdersTable();
    void initPosTable();
    void initFilterBox();
    QStringList tableHeaders(QString) const;
    void fillOrdersTable(const QJsonArray&);
    void fillPosTable(const QJsonArray&);
    void viewTablesUpdate(); //exec after each new request and research
    void goExchange(const QJsonObject &jresult_obj);
    void prepareOrdersReq();
    void preparePosReq();
    void waitOrders(const QJsonObject&);
    void waitPos(const QJsonObject&);
    void finished();

public slots:
    void slotJsonReply(int, const QJsonObject&);

private:
    void getTSRange(qint64 &ts1, qint64 &ts2);

};



#endif // BB_HISTORYPAGE_H

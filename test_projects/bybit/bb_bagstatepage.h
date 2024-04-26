#ifndef BB_BAGSTATEPAGE_H
#define BB_BAGSTATEPAGE_H

#include "bb_basepage.h"
#include "bb_apistruct.h"


//BB_BagStatePage
class BB_BagStatePage : public BB_BasePage
{
    Q_OBJECT
public:
    enum BStage {bsGetUnified = 175, bsGetFund, bsFinished};

    BB_BagStatePage(QWidget*);
    virtual ~BB_BagStatePage() {}

    QString iconPath() const {return QString(":/icons/images/bag.svg");}
    QString caption() const {return QString("Bag state");}
    //virtual void load(QSettings&) {}

    void updateDataPage(bool force = false);

protected:
    LTableWidgetBox     *m_table;
    BB_BagState          m_state;
    int                  m_stage;

    LTableWidgetBox     *m_historyTable;
    BB_HistoryState      m_historyState;

    void init();
    void initBagTable();
    void initHistoryTable();

    void updateBagTable();
    void updateHistoryTable();
    void goExchange();
    void getUnifiedState();
    void getFundState();
    void getHistoryInfo();

public slots:
    void slotJsonReply(int, const QJsonObject&);

signals:
    void signalGetPosState(BB_BagState&);
    void signalGetHistoryState(BB_HistoryState&);

};



#endif // BB_BAGSTATEPAGE_H



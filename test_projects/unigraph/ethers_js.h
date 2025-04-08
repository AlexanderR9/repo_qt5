#ifndef ETHERS_JS_H
#define ETHERS_JS_H

#include "ug_basepage.h"

class QJsonObject;
class QLineEdit;
class QSettings;
class LProcessObj;
class LTabWidgetBox;
class JSWalletTab;
class JSApproveTab;


//EthersPage
class EthersPage : public UG_BasePage
{
    Q_OBJECT
public:
    EthersPage(QWidget*);
    virtual ~EthersPage() {}

    QString iconPath() const {return QString(":/icons/images/crypto/ETH.png");}
    QString caption() const {return QString("Ethers JS");}
    //void setExpandLevel(int);

    void updateDataPage(bool) {qDebug("EthersPage::updateDataPage");}
    virtual void saveData() {};
    virtual void loadData() {};

    virtual void startUpdating(quint16); //выполняется когда пользователь нажимает кнопку "Start"

   // QString freeQueryData() const;

protected:
    LProcessObj     *m_procObj;
    LTabWidgetBox   *m_tab;
    JSWalletTab     *m_walletPage;
    JSApproveTab    *m_approvePage;

    void initWidgets();
    void initProcessObj();
    virtual void clearPage() {}
    void tryUpdateBalace();
    void parseResultBuffer();

    bool walletPageNow() const;

public slots:
    void slotJsonReply(int, const QJsonObject&);
    void slotReqBuzyNow() {}

protected slots:
    void slotScriptFinished();

};

#endif // ETHERS_JS_H

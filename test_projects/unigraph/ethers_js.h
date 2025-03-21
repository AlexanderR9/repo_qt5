#ifndef ETHERS_JS_H
#define ETHERS_JS_H

#include "ug_basepage.h"

class QJsonObject;
class QLineEdit;
class QSettings;
class LProcessObj;


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


    /*
    LTreeWidgetBox      *m_replyBox;
    quint8               m_precision;
    QLineEdit           *m_reqEdit;


    void initWidgets();
    void initQueryBox();
    void cutPrecision(QTreeWidgetItem*);
    */
    virtual void clearPage() {}

public slots:
    void slotJsonReply(int, const QJsonObject&) {}
    void slotReqBuzyNow() {}

protected slots:
    void slotScriptFinished();

};

#endif // ETHERS_JS_H

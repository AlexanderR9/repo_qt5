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
class JSTxTab;
class JSPoolTab;
class BalanceHistoryTab;
class JSPosManagerTab;
class JSTxLogger;
class LSplash;


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
    //сохранение/восстановление сплитеров
    virtual void load(QSettings&);
    virtual void save(QSettings&);

    virtual void startUpdating(quint16); //выполняется когда пользователь нажимает кнопку "Start"

    static QString inputParamsJsonFile() {return QString("params.json");}

protected:
    LProcessObj     *m_procObj;
    LTabWidgetBox   *m_tab;
    JSWalletTab     *m_walletPage;
    JSApproveTab    *m_approvePage;
    JSTxTab         *m_txPage;
    JSPoolTab       *m_poolPage;
    BalanceHistoryTab   *m_balanceHistoryPage;
    JSPosManagerTab   *m_posManagerPage;
    JSTxLogger      *m_txLogger;
    LSplash         *m_splashWidget;

    void initWidgets();
    void initProcessObj();
    void initTxLoggerObj();
    virtual void clearPage() {}
    void tryUpdateBalace();
    void parseResultBuffer();
    void startProcessObj();

    bool walletPageNow() const;
    bool approvePageNow() const;
    bool txPageNow() const;
    bool poolsPageNow() const;
    bool balancePageNow() const;
    bool posManagerPageNow() const;


private:
    //при формировании результа (json) в node_js скриптах - недопустимо использовать символы ',' '['  ']' ':' в текстовых значениях полей,
    //т.к. по этим символам распознаються границы пар-значений и значения-массивы.
    //данная функция подрехтовывает JS json-result чтобы он был валиден для загрузки в QJsonObject
    QString transformJsonResult(const QString&) const;

public slots:
    void slotJsonReply(int, const QJsonObject&);
    void slotReqBuzyNow() {}
    void slotCheckUpproved(QString);
    void slotApprove(const QStringList&);
    void slotWalletTx(const QStringList&);
    void slotCheckTx(const QStringList&);
    void slotPoolAction(const QStringList&);
    void slotPosManagerAction(const QStringList&);
    void slotChainUpdated();
    void slotRewriteParamJson(const QJsonObject&);

protected slots:
    void slotScriptFinished();

signals:
    void signalEnableControls(bool);
    void signalScriptBroken(); //скрипт не выполнился, произошла ошибка и ответ содержит поле 'error'


};

#endif // ETHERS_JS_H

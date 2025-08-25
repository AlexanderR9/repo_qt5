#ifndef V3_BASETAB_H
#define V3_BASETAB_H

#include "lsimplewidget.h"


class QTimer;
class QJsonObject;
class NodejsBridge;
class DefiWalletTabPage;


//главный таб для работы в одной сети
//DefiChainTabV3
class DefiChainTabV3 : public LSimpleWidget
{
    Q_OBJECT
public:
    DefiChainTabV3(QWidget*, int chain_id);
    virtual ~DefiChainTabV3() {reset();}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    inline int chainId() const {return userSign();}

    QTabWidget* tabWidget() const;

    void tabActivated(); //выполняется когда происходит смена сети и этот таб всплывает наверх в стеке табов
    void startUpdating(); //запустить сценарий запросов
    void stopUpdating();

    void connectPageSignals();


protected:
    LTabWidgetBox       *m_tab;
    QTimer              *m_timer; // таймер для отслеживания состояния сценария запросов и ошибок
    quint32              m_timerCounter; //счетчик тиков таймера состояния (m_timer)
    NodejsBridge        *js_bridge; //объект для взаимодействия со скриптами nodejs

    virtual void reset() {}
    void initTab();
    void initJsBridgeObj();
    const DefiWalletTabPage* walletPage() const;

protected slots:
    void slotTimer(); // когда запускается некий сценарий запросов начинает работать m_timer и этот слот выполняется каждый его тик для оценки текщего состояния выполнения запросов
    void slotJSScriptFinished(int); //выполняется всякий раз когда очередной nodejs скрипт завершил работу (параметр - код успешности выполнения)
    //void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs

public slots:
    void slotTabsPricesUpdate(); // выполняется после упешно полученных последних цен


signals:
    void signalRewriteJsonFile(const QJsonObject&, QString);
    void signalEnableControls(bool);
    void signalGetPrices();


};


#endif // V3_BASETAB_H

#ifndef V3_BASETAB_H
#define V3_BASETAB_H

#include "lsimplewidget.h"


class QTimer;
class QJsonObject;
class NodejsBridge;
class DefiWalletTabPage;
class QLineEdit;
class DefiTxTabPage;
class DefiPositionsPage;


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
    void autoCheckStatusLastTx(); // после выполнения транзакции и задержки после нее, необходимо автоматом проверить статус выполнения
    int currentPageKind() const;
    void mintPos(); // запустить диалог для чеканки позы

protected:
    LTabWidgetBox       *m_tab;
    QTimer              *m_timer; // таймер для отслеживания состояния сценария запросов и ошибок
    quint32              m_timerCounter; //счетчик тиков таймера состояния (m_timer)
    NodejsBridge        *js_bridge; //объект для взаимодействия со скриптами nodejs
    QLineEdit           *m_reqStateEdit;

    virtual void reset() {}
    void initTab();
    void initJsBridgeObj();
    void initReqStateWidget();
    void changeCurrentPage(int);

    const DefiWalletTabPage* walletPage() const;
    DefiTxTabPage* txPage() const;
    DefiPositionsPage* positionsPage() const;

protected slots:
    void slotTimer(); // когда запускается некий сценарий запросов начинает работать m_timer и этот слот выполняется каждый его тик для оценки текщего состояния выполнения запросов
    void slotJSScriptFinished(int); //выполняется всякий раз когда очередной nodejs скрипт завершил работу (параметр - код успешности выполнения)
    void slotPageChanged(int);    // когда переключается вкладка на табе, параметр это индекс страницы
    void slotUpdatePageBack(QString req_name, QString extra_data = QString()); // после анализа результа последней транзакции перейти обратно на соотвутствующую страницу и обновить ее.

public slots:
    void slotTabsPricesUpdate(); // выполняется после упешно полученных последних цен
    void slotPageSendReq(QString, const QStringList&); //выполняется когда со страницы пришел сигнал на запуск js_bridge объекта

signals:
    void signalRewriteJsonFile(const QJsonObject&, QString);
    void signalEnableControls(bool);
    void signalTabPageChanged(int); // имитится когда переключается вкладка на табе, параметр это тип страницы (элемент DefiPageKind)

};


#endif // V3_BASETAB_H

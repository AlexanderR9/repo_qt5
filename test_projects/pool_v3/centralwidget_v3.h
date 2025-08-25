#ifndef V3_CENTRALWIDGET_H
#define V3_CENTRALWIDGET_H

#include "lsimplewidget.h"


class QStackedWidget;
class QSettings;
class DefiChainTabV3;
class QJsonObject;
class QJsonArray;
class LHttpApiRequester;

//CentralWidgetV3
class CentralWidgetV3 : public LSimpleWidget
{    
    Q_OBJECT
public:
    CentralWidgetV3(QWidget*);
    virtual ~CentralWidgetV3() {}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    void startUpdating(); // запустить сценарий запросов для обновления данных на текущей странице
    void breakUpdating(); // принудительный останов сценария запросов по кнопке
    void setEnableControl(bool);

protected:
    LListWidgetBox      *w_list;
    QStackedWidget      *w_stack;
    LHttpApiRequester   *http_requester; //bb price requester

    void init();
    void clearStack();
    void loadDefiData(); // загрузить данные из defi_config и инициализировать объект этими данными
    void createTabPages(int); //создать страницы таба для указанной сети
    void initHttpRequester();
    void newPricesReceived(const QJsonArray&); // был успешно получен ответ от http_requester, далее можно извлечь из него новые цены


    const DefiChainTabV3* tabByChain(int) const;
    DefiChainTabV3* currentTab() const;

protected slots:
    void slotChainChanged(int); // когда пользователь кликает в списке сетей по другой сети
    void slotRewriteJsonFile(const QJsonObject&, QString);
    void slotHttpReqFinished(int); //выполняется всякий раз когда завершился очередной запрос http_requester, независимо от кода успешности

public slots:
    void slotSendHttpReq(); //выполнить запрос цен
    //void slotEnableControls(bool);
    //void slotBreakUpdating();


signals:
    void signalEnableControls(bool);
    void signalTabPageChanged(int);
    void signalTabsPricesUpdate(); // имитится после упешно полученного ответа от http_requester

};



#endif // CentralWidgetV3

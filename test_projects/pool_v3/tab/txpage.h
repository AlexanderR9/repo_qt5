#ifndef TXTABPAGE_H
#define TXTABPAGE_H

#include "basetabpage_v3.h"


class QJsonObject;
class DefiTxLogger;
struct TxLogRecord;
class QTimerEvent;



//DefiTxTabPage
//страница дял отображения информации о транзакциях.
// объект DefiTxLogger хранит в себе записи транзакций, а так же загружает/выгружает их в файлы на диске.
class DefiTxTabPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiTxTabPage(QWidget*);
    virtual ~DefiTxTabPage() {}

    virtual void sendUpdateDataRequest() {}; //срабатывает по нажатию пользователем кнопки в тулбаре
    virtual void setChain(int);

    void autoCheckStatusLastTx(); // после выполнения транзакции и задержки после нее, необходимо автоматом проверить статус выполнения

protected:
    DefiTxLogger    *m_logger; //manager tx records
    bool  m_autoMode;   //признак того что выполнение проверки статуса происходит в автоматическом режиме а не из меню вручную.

    void initTable();
    void reloadTables();
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void updateRowColor(int); // обновить цвета  некоторых ячеек строки (tx_kind/status)
    void updateTotalTable(); //обновить поля обобщенной таблицы
    void setRecordIcon(int, const TxLogRecord&); // обновить тултип и иконку для типа транзакции

    // после получения статуса транзакции и обновления самой записи в m_logger необходимо обновить соответствующую строку в таблице
    void updateTableRowByRecord(const QString&);

    // после выполнения транзакции и проверки ее статуса необходимо проанализировать полученный результат.
    //если транзакция завершилась с успехом, то перейти обратно на соотвутствующую страницу и обновить ее.
    void analyzeStatusLastTx();

    void timerEvent(QTimerEvent*);

public slots:
    void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs

    // выполняется после успешного получения в ответе tx_hash на любой другой странице.
    // теперь эту запись необходимо добавить в лог-файлы tx_*.txt
    void slotNewTx(const TxLogRecord&);

    // слоты, которые выполняются по сигналам страницы DefiStatPosPage
    void slotSetTxHashHistory(QStringList&); // получить историю всех хешей для этой сети и только успешно-выполненные
    void slotSetTxLogger(const DefiTxLogger*&);


protected slots:
    void slotTxStatus();


signals:
    void signalStartTXDelay(QString); //запустить диалоговое окно для блокировки интерфейса на определенную задержку
    void signalUpdatePageBack(QString req_name, QString extra_data = QString()); // после анализа результа последней транзакции отправить сигнал на обновление страницы с которой отправилась транзакция


};




#endif // TXTABPAGE_H

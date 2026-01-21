#ifndef STATPOSPAGE_H
#define STATPOSPAGE_H


#include "basetabpage_v3.h"

#include <QMap>

struct TxLogRecord;
class DefiTxLogger;


// страница отображает результаты закрытых и открытых позиций.
// на странице ничего не запрашивается в сети, работа только с локальными данными из памяти.
// весь алгоритм выполняется по кнопке 'Update'.
// 1. страница запрашивает список всех хешей всех транзакций для этой сети (только со статусом "ОК")
// 2. перебирает все транзакции (по дате с конца) и создает строки для записей типа mint и increase.
// 3. снова перебирает все транзакции (по дате с конца) и фиксирует результаты закрытых поз т.е. в строках для записей типа take_away и decrease.



//DefiStatPosPage
class DefiStatPosPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiStatPosPage(QWidget*);
    virtual ~DefiStatPosPage() {}

    //выполняется запрос к сети(read only) для обновления данных на странице.
    //выполняется когда пользователь в тулбаре нажимает кнопку "Update"
    void sendUpdateDataRequest();


protected:
    QStringList m_txHashList; // история хешей всех транзакций для этой сети (только со статусом "ОК")

    void initTable();
    void checkTx(const TxLogRecord*);
    void checkClosedTx(const TxLogRecord*);
    void checkOpenedPositions();
    void checkOpenedPosition(int, const QStringList&);
    void updateOpenedPosition(int, int, const QStringList&);

    QString strPriceRange(const TxLogRecord*) const;
    QString strTickRange(const TxLogRecord*) const;
    QString strDepositedAssets(const TxLogRecord*) const;
    QString strClosedAssets(const TxLogRecord*) const; // возращает количество активов выведенных при закрытии позы (без учета rewards)
    QString strClosedRewards(const TxLogRecord*) const; // возращает количество активов полученных от сбора комиссий при закрытии позы (rewards)
    QString strNestedUserAmount(const TxLogRecord*) const; // возращает сумму вложенных активов в одном токене (согласно prior_data) на момент транзакции
    QString strClosedUserRewards(const TxLogRecord*) const; // возращает сумму полученных комиссий в одном токене (согласно prior_data) на момент закрытия
    QString strStartExitPrices(const TxLogRecord*, int) const; // возращает возвращает пару цен (для привычного токена), на момент открытия и на момент закрытия позы

    // рассчитать итоговую дохоходность для указанной позы (закрытой).
    // рассчет проводится как отношение суммы полученных наград (в приоритетном токене) на момент закрытия к
    // вложенной сумме (в приоритетном токене) на момент открытия.
    // примечание: функция применима и к открытой позе
    void calcClosedYield(int row);


    // рассчитать итоговый результат для указанной позы (закрытой) в процентах.
    // рассчет проводится как абсолютная величина прироста вложенных средств (в приоритетном токене) на момент открытия, с учетом rewards.
    // result = (closed_assets + rewards) - nested_assets, годовая доходность не влияет.
    // значение может быть отрицательным, т.е. цена изменилась так что был получен убыток (в приоритетном токене) на момент закрытия.
    // значение в таблице будет указано в процентах.
    void calcClosedTotalResult(int row);
    void calcOpenedTotalResult(int row); // аналогична calcClosedTotalResult но применяется к открытым позам

    int findRowClosedPosision(const TxLogRecord*) const; // найти в таблице строку по указанной закрытой позиции

private:
    // функция возвращает общую сумму обоих токенов в приоритетном токене
    // из соответствующей строки в указанной во 2-м параметре ячейке.
    // 2-й параметр может принимать значения:  nested/closed/reward/assets
    float userTokenSum(int, QString) const;

    // функция возвращает интервал в днях (дробное число) между моментом открытия и закрытия указанной позы.
    float lastingDays(int) const;

    // обновляет в указанной ячекйке toolTip, сумму в приоритетном токене в открытой позе
    void updatePriorTokenOpenedPosition(int, int, const QStringList&);


public slots:
    void slotNodejsReply(const QJsonObject&) {} //получен успешный ответ от скрипта nodejs

signals:
    void signalGetTxHashHistory(QStringList&);
    void signalGetTxLogger(const DefiTxLogger*&);
    void signalGetOpenedPosState(QMap<int, QStringList>&); // получить инфу по текущим открытым позам со страницы DefiPositionsPage

};




#endif // STATPOSPAGE_H

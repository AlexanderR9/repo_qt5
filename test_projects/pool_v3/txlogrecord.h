#ifndef TXLOGRECORD_H
#define TXLOGRECORD_H


#include <QDateTime>
#include <QPair>


//структура для хранения одной log-записи.
//данные структуры хранятся в трех файлах - data/tx_list.txt, data/tx_state.txt, data/tx_details.txt
//типы транзакций(tx_kind) могут быть следующие:
//1. wrap  (оборачивание нативного токена в завернутый)
//2. unwrap  (обратная процедура операции wrap)
//3. approve
//4. transfer  (передача любого токена кошелька на другой кошелек)
//5. mint (чеканка новой позы)
//6. increase (добавление ликвидности в существующую позу)
//7. decrease (перевод части/всей ликвидности указанной позы в раздел невостребованные комиссии этой позы)
//8. collect (вывод токенов-комиссий у заданной позы на кошелек)
//9. swap (обмен токенов один на другой в кошельке используя конкретный пул)
//10. burn сжигание существующей позы (без ликвидности)
//набор полей для каждого типа индивидуальный.
struct TxLogRecord
{
    TxLogRecord() {reset();}
    TxLogRecord(QString type, QString chain); //при вызове этого конструктора автоматом устанавливается текущее время

    // структура для хранения инфы о статусе транзакции и уплаченном газе
    struct TxStatusLog
    {
        TxStatusLog() {reset();}
        QString result; // OK/FAULT/UNKNOWN
        quint32 gas_used; // потрачено единиц газа
        float fee_coin; // уплачено комисси в родных единицах сети
        float fee_cent; // уплачено комисси в USD-центах
        void reset() {result="UNKNOWN"; gas_used=0; fee_coin=fee_cent=-1;}
    };
    // структура для хранения инфы по транзакциям связанными с токенами кошелька:  wrap/unwrap/approve/transfer
    struct TxWalletLog
    {
        TxWalletLog() {reset();}
        QString token_addr; //актив с которым происходит действие
        QString contract_addr; // используется только при approve
        QString target_wallet; // используется только при transfer
        float token_amount;
        void reset() {token_addr = contract_addr = target_wallet = "0x0"; token_amount=-1;}
    };
    // структура для хранения инфы по транзакциям связанными с операциями в пуле:  mint/increase/decrease/collect/swap/take_away
    struct TxPoolLog
    {
        TxPoolLog() {reset();}
        QString pool_addr; //адрес пула
        quint32 pid;
        int tick; //current tick of pool
        float price; //current price of pool
        QPair<int, int> tick_range;
        QPair<float, float> price_range;
        QPair<float, float> token_sizes;
        QPair<float, float> reward_sizes;
        QString token_in; //адрес токена который меняется при свопе
        void reset()
        {
            pool_addr = token_in = "0x0";
            pid = tick = tick_range.first = tick_range.second = 0;
            price = -1;
            price_range.first = price_range.second = 0;
            token_sizes.first = token_sizes.second = 0;
            reward_sizes.first = reward_sizes.second = 0;
        }
    };

    //основные поля
    QString tx_hash; // уникальный хеш транзакции
    QString tx_kind;  // тип транзакции (кодовое слово)
    QString chain_name;
    QDateTime dt; //дата и время совершения транзакции
    QString note; //примечание в свободной форме

    TxStatusLog status;
    TxWalletLog wallet;
    TxPoolLog pool;

    //funcs
    void reset();
    bool invalid() const;
    void setDateTime(QString, QString);
    QString strDate() const;
    QString strTime() const;
    void parseDetails(const QString&); // извлечь поля детализации записи из соответствующей части строки файла tx_details.txt
    void parseDetail(QString, QString); // извлечь конкретное поле детализации
    void parseStatus(const QStringList&); // загрузить данные статуса из строки файла tx_state.txt
    void formNote(QString); // сформировать поле note

    // finished status funcs
    bool resultOk() const; // транзакция завершилась успешно
    bool resultFault() const; // транзакция завершилась НЕ успешно
    bool isFinishedStatus() const; // транзакция завершилась (не важно с каким результатом)

    QString listFileLine() const;
    QString statusFileLine() const;
    QString detailsFileLine() const;

private:
    void parseFloatPair(QString, QPair<float, float>&);

};




#endif // TXLOGRECORD_H

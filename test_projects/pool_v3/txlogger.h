#ifndef TXLOGGER_H
#define TXLOGGER_H


#include "lsimpleobj.h"


#include <QDateTime>
#include <QPair>

//класс для ведения журнала записей о совершенных транзакциях.
//объект класса добавляет записи в файл, а также извлекает их оттуда  по запросу.
//сведенья о транзакциях размещаются в 3-х файлах, создавая что-то вроде БД с общим полем tx_hash.
// Файл_1. data/tx_list.txt - основной файл с перечнем всех транзакций, отправленных в сеть.
//    в этот файл записи попадают сразу после отправки транзакции в сеть и получения tx_hash.
//    записи в файле имеют вид: tx_hash / date / time / chain_name / tx_kind
// Файл_2. data/tx_state.txt - файл с информацией о состоянии транзакции и уплаченной стоимости/количества газа,
//   в этот файл записи заносятся после получения статуса любой транзакции, причем транзакция должна быть завершена (с результатом OK или FAULT).
//   пока транзакция выполняется, она не попадет в этот файл.
//   состояние любой транзакции можно запросить в любой момент в пользовательском интервейсе,
//   после получения из сети ее состояния в файл добавится новая запись либо перепишется, уже существующая с таким tx_hash
//   записи в файле имеют вид: tx_hash / STATUS(OK/FAULT) / gas_used / gas_fee_coin / gas_fee_usd_cents
// Файл_3. data/tx_details.txt - файл с подробностями транзакции, эта информация содержит дополнительную информацию, чисто для истории.
// в этот файл записи попадают сразу после отправки транзакции в сеть и получения tx_hash.
//    записи в файле имеют вид: tx_hash / (набор полей, соответствующий tx_kind, через ';')
//  поле note присутствует во всех случаях и содержит просто поясняющий текст.
//  поле current_price присутвует там где участвует пул т.е. поле 'pool_addr'. Так как пул имеет 2 цены в любой момент времени, в конфиге предварительно должна быть описана секция,
//          в которой указаны для какого токена рисовать цену в единицах 2-го для конкретной пары, если не указано то цена рисуется для token0 в единицах token1.
//   комбинации полей при каждом типе транзакции:
//  - wrap/unwrap: token_addr[0xvalue]; token_amount[value] (т.е. адрес токена и объем который был врапнут)
//  - approve: token_addr[0xvalue]; token_amount[value]; to_contract[0xvalue] (т.е. адрес токена и объем который был апрувнут и адрес контракта для кого апрувнут)
//  - transfer: token_addr[0xvalue]; token_amount[value]; to_wallet[target wallet_addr] (т.е. адрес токена и объем, адрес кошелька, на который отправили токены)
//  - swap: pool_addr[0x_value]; token_in[0xvalue]; token_amount[value]; current_price[value] (адрес пула в котором меняем, входной токен, который отдаем и сколько отдаем, текущая цена)
//  - mint: pool_addr[0x_value]; current_price[value]; tick[cur_tick]; token_sizes[value0:value1]; pid[-1]; tick_range[t_low:t_high]; price_range[p_low:p_high]
//            примечание: pid можно получить только после создании позы, в следующем запросе
//  - increase/decrease/collect:  pid[value]; pool_addr[0x_value]; current_price[value]; tick[cur_tick]; token_sizes[value0:value1]; tick_range[t_low:t_high]; price_range[p_low:p_high]
//  - burn: pid[value]; pool_addr[0x_value]; tick[cur_tick]; tick_range[t_low:t_high];




//структура для хранения одной log-записи.
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
struct DefiTxLogRecord
{
    DefiTxLogRecord() {reset();}
    DefiTxLogRecord(QString type, QString chain); //при вызове этого конструктора автоматом устанавливается текущее время

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
        QString contract_addr; // используется только при апруве
        QString target_wallet; // используется только при transfer
        float token_amount;
        void reset() {token_addr = contract_addr = target_wallet = "0x0"; token_amount=-1;}
    };
    // структура для хранения инфы по транзакциям связанными с операциями в пуле:  mint/increase/decrease/collect/swap/burn
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
        QString token_in; //адрес токена который меняется при свопе
        void reset()
        {
            pool_addr = token_in = "0x0";
            pid = tick = tick_range.first = tick_range.second = 0;
            price = -1;
            price_range.first = price_range.second = 0;
            token_sizes.first = token_sizes.second = 0;
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
    bool invalid();
    void setDateTime(QString, QString);


    /*
    QString pool_address; // используется для (mint, increase, decrease, collect, swap)
    QString token_address; // указывает токен, с которым совершается действие , используется для (wrap, unwrap, approve, transfer, swap)

    quint8 line_step; //текущий шаг в цепочке итераций по конкретному пулу, для отслеживания убытков/прибыли, значение 0 не имеет значение
    quint64 pid; // используется для (increase, decrease, collect)

    // при операции swap, чтобы понять какой токен был входной нужно посмотреть token_address
    // при операции swap: token0_size - отдали входного токена,  token1_size - получили выходного токена
    float token0_size;
    float token1_size;

    // при операции swap здесь указывается цена входного токена в единицах выходного, т.е. цена по которой продаем входной токен
    float current_price;


    int current_tick;
    QString tick_range; // используется для (mint, increase, decrease, collect)
    QString price_range;  // используется для (mint, increase, decrease, collect)
    QString note; //для произвольной вспомогательной информации

    void reset();
    QString toFileLine() const;
    void fromFileLine(const QString&);
    bool invalid() const;

    void fromPriceRange(float&, float&) const; //попытаться вытащить вещественные значения из переменной price_range
    */

};




//DefiTxLogger
//объект для управления записями совершенных транзакций.
//при инициализации сразу пытается загрузить записи транзакций из файлов.
class DefiTxLogger : public LSimpleObject
{
    Q_OBJECT
public:
    DefiTxLogger(QString, QObject*);
    virtual ~DefiTxLogger() {}

    void reloadLogFiles(); //загрузить данные в группы файлов tx_*.txt
    virtual QString name() const {return QString("defi_tx_logger");}

    inline const DefiTxLogRecord& recordAt(int i) const {return m_logData.at(i);}
    inline int logSize() const {return m_logData.count();}
    inline bool logEmpty() const {return m_logData.isEmpty();}

protected:
    QString m_chain; //название текущей сети
    QList<DefiTxLogRecord> m_logData; //контейнер для загрузки всех записей из файла txLogFile

    void loadTxListFile(); //load tx_list.txt
    void loadTxStateFile(); //load tx_state.txt
    void loadTxDetailsFile(); //load tx_details.txt


    /*
public slots:
    void slotAddLog(const JSTxLogRecord&); // добавить запись о транзакции в лог файл txLogFile

signals:
    void signalStartTXDelay();
    */


};




#endif // TXLOGGER_H



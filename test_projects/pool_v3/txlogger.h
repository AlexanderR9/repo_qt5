#ifndef TXLOGGER_H
#define TXLOGGER_H


#include "lsimpleobj.h"
#include "txlogrecord.h"


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

// Файл_3. data/tx_details.txt - файл с подробностями транзакции, содержит дополнительную информацию о параметрах транзакции (чисто для истории).
// в этот файл записи попадают сразу после отправки транзакции в сеть и получения tx_hash.
// ПОСЛЕ ПОЛУЧЕНИЯ СТАТУСА запись в файле обновляется.
//    записи в файле имеют вид: tx_hash / status / (набор дополнительных полей, соответствующий tx_kind, через ';')
//  дополнительное поле note присутствует во всех случаях и содержит просто поясняющий текст.
//  дополнительное поле current_price присутвует там где участвует пул т.е. поле 'pool_addr'. Так как пул имеет 2 цены в любой момент времени, в конфиге предварительно должна быть описана секция,
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




//DefiTxLogger
//объект для управления записями совершенных транзакций.
//при инициализации сразу пытается загрузить записи транзакций из файлов.
class DefiTxLogger : public LSimpleObject
{
    Q_OBJECT
public:
    DefiTxLogger(QString, QObject*);
    virtual ~DefiTxLogger() {}


    void reloadLogFiles(); //загрузить данные в m_logData из группы файлов tx_*.txt
    void addNewRecord(const TxLogRecord&); //добавить инфу в лог файлы новую запись, кроме tx_state.txt т.к. статус выполнения еще неизвестен
    void updateRecStatus(const QString&, QString, float, quint32); // обновить статус записи по указанному хешу
    const TxLogRecord* recByHash(const QString&) const;

    virtual QString name() const {return QString("defi_tx_logger");}

    inline const TxLogRecord& recordAt(int i) const {return m_logData.at(i);}
    inline int logSize() const {return m_logData.count();}
    inline bool logEmpty() const {return m_logData.isEmpty();}

protected:
    QString m_chain; //название текущей сети
    QList<TxLogRecord> m_logData; //контейнер для загрузки всех записей из файла txLogFile

    void loadTxListFile(); //load tx_list.txt
    void loadTxStateFile(); //load tx_state.txt
    void loadTxDetailsFile(); //load tx_details.txt

    // после получения обновленной инфы по некоторой записи (например status или PID position) .
    // эти правки нужно внести в соответствующих строках в файлах tx_state.txt / tx_details.txt (т.е. просто переписать их).
    // т.е. предполагается что базовая инфа уже была добавлена ранее в tx_list.txt / tx_details.txt.
    void rewriteStatusFiles();

};




#endif // TXLOGGER_H



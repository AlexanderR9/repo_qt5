#ifndef JSTXLOGGER_H
#define JSTXLOGGER_H


#include "lsimpleobj.h"


#include <QDateTime>


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
//набор полей для каждого типа индивидуальный.
struct JSTxLogRecord
{
    JSTxLogRecord() {reset();}
    JSTxLogRecord(QString type, QString chain); //при вызове этого конструктора автоматом устанавливается текущее время

    QString tx_hash;
    QString tx_kind;
    QString chain_name;
    QDateTime dt; //дата и время совершения транзакции
    QString pool_address; // используется для (mint, increase, decrease, collect, swap)
    QString token_address; // указывает токен, с которым совершается действие , используется для (wrap, unwrap, approve, transfer, swap)

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

};


//класс для ведения журнала записей о совершенных транзакциях.
//объект класса добавляет записи в файл, а также извлекает их оттуда  по запросу.
//UG_BasePage
class JSTxLogger : public LSimpleObject
{
    Q_OBJECT
public:
    JSTxLogger(QObject*);
    virtual ~JSTxLogger() {}

    virtual QString name() const {return QString("jstx_logger");}


    inline void setChainName(QString s) {m_currentChain = s.trimmed();}
    inline QString chainName() const {return m_currentChain;}


    static QString txLogFile() {return QString("defi/tx_history.log");}

protected:
    QString m_currentChain; //название текущей сети

public slots:
    void slotAddLog(const JSTxLogRecord&); // добавить запись о транзакции в лог файл txLogFile


};

#endif // JSTXLOGGER_H

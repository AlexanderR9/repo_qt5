#ifndef WALLETBALANCELOGGER_H
#define WALLETBALANCELOGGER_H



#include "lsimpleobj.h"

#include <QDateTime>


//WalletBalanceLogger
//объект для отслеживания изменения балансов токенов кошелька и ведения журнала.
//объект следит за балансами всех токенов кроме нативного.
//при изменении цены более чем на m_sensivity(%) записывает запись в файл balance_history.log.
// записи в файле имеют вид: token_name / chain_name / balance / <datetime> в виде одного целого числа
//при инициализации загружает файл журнала balance_history.log и выводит на страницу истории балансов.
class WalletBalanceLogger : public LSimpleObject
{
    Q_OBJECT
public:
    struct TokenBalanceRecord
    {
        TokenBalanceRecord() {reset();}
        TokenBalanceRecord(QString a) {reset(); chain = a.trimmed().toLower();}
        QString name;
        QString chain;
        float balance;
        quint32 time_point;
        void reset() {name.clear(); chain.clear(); balance=0; time_point=0;}
        QString fileLine() const; // выдать данные записи в виде строки файла логов
        QDateTime toDT() const {return QDateTime::fromSecsSinceEpoch(time_point);}
    };

    WalletBalanceLogger(const QStringList&, QObject*);
    virtual ~WalletBalanceLogger() {}

    void loadLogFile(QString); // загрузить при старте ПО всю историю в m_log
    float recBalanceDeviation(int) const; // посчитать абсолютное значение отклонения для указанной записи
    void receiveNewValue(const QString&, float); // получено текущее значение баланса из сети, необходимо его проанализировать
    void setSensivityChainging(float); // задать чувствительнось к изменениям баланса в процентах

    inline int logSize() const {return m_log.count();}
    inline const TokenBalanceRecord& recAt(int i) const {return m_log.at(i);}

protected:
    float m_sensivity; // чувствительность при привышении которой будет запись в журнал, %
    QString m_chain;
    QList<TokenBalanceRecord> m_log;
    const QStringList &m_trackingTokens; // названия отслеживаемых токенов

    void initEmptyFile(); // файл логов еще не был создан, необходимо создать пустой файл
    QString balanceFileName() const; // полное имя файла логов балансов
    void parseFileData(const QStringList&); //файл логов был успешно прочитан, теперь необходимо распасить его содержимое и поместить в m_log
    void checkAbsentBalances(); //проверить отсутствие записей по всем токена, если такие будут найдены то создать 1-ю с нулевым балансом
    float lastBalance(QString) const; //возвращает последнее(саммое сввежее) значение указанного токена, или -1
    void addNewRecToFile(const QString&, float);

signals:
    void signalAddNewRecord(float);

};



#endif // WALLETBALANCELOGGER_H


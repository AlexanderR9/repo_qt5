#ifndef WALLETBALANCEHISTORY_H
#define WALLETBALANCEHISTORY_H

#include "lsimpleobj.h"

#include <QDateTime>
#include <QList>
#include <QMap>

//WalletBalanceHistory
class WalletBalanceHistory : public LSimpleObject
{
public:
    struct SnapshotPointAsset
    {
        SnapshotPointAsset() {reset();}
        SnapshotPointAsset(QDateTime dt, const QString &addr, double b) :time(dt),token_address(addr),balance(b) {}

        //vars
        QDateTime time;
        QString token_address;
        double balance;
        QString chain;

        //funcs
        void reset() {time = QDateTime(); token_address.clear(); balance = -1; chain.clear();}
        void fromFileLine(const QString&);
        QString toFileLine() const;
        bool invalid() const;
        QString toStr() const;
        void setData(const SnapshotPointAsset&);

        static QString timeMask() {return QString("yyyy_MM_dd.hh:mm:ss");}
    };


    WalletBalanceHistory(QObject *parent = NULL);
    virtual ~WalletBalanceHistory() {}

    //каждый раз когда происходит запрос балансов кошелька из сети(конкретной), нужно затем выполнить эту функцию.
    //здесь происходит сравнение текущих реальных балансов полученных только что, с теми что в m_lastSnapshots.
    //если будут замечены отличия в величинах более чем на 0,01% то соответствующие записи будут замененны в m_lastSnapshots,
    //а так же они будут добавлены в файл истории JS_BALANCE_FILE c текущей датой.
    void updateBalances(const QMap<QString, float>&, QString);

protected:
    //последние значения балансов активов всех цепочек, самые свежие значения из истории, по одному на каждый актив.
    //для каждой пары: (адреса актива/сеть)  может быть только одна запись в этом контейнере.
    QList<SnapshotPointAsset> m_lastSnapshots;

    QString m_currentChain; //название сети в которой идет работа


    //при создании объекта сразу загружается истории из файла JS_BALANCE_FILE.
    //m_lastSnapshots заполняется самыми свежими значениями для каждого актива.
    //одна запись в файле имеет вид : chain_name / date.time / token_address / balance_value
    void loadHistoryFile();

    //проверить если эта запись более свежая, то заменить ее в m_lastSnapshots на соответствующую.
    //здесь проверяются именно даты а не значения балансов.
    void initLastRecord(const SnapshotPointAsset&);

    //вернет последнее значение баланса для указанного актива(взятое из m_lastSnapshots) или -1
    //float lastAssetBalance(const QString&) const;

    //вернет индекс в m_lastSnapshots для указанной пары актив/сеть или -1
    int findAssetRec(const QString&, QString) const;

    //добавить обнаруженные более свежие точки балансов в файл истории JS_BALANCE_FILE c текущей датой.
    void appendFreshPointToFile(const QList<SnapshotPointAsset>&);

    //сравнить текущий баланс указанного токен с тем что лежит в m_lastSnapshots.
    //если есть отличие, то обновить соответствующую запись в m_lastSnapshots и добавить ее в контейнер для последующего добавления в файл истории.
    void compareTokenBalances(const QString&, double, QList<SnapshotPointAsset>&);

private:
    void out(); //diag func

};

#endif // WALLETBALANCEHISTORY_H

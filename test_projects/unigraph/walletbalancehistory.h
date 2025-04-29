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
        SnapshotPointAsset(QDateTime dt, const QString &addr, float b) :time(dt),token_address(addr),balance(b) {}

        //vars
        QDateTime time;
        QString token_address;
        float balance;

        //funcs
        void reset() {time = QDateTime(); token_address.clear(); balance = -1;}
        void fromFileLine(const QString&);
        QString toFileLine() const;
        bool invalid() const;
        QString toStr() const;
        void setData(const SnapshotPointAsset&);

        static QString timeMask() {return QString("yyyy_MM_dd.hh:mm:ss");}
    };


    WalletBalanceHistory(QObject *parent = NULL);
    virtual ~WalletBalanceHistory() {}

    //каждый раз когда происходит запрос балансов кошелька из сети, нужно затем выполнить эту функцию.
    //здесь происходит сравнение текущих реальных балансов полученных только что, с теми что в m_lastSnapshots.
    //если будут замечены отличия в величинах более чем на 0,01% то соответствующие записи будут замененны в m_lastSnapshots,
    //а так же они будут добавлены в файл истории JS_BALANCE_FILE c текущей датой.
    void updateBalances(const QMap<QString, float>&);

protected:
    //последние значения балансов активов, т.е. самые свежие из истории.
    //для каждого адреса актива может быть только одна запись в этом контейнере.
    QList<SnapshotPointAsset> m_lastSnapshots;

    //при создании объекта сразу загружается истории из файла JS_BALANCE_FILE.
    //m_lastSnapshots заполняется самыми свежими значениями.
    //одна запись в файле имеет вид : date.time / token_address / balance_value
    void loadHistoryFile();

    //проверить если эта запись более свежая, то заменить ее в m_lastSnapshots на соответствующую.
    //здесь проверяются именно даты а не значения балансов.
    void initLastRecord(const SnapshotPointAsset&);

    //вернет последнее значение баланса для указанного актива(взятое из m_lastSnapshots) или -1
    float lastAssetBalance(const QString&) const;

    //вернет индекс в m_lastSnapshots для указанного актива или -1
    int findAssetRec(const QString&) const;

    //добавить обнаруженные более свежие точки балансов в файл истории JS_BALANCE_FILE c текущей датой.
    void appendFreshPointToFile(const QList<SnapshotPointAsset>&);

    //сравнить текущий баланс указанного токен с тем что лежит в m_lastSnapshots.
    //если есть отличие, то обновить соответствующую запись в m_lastSnapshots и добавить ее в контейнер для последующего добавления в файл истории.
    void compareTokenBalances(const QString&, float, QList<SnapshotPointAsset>&);

private:
    void out(); //diag func

};

#endif // WALLETBALANCEHISTORY_H

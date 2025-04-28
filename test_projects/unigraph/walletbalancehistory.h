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
        QDateTime time;
        QString token_address;
        float balance;
        void reset() {time = QDateTime(); token_address.clear(); balance = -1;}
        void fromFileLine(const QString&);
        QString toFileLine() const;
        bool invalid() const;
        QString toStr() const;

        static QString timeMask() {return QString("yyyy_MM_dd.hh:mm:ss");}
    };


    WalletBalanceHistory(QObject *parent = NULL);
    virtual ~WalletBalanceHistory() {}

    //каждый раз когда происходит запрос балансов кошелька из сети, нужно затем выполнить эту функцию
    void updateBalances(const QMap<QString, float>&);

protected:
    //последние значения балансов активов, т.е. самые свежие из истории.
    //для каждого адреса актива может быть только одна запись в этом контейнере.
    QList<SnapshotPointAsset> m_lastSnapshots;

    //при создании объекта сразу загружается истории из файла JS_BALANCE_FILE.
    //m_lastSnapshots заполняется самыми свежими значениями.
    //одна запись в файле имеет вид : date.time / token_address / balance_value
    void loadHistoryFile();
    void initLastRecord(const SnapshotPointAsset&); //проверить если эта запись более свежая, то заменить ее в m_lastSnapshots на соответствующую

    //вернет последнее значение баланса для указанного актива(взятое из m_lastSnapshots) или -1
    float lastAssetBalance(const QString&) const;

    //вернет индекс в m_lastSnapshots для указанного актива или -1
    int findAssetRec(const QString&) const;

    //добавить обнаруженные более свежие точки балансов
    void appendFreshPointToFile(const QList<SnapshotPointAsset>&);

private:
    void out(); //diag func

};

#endif // WALLETBALANCEHISTORY_H

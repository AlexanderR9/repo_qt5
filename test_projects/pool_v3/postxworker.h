#ifndef POSTXWORKER_H
#define POSTXWORKER_H

#include "lsimpleobj.h"

class QWidget;
class QTableWidget;
struct DefiPosition;
struct TxDialogData;
class QJsonObject;
struct TxLogRecord;
class DefiMintDialog;




// PosTxWorker
//класс для формирования параметров для отправки транзакций связанных с позициями пулов.
//increase/decrease/collect/burn/mint
class PosTxWorker : public LSimpleObject
{
    Q_OBJECT
public:
    struct LastTxInfo
    {
        LastTxInfo() {reset();}
        int tx_kind; // тип последней отправленной транзакции (элемент NodejsTxCommand)
        int t_row;  // для типов collect/increase/decrease это номер строки в таблице, т.е. выделенная поза
        int pid; // для типов collect/increase/decrease это PID позиции с которой совершается действие
        bool need_reupdate; // признак что после успешной транзакции необходимо переобновить страницу
        bool mint_activated; // запущен диалог MINT
        void reset() {tx_kind = t_row = pid = -1; need_reupdate = mint_activated =false;}
        inline bool invalid() const {return (tx_kind < 0);}
    };

    PosTxWorker(QObject*, int, QTableWidget*);
    virtual ~PosTxWorker() {}

    inline int chainId() const {return userSign();}
    inline int lastTx() const {return m_lastTx.tx_kind;}
    inline int lastPosPid() const {return m_lastTx.pid;}
    inline bool needReupdatePage() const {return m_lastTx.need_reupdate;}
    inline void setReupdatePage(bool b) {m_lastTx.need_reupdate = b;}
    inline bool mintDialogActivated() const {return m_lastTx.mint_activated;}


    // подготовить параметры и отправить транзакцию указанного типа (элемент NodejsTxCommand)
    void tryTx(int, const QList<DefiPosition>&);

    // была отправлена реальная транзакция, теперь необходимо сформировать соответствующую запись и отправить в журнал
    void prepareTxLog(const QJsonObject &js_reply, const QList<DefiPosition>&);

    // подготовить поле note для лога последней транзакции
    QString extraDataLastTx(const QJsonObject &js_reply) const;

    // for mint operation
    void poolStateReceived(const QStringList&); // после запроса на обновление состояния пула пришел ответ от nodejs на страницу pools
    void emulMintReply(const QJsonObject &js_reply); // после отправки транзакции типа 'mint' в режиме эмуляции пришел ответ от скрипта nodejs

protected:
    QTableWidget *m_table; // указатель на таблицу со страницы DefiPositionsPage
    LastTxInfo m_lastTx; // инфо о последней отправленной транзакции
    const DefiMintDialog *m_mintDialog;


    QWidget* parentWidget() const; // указатель страницу-родителя DefiPositionsPage
    QString chainName() const; // название текущей сети
    void fillTxLogRecord(TxLogRecord&, const QJsonObject&, const DefiPosition&); // заполнить соответствующие поля записи лога последней тразакции
    void fillTxMintLogRecord(TxLogRecord&, const QJsonObject&); // заполнить соответствующие поля записи лога последней тразакции (txMint)
    int tableRowIndexOf(int) const; // поиск строки c указанным pid

    void burnSelected(const QList<DefiPosition>&); // сжечь выделенные позиции (может быть несколько), можно сжечь позы только без ликвидности и с полностью выведенными токенами
    void collectSelected(const QList<DefiPosition>&); // собрать rewards у выделенной одной позиции
    void decreaseSelected(const QList<DefiPosition>&); // удалить ликвидность у выделенной одной позиции (перенести активы в зону reward)
    void increaseSelected(const QList<DefiPosition>&); // добавить ликвидность к указанной позе
    void takeawaySelected(const QList<DefiPosition>&); // вывести всю ликвидность у выделенной одной позиции (перенести на кошелек)
    void mintPos(); // чеканка новой позы


protected slots:
    void slotEmulateMint(const TxDialogData&);

signals:
    void signalGetPosIndexByPid(int, int&); // запросить у страницы-родителя индекс позиции в контейнере m_positions по ее PID
    void signalSendTx(const TxDialogData&); // отправить команду в nodejs_bridge для запроса на запись транзакции в сеть
    void signalNewTx(const TxLogRecord&); // отправляется странице DefiTxTabPage после успешного получения в ответе tx_hash
    void signalGetTokenBalance(QString, float&) const;  //запрос текущего баланса токена в кошельке по его тикеру.
    void signalTryUpdatePoolState(const QString&); // запрос текущего состояния пула


};



#endif // POSTXWORKER_H

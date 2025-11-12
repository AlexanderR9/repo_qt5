#ifndef POSITIONSPAGE_H
#define POSITIONSPAGE_H

#include "basetabpage_v3.h"
#include "defiposition.h"


class QJsonObject;
class QJsonValue;
class PosTxWorker;



//DefiPositionsPage
class DefiPositionsPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiPositionsPage(QWidget*);
    virtual ~DefiPositionsPage() {}

    virtual void sendUpdateDataRequest();
    virtual void setChain(int);
    virtual void updatePageBack(QString); // выполняется после анализа результа последней транзакции, происходит переход обратно на страницу и обновление ее.

    inline int posCount() const {return m_positions.count();}
    inline bool hasPos() const {return (posCount() > 0);}

    void mintPos(); // запустить диалог для чеканки позы


protected:
    QList<DefiPosition> m_positions;
    PosTxWorker *m_txWorker;


    void initTable();
    void updatePositionsData(const QJsonObject&);
    void updatePositionsState(const QJsonObject&);
    void updatePositionState(int, const QJsonValue&);
    void readNodejsPosFile(); // после успешного запроса позиций необходимо считать файл positions.txt
    void reloadTableByRecords(); //перезаписать таблицу после успешного запроса позиций и парсинга positions.txt
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    int posIndexOf(int) const; // поиск позиции по ее PID, вернет индекс в контейнере или -1
    void updateIntegratedTable();
    bool hasBalances() const;// проверка что балансы кошелька были получены
    void getPoolStateFromPoolPage(const QJsonObject&);


    //check nodejs reply after TX
    void checkTxResult(QString, const QJsonObject&); // проанализировать ответ после попытки отправить очередную транзакцию


private:
    QString poolInfo(const DefiPosition&) const;

public slots:
    virtual void slotNodejsReply(const QJsonObject&); //получен успешный ответ от скрипта nodejs

protected slots:
    void slotGetSelectedPosState();
    void slotGetLiquidityPosState();
    void slotGetNoneLiqPosState();
    void slotGetPoolState(const QString&);

    void slotSetPosIndexByPid(int pid, int &j) {j = posIndexOf(pid);} // запросить у страницы-родителя индекс позиции в контейнере m_positions по ее PID
    void slotSendTx(const TxDialogData&); // транслировать команду от m_txWorker в nodejs_bridge

    // TX actions
    void slotBurnPosSelected(); // сжечь выделенные позиции, можно сжечь позы только без ликвидности и с полностью выведенными токенами
    void slotCollectPosSelected(); // собрать rewards у выделенной одной позиции
    void slotDecreasePosSelected(); // удалить ликвидность у выделенной одной позиции (перенести активы в зону reward)
    void slotTakeawayPosSelected(); // извлечь все активы у выделенной одной позиции (на кошелек)
    void slotIncreasePosSelected(); // добавить ликвидность в существующую указанную позицию

signals:
    void signalGetPoolStateFromPoolPage(const QString&, QStringList&);

};




#endif // POSITIONSPAGE_H


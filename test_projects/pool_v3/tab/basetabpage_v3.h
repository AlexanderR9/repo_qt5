#ifndef BASETABPAGE_V3_H
#define BASETABPAGE_V3_H


#include "lsimplewidget.h"

class QJsonObject;
struct TxLogRecord;
struct TxDialogData;




// BaseTabPage_V3
class BaseTabPage_V3 : public LSimpleWidget
{
    Q_OBJECT
public:
    BaseTabPage_V3(QWidget*, int, int);
    virtual ~BaseTabPage_V3() {}

    inline int kind() const {return userSign();}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    virtual void setChain(int);
    virtual QString curChainName() const {return userData();}


    // выполняется после анализа результа последней транзакции, происходит переход обратно на страницу и обновление ее.
    virtual void updatePageBack(QString) {}


    //выполняется запрос к сети(read only) для обновления данных на странице.
    //выполняется когда пользователь в тулбаре нажимает кнопку "Update"
    virtual void sendUpdateDataRequest() = 0;

protected:
    LTableWidgetBox     *m_table;
    LTableWidgetBox     *m_integratedTable;

    virtual void sendReadNodejsRequest(const QJsonObject&); // отаправить команду в nodejs_bridge для запроса на чтение из сети
    virtual void sendTxNodejsRequest(const TxDialogData&); // отаправить команду в nodejs_bridge для запроса на запись транзакции в сеть
    virtual void selectRowByCellData(const QString&, int col); //веделить строку со значение ячеики в указанном столбце
    virtual bool hasBalances() const {return false;} // проверка что балансы кошелька были получены
    virtual int tableRowByCellData(const QString&, int col) const; // вернет индекс строки таблицы, в которой значение ячеики в указанном столбце

public slots:
    virtual void slotNodejsReply(const QJsonObject&) = 0; //получен успешный ответ от скрипта nodejs

signals:
    void signalRewriteJsonFile(const QJsonObject&, QString);
    void signalRunNodejsBridge(QString, const QStringList&);
    void signalNewTx(const TxLogRecord&); // отправляется странице DefiTxTabPage после успешного получения в ответе tx_hash
    void signalGetTokenBalance(QString, float&) const;  //запрос текущего баланса токена в кошельке по его тикеру.

};

#endif // BASETABPAGE_V3_H



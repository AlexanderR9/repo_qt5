#ifndef JSPOOLTAB_H
#define JSPOOLTAB_H


#include "ug_basepage.h"

class LSearchTableWidgetBox;
class QJsonObject;
struct TxDialogData;
struct JSTxLogRecord;


//JSPoolRecord
struct JSPoolRecord
{
    JSPoolRecord() {reset();}

    QString address;
    quint16 fee;
    QString token0_addr;
    QString token1_addr;
    QString chain;
    QString assets; //pair assets, example LDO/USDC

    void reset();
    bool invalid() const;
    void fromFileLine(const QString&);
    QString strFee() const;
    int tickSpace() const;
    bool isStablePool() const; //признак что оба актива стейблы
    QString ticker0() const;
    QString ticker1() const;

};


//JSPoolTab
class JSPoolTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSPoolTab(QWidget*);
    virtual ~JSPoolTab() {}

    void loadPoolsFromFile(QString); //загрузить список пулов из файла node_js
    void parseJSResult(const QJsonObject&);

protected:
    LSearchTableWidgetBox       *m_table;
    QList<JSPoolRecord>         m_poolData;

    void initTable();
    void reloadTable();
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void answerState(const QJsonObject&);
    void answerSwap(const QJsonObject&);
    void answerMintTx(const QJsonObject&);
    void sendMintTx(const TxDialogData&, int);
    void sendTxRecordToLog(int, const QJsonObject&); //подготовить и отправить запись о выполненной транзакции в JSTxLogger для добавления в журнал

private:
    int pricePrecision(float, bool) const;

protected slots:
    void slotGetPoolState();
    void slotTrySwapAssets();
    void slotMintPosition();

public slots:
    void slotScriptBroken(); //скрипт не выполнился, произошла ошибка и ответ содержит поле 'error'
    void slotSetPoolInfo(const QString&, QString&); //выдать краткую информацию о пуле по ее адресу

signals:
    void signalPoolAction(const QStringList&);
    void signalResetApproved(const QString&); //при совершениии обмена необходимо отправить сигнал странице approve для сброса соответствующей записи
    void signalGetApprovedSize(QString, const QString&, float&); //запрос текущих опрувнутых токенов для свопа указанного актива
    void signalGetTokenPrice(const QString&, float&); //получить текущую цену токена со страницы кошелька
    void signalRewriteParamJson(const QJsonObject&);
    void signalSendTxLog(const JSTxLogRecord&);
    void signalGetChainName(QString&);


};




#endif // JSPOOLTAB_H

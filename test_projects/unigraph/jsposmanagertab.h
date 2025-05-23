#ifndef JSPOSMANAGER_TAB_H
#define JSPOSMANAGER_TAB_H

#include "ug_basepage.h"


class QJsonArray;
class QJsonObject;
struct TxDialogData;


//JSPosTab
class JSPosManagerTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSPosManagerTab(QWidget*);
    virtual ~JSPosManagerTab() {}

    void updatePidList(); //запросить из сети список всех PID (поз)
    void parseJSResult(const QJsonObject&); //проверить ответ полученный от  скриптов node_js


protected:
    LSearchTableWidgetBox     *m_tablePos;
    LSearchTableWidgetBox     *m_tableLog;

    void initTables();
    void reloadPidListToTable(const QJsonArray&);
    void updateCurrentPrices(); //запросить со страницы кошелька текущие цены активов

    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void sendIncreaseTx(const TxDialogData&, int);
    void sendDecreaseTx(const TxDialogData&, int);
    void sendCollectTx(const TxDialogData&, int);
    void sendTx(QString, int);
    void rereadJSPosFileData();

    //reseived node_js result
    void jsonPidListReceived(const QJsonObject&);
    void jsonPosFileDataReceived(const QJsonObject&);
    void jsonPosStateReceived(const QJsonObject&);
    void jsonTxIncreaseReceived(const QJsonObject&);
    void jsonTxDecreaseReceived(const QJsonObject&);
    void jsonTxCollectReceived(const QJsonObject&);

protected slots:
    void slotGetPositionState();
    void slotTryIncreaseLiquidity();
    void slotTryDecreaseLiquidity();
    void slotTryCollectTokens();

public slots:
    void slotScriptBroken(); //скрипт не выполнился, произошла ошибка и ответ содержит поле 'error'

signals:
    void signalPosManagerAction(const QStringList&);
    void signalRewriteParamJson(const QJsonObject&);


};




#endif // JSPOSMANAGER_TAB_H

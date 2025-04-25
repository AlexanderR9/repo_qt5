#ifndef JSPOOLTAB_H
#define JSPOOLTAB_H


#include "ug_basepage.h"

class LSearchTableWidgetBox;
class QJsonObject;


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

    void loadPoolsFromFile(); //загрузить список пулов из файла node_js
    void parseJSResult(const QJsonObject&);

protected:
    LSearchTableWidgetBox     *m_table;
    QList<JSPoolRecord>         m_poolData;

    void initTable();
    void reloadTable();
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void answerState(const QJsonObject&);
    void answerSwap(const QJsonObject&);

private:
    int pricePrecision(float, bool) const;
    void rewriteParamJson(const QJsonObject&);

protected slots:
    void slotGetPoolState();
    void slotTrySwapAssets();

signals:
    void signalPoolAction(const QStringList&);
    void signalResetApproved(const QString&); //при совершениии обмена необходимо отправить сигнал странице approve для сброса соответствующей записи
    void signalGetApprovedSize(QString, const QString&, float&); //запрос текущих опрувнутых токенов для свопа указанного актива


};




#endif // JSPOOLTAB_H

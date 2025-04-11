#ifndef JSWALLETTAB_H
#define JSWALLETTAB_H

#include "ug_basepage.h"

class LTableWidgetBox;
class QJsonObject;

//JSWalletTab
class JSWalletTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSWalletTab(QWidget*);
    virtual ~JSWalletTab() {}

    void loadAssetsFromFile(); //загрузить список активов из файла
    void getBalacesArgs(QStringList&); // список аргументв для получения балансов активов
    int assetsCount() const;
    QMap<QString, QString> assetsTokens() const;
    void parseJSResult(const QJsonObject&);

protected:
    LTableWidgetBox     *m_table;

    void addTokenToTable(const QStringList&);
    void updateTokenBalance(const QString&, const QString&);
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void updateBalances(const QJsonObject&);
    void lookTxAnswer(const QJsonObject&);

private:
    void initNativeToken();

protected slots:
    void slotWrap();
    void slotUnwrap();
    void slotTransfer();

public slots:
    void slotScriptBroken();

signals:
    void signalWalletTx(const QStringList&);


};



#endif // JSWALLETTAB_H

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
    void updateBalances(const QJsonObject&);

protected:
    LTableWidgetBox     *m_table;

    void addTokenToTable(const QStringList&);
    void updateTokenBalance(const QString&, const QString&);

private:
    void initNativeToken();

};



#endif // JSWALLETTAB_H

#ifndef JSAPPROVE_TAB_H
#define JSAPPROVE_TAB_H

#include "ug_basepage.h"

class LTableWidgetBox;
class QJsonObject;

//JSApproveTab
class JSApproveTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSApproveTab(QWidget*);
    virtual ~JSApproveTab() {}


    void setTokens(const QStringList&);
    /*
    void loadAssetsFromFile(); //загрузить список активов из файла
    void getBalacesArgs(QStringList&); // список аргументв для получения балансов активов
    int assetsCount() const;
    void updateBalances(const QJsonObject&);
    */

protected:
    LTableWidgetBox     *m_table;

    /*
    void addTokenToTable(const QStringList&);
    void updateTokenBalance(const QString&, const QString&);

private:
    void initNativeToken();
    */

};



#endif // JSAPPROVE_TAB_H

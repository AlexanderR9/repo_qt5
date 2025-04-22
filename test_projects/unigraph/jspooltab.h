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

};


//JSPoolTab
class JSPoolTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSPoolTab(QWidget*);
    virtual ~JSPoolTab() {}

    void loadPoolsFromFile(); //загрузить список пулов из файла node_js

protected:
    LSearchTableWidgetBox     *m_table;
    QList<JSPoolRecord>         m_poolData;

    void initTable();
    void reloadTable();
    void initPopupMenu(); //инициализировать элементы всплывающего меню

protected slots:
    void slotGetPoolState();
    void slotTrySwapAssets();

};




#endif // JSPOOLTAB_H

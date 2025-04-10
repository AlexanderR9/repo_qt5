#ifndef JS_TX_DIALOG_H
#define JS_TX_DIALOG_H

#include "lsimpledialog.h"


//Orepation Type
enum TX_OrepationType {txWrap = 1401, txUnwrap, txApprove, txNone = 0};

//TxDialogData
struct TxDialogData
{
    TxDialogData(int t) :tx_kind(t) {reset();}


    int tx_kind; //enum TX_OrepationType
    QString token_addr;

    QMap<QString, QString> dialog_params; //параметры которые будут возвращены после закрытия диалога

    void reset() {token_addr.clear(); dialog_params.clear();}
    //bool invalidType() const {return (order_type < totBuyLimit || order_type > totStopLoss);}
  //  bool invalid() const {return (invalidType() || lots==0 || price<=0 || company.trimmed().isEmpty());}
//    QString toStr() const {return QString("TradeOperationData: order_type=%1, lots=%2, price=%3").arg(order_type).arg(lots).arg(price);}

};

//APITradeDialog
class TxApproveDialog : public LSimpleDialog
{
    Q_OBJECT
public:
    TxApproveDialog(TxDialogData&, QWidget*);
    virtual ~TxApproveDialog() {}

    //static QString captionByOrderType(int);
    //static QString iconByOrderType(int);

protected:
    TxDialogData&     m_data;


    void init();
    void updateTitle();
    /*
    void updateWidgetsSizePolicy();
    void fillLots();
    void fillDeviations();
    void updateKindWidget();



protected slots:
    void slotRecalcResult();
*/

protected slots:
    void slotApply(); // {m_apply = true; close();}

};



#endif // JS_TX_DIALOG_H



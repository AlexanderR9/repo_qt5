#ifndef JS_TX_DIALOG_H
#define JS_TX_DIALOG_H

#include "lsimpledialog.h"


//Orepation Type
enum TX_OrepationType {txWrap = 1401, txUnwrap, txApprove, txTransfer, txSwap,
                       txMint, txIncrease, txDecrease, txCollect, txNone = 0};

//TxDialogData
struct TxDialogData
{
    TxDialogData(int t) :tx_kind(t) {reset();}

    int tx_kind; //enum TX_OrepationType
    QString token_addr;
    QString token_name;
    QMap<QString, QString> dialog_params; //параметры которые будут возвращены после закрытия диалога

    void reset() {token_addr.clear(); token_name.clear(); dialog_params.clear();}
    bool invalid() const {return (tx_kind < txWrap || tx_kind > txCollect);}

};

//TxDialogBase
class TxDialogBase : public LSimpleDialog
{
    Q_OBJECT
public:
    TxDialogBase(TxDialogData&, QWidget*);
    virtual ~TxDialogBase() {}

    static QString iconByTXType(int);
    static QString captionByTXType(int);

protected:
    TxDialogData&     m_data;

    virtual void updateTitle();

};

//TxApproveDialog
class TxApproveDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxApproveDialog(TxDialogData&, QWidget*);
    virtual ~TxApproveDialog() {}

protected:
    void init();

protected slots:
    void slotApply();
};

//TxWrapDialog
class TxWrapDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxWrapDialog(TxDialogData&, QWidget*);
    virtual ~TxWrapDialog() {}

protected:
    void init();

protected slots:
    void slotApply();
};

//TxTransferDialog
class TxTransferDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxTransferDialog(TxDialogData&, QWidget*);
    virtual ~TxTransferDialog() {}

protected:
    void init();

protected slots:
    void slotApply();
};

//TxSwapDialog
class TxSwapDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxSwapDialog(TxDialogData&, QWidget*);
    virtual ~TxSwapDialog() {}

protected:
    void init();

protected slots:
    void slotApply();
};





//TxMintPositionDialog
class TxMintPositionDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxMintPositionDialog(TxDialogData&, QWidget*);
    virtual ~TxMintPositionDialog() {}

protected:
    QGroupBox *m_mintParamsBox;

    void init();
    void initStaticFields();
    void initMintFields();
    void replaceToGridLayout();
    void checkMintParamsValidity(QString&);
    void parseNoteText();

protected slots:
    void slotApply();
    void slotRangeTypeChanged(int);
    void slotAmountsChanged();


};




#endif // JS_TX_DIALOG_H



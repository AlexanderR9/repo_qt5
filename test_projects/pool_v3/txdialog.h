#ifndef DEFI_TX_DIALOG_H
#define DEFI_TX_DIALOG_H

#include "lsimpledialog.h"


//TxDialogData
struct TxDialogData
{

    TxDialogData() :tx_kind(-1) {reset();}
    TxDialogData(int t) :tx_kind(t) {reset();}

    int tx_kind; //enum NodejsTxCommand
    QString pool_addr;
    QString token_addr;
    QString token_name;
    QMap<QString, QString> dialog_params; //параметры которые будут возвращены после закрытия диалога

    void reset() {token_addr.clear(); token_name.clear(); dialog_params.clear(); pool_addr.clear();}
    bool invalid() const; // {return (tx_kind < txWrap || tx_kind > txDestroy);}
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



/*

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
    TxMintPositionDialog(TxDialogData&, const TxDialogData&, QWidget*);
    virtual ~TxMintPositionDialog() {}

protected:
    QGroupBox *m_mintParamsBox;
    //const TxDialogData &m_savedParams;

    void init();
    void initStaticFields();
    void initMintFields();
    void replaceToGridLayout();
    void checkMintParamsValidity(QString&);
    //void parseNoteText();
    void applySavedParams(const TxDialogData&); //проверить значения из прошлой аналогичной операции из заполнить соответствующие поля

protected slots:
    void slotApply();
    void slotRangeTypeChanged(int);
    void slotAmountsChanged();
    void slotCalcAverageRangePrice();

};


//TxRemoveLiqDialog
class TxRemoveLiqDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxRemoveLiqDialog(TxDialogData&, QWidget*);
    virtual ~TxRemoveLiqDialog() {}

protected:
    void init();
    void transformByTxKind();

protected slots:
    void slotApply();
};


//TxIncreaseLiqDialog
class TxIncreaseLiqDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxIncreaseLiqDialog(TxDialogData&, QWidget*);
    virtual ~TxIncreaseLiqDialog() {}

protected:
    void init();
    void transformEditValues();
    void initAmountFields();
    void checkIncreaseParamsValidity(QString&);

protected slots:
    void slotApply();
    void slotAmountsChanged();

};

*/




#endif // DEFI_TX_DIALOG_H



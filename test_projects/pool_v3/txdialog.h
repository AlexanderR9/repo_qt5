#ifndef DEFI_TX_DIALOG_H
#define DEFI_TX_DIALOG_H

#include "lsimpledialog.h"


//TxDialogData
//универсальная структура данных для задания параметров транзакции(любой),
//считывая поля этой структуры генерится файл tx_params.json перед вызовом nodejs скрипта.
struct TxDialogData
{
    TxDialogData() :tx_kind(-1), chain_name(QString("???")) {reset();}
    TxDialogData(int t, QString chain) :tx_kind(t), chain_name(chain)  {reset();}

    int tx_kind; //enum NodejsTxCommand
    QString pool_addr;
    QString token_addr;
    QString chain_name; //current chain

    //параметры которые будут возвращены после закрытия диалога.
    //если в контейнере присутствует поле 'error', то значит параметры заданы не корректно и текст этой ошибки вывести в протокол.
    QMap<QString, QString> dialog_params;

    void reset() {token_addr.clear(); dialog_params.clear(); pool_addr.clear();}
    bool invalid() const;
};


//TxDialogBase
//пользовательский диалоговый интерфейс для настройки полей структуры TxDialogData перед совершением совершением транзакции
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
    virtual QString findTokenName() const;
    virtual void addErrorField(QString);
    //virtual void checkFields(QString&) {};
    virtual void addSimulateField();

protected slots:
    virtual void slotApply();

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
    void checkFields(QString&);

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

//TxSwapDialog
class TxSwapDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxSwapDialog(TxDialogData&, QWidget*);
    virtual ~TxSwapDialog() {}

protected:
    void init();
    QString token0() const;
    QString token1() const;
    int curInputIndex() const;

protected slots:
    void slotApply();
    void slotInputTokenChanged(int);
    void slotAmountChanged();

};


//TxBurnPosDialog
class TxBurnPosDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxBurnPosDialog(TxDialogData&, QWidget*);
    virtual ~TxBurnPosDialog() {}

protected:
    void init();

protected slots:
    void slotApply();
};


//TxCollectRewardDialog
class TxCollectRewardDialog : public TxDialogBase
{
    Q_OBJECT
public:
    TxCollectRewardDialog(TxDialogData&, QWidget*);
    virtual ~TxCollectRewardDialog() {}

protected:
    void init();

protected slots:
    void slotApply();


};


/*
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



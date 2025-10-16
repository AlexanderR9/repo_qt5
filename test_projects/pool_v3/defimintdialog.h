#ifndef DEFIMINTDIALOG_H
#define DEFIMINTDIALOG_H


#include <QDialog>

#include "ui_defimintdialog.h"

struct TxDialogData;


// DefiMintDialog
class DefiMintDialog : public QDialog, public Ui::MintDialog
{
    Q_OBJECT
public:
    DefiMintDialog(TxDialogData&, QWidget *parent = 0);
    virtual ~DefiMintDialog() {}

    inline bool isApply() const {return m_apply;}

protected:
    TxDialogData&       m_data;
    bool                m_apply;
    QMap<QString, QString>  m_tokens; // key = token_name,  value = token_address

    void init(); // инициализация виджетов
    void initBaseTokens(); // заполнить список базовых токенов

private:
    QString baseTokenAddrSelected() const;

protected slots:
    void slotApply(); // {m_apply = true; close();}
    void slotBaseTokenChainged();


};




#endif // DEFIMINTDIALOG_H




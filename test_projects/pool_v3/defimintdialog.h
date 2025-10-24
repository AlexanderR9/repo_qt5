#ifndef DEFIMINTDIALOG_H
#define DEFIMINTDIALOG_H


#include <QDialog>

#include "ui_defimintdialog.h"

struct TxDialogData;
class QJsonObject;


// DefiMintDialog
class DefiMintDialog : public QDialog, public Ui::MintDialog
{
    Q_OBJECT
public:
    DefiMintDialog(TxDialogData&, QWidget *parent = 0);
    virtual ~DefiMintDialog() {}

    inline bool isApply() const {return m_apply;}
    void poolStateReceived(const QStringList&) const; // после запроса на обновление состояния пула пришел ответ от nodejs
    void emulMintReply(const QJsonObject &js_reply) const; // после отправки транзакции типа 'mint' в режиме эмуляции пришел ответ от скрипта nodejs

protected:
    TxDialogData&       m_data;
    bool                m_apply;
    QMap<QString, QString>  m_tokens; // key = token_name,  value = token_address

    void init(); // инициализация виджетов
    void initBaseTokens(); // заполнить список базовых токенов

    void resetPoolStateTable();
    void resetPreviewTable();
    void initMintSettings() const;

private:
    QString baseTokenAddrSelected() const;


protected slots:
    void slotApply();
    void slotBaseTokenChanged(); // выполняется когда пользователь меняет базовый токен в tokenComboBox
    void slotPoolChanged(); // выполняется когда пользователь меняет пул в poolComboBox
    void slotUpdatePoolState();
    void slotAmountsEdited();
    void slotEmulateMint();

signals:
    void signalGetTokenBalance(QString, float&) const;  //запрос текущего баланса токена в кошельке по его тикеру.
    void signalTryUpdatePoolState(const QString&); // запрос текущего состояния пула
    void signalEmulateMint(const TxDialogData&); // провести предварительную операцию в режиме имитации

};




#endif // DEFIMINTDIALOG_H




#ifndef DEFIMINTDIALOG_H
#define DEFIMINTDIALOG_H


#include <QDialog>

#include "ui_defimintdialog.h"

struct TxDialogData;
class QJsonObject;


//диалог настроек для чеканки новой позы.
//предварительно нужно в двух комбобоксах базовый актив и требуемый с ним пул.
// затем в левой части запросить состояние пула, нажатием спец toolButton.
//затем в правой части задать диапазон и один(только один) из объемов актива из пары пула.
//ВНИМАНИЕ: диапазон можно задать как в вличинах привычных цен, а так же в виде значений тиков,
//для этого перед значением тика нужно поставить символ 't', (example t3 or t-5646)

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
    void updateTokensBalances(); // запрос текущих балансов пары токенов текущего пула
    void getTokensPairOfPool(QPair<QString, QString>&) const; // получить название пары токенов у текущего выбранного пула
    void checkValidityMintParams(QString&); // проверка валидности заданных параметров для чеканки позы
    void fillMintParams(int); // внесение валидных MINT параметров в m_data (основных - amounts/range)


private:
    QString baseTokenAddrSelected() const;


protected slots:
    void slotApply();
    void slotBaseTokenChanged(); // выполняется когда пользователь меняет базовый токен в tokenComboBox
    void slotPoolChanged(); // выполняется когда пользователь меняет пул в poolComboBox
    void slotUpdatePoolState();
    void slotAmountsEdited();
    void slotMint(); // выполняется когда пользователь нажимает на кномку QDialogButtonBox::Apply, происходит вызов скрипта nodejs (emulate/real_tx)
    void slotPrevewTableReset();

signals:
    void signalGetTokenBalance(QString, float&) const;  //запрос текущего баланса токена в кошельке по его тикеру.
    void signalTryUpdatePoolState(const QString&); // запрос текущего состояния пула
    void signalEmulateMint(const TxDialogData&); // провести предварительную операцию в режиме имитации



};




#endif // DEFIMINTDIALOG_H




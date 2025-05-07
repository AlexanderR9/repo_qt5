#ifndef JSTXTAB_H
#define JSTXTAB_H

#include "ug_basepage.h"

#include <QDateTime>

class LTableWidgetBox;
class QJsonObject;



//JSTxRecord
struct JSTxRecord
{
    JSTxRecord() {reset();}

    QString hash;

    // комиссия за транзакцию в нативных токенах сети.
    // если -1:  результат пока неизвестен, транзакция еще выполняется либо статус не проверялся,
    double fee;

    QDateTime dt;
    QString kind; //transaction type, example: approve or mint
    QString chain;
    bool finished_fault; //принак того, что транзакция завершилась, но действие так и не выполнилось(произошла ошибка)

    void reset() {hash.clear(); kind="none"; fee=-1; dt = QDateTime(); chain.clear(); finished_fault = false;}
    bool txFault() const {return finished_fault;}
    bool txUnknown() const {return (fee == -1);}
    bool txOk() const {return (fee >= 0 && !finished_fault);}
    void fromFileLine(const QString&);
    QString toLocalFileLine() const;
    bool invalid() const;
    QString strTime() const;
    QString strDate() const;
    QString strFee() const;
    QString strResult() const;
    void setJSResponse(QString, QString, bool); //обновить данные после проверки статуса

};

//JSTxTab
class JSTxTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSTxTab(QWidget*);
    virtual ~JSTxTab() {}

    void loadTxFromFile(QString); //загрузить список hash из файла
    inline int txCount() const {return tx_data.count();}
    void parseJSResult(const QJsonObject&);

protected:
    LTableWidgetBox     *m_table;
    QList<JSTxRecord>    tx_data;


    //информация загруженная из локального файла, содержит информацию только о завершенных транзакциях.
    //вид строки: hash / fee_size / status (OK/FAULT)
    QStringList          m_locData;

    void initTable();
    void reloadTable();
    void applyLocalData();
    void loadLocalData(); //from local defi file
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    JSTxRecord* recordByHash(QString);
    void updateTableRowByRecord(const JSTxRecord*);
    void addRecToLocalFile(const JSTxRecord *rec); //после получения ответа от nodejs добавить информацию в локальный файл

protected slots:
    void slotTxStatus();

signals:
    void signalCheckTx(const QStringList&);

};

#endif // JSTXTAB_H

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
    // если -2: транзакция завершилась, но действие так и не выполнилось(произошла ошибка)
    double fee;

    QDateTime dt;
    QString kind; //transaction type, example: approve or mint
    QString chain;

    void reset() {hash.clear(); kind="none"; fee=-1; dt = QDateTime(); chain.clear();}
    bool txFault() const {return (fee == -2);}
    bool txOk() const {return (fee >= 0);}
    void fromFileLine(const QString&);
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

    void loadTxFromFile(); //загрузить список hash из файла
    inline int txCount() const {return tx_data.count();}
    void parseJSResult(const QJsonObject&);

protected:
    LTableWidgetBox     *m_table;
    QList<JSTxRecord>    tx_data;

    void initTable();
    void reloadTable();
    void initPopupMenu(); //инициализировать элементы всплывающего меню
    JSTxRecord* recordByHash(QString);
    void updateTableRowByRecord(const JSTxRecord*);

protected slots:
    void slotTxStatus();

signals:
    void signalCheckTx(const QStringList&);

};

#endif // JSTXTAB_H

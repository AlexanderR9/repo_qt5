#ifndef BASETABPAGE_V3_H
#define BASETABPAGE_V3_H


#include "lsimplewidget.h"

class QJsonObject;


// BaseTabPage_V3
class BaseTabPage_V3 : public LSimpleWidget
{
    Q_OBJECT
public:
    BaseTabPage_V3(QWidget*, int, int);
    virtual ~BaseTabPage_V3() {}

    inline int kind() const {return userSign();}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    virtual void setChain(int);

    //выполняется запрос к сети(read only) для обновления данных на странице.
    //выполняется когда пользователь в тулбаре нажимает кнопку "Update"
    virtual void sendUpdateDataRequest() = 0;

protected:
    LTableWidgetBox     *m_table;
    LTableWidgetBox     *m_integratedTable;

    virtual void sendReadNodejsRequest(const QJsonObject&); // отаправить команду в nodejs_bridge для запроса на чтение из сети
    virtual void sendTxNodejsRequest(const QJsonObject&); // отаправить команду в nodejs_bridge для запроса на запись транзакции в сеть

public slots:
    virtual void slotNodejsReply(const QJsonObject&) = 0; //получен успешный ответ от скрипта nodejs

signals:
    void signalRewriteJsonFile(const QJsonObject&, QString);
    void signalRunNodejsBridge(QString, const QStringList&);

};

#endif // BASETABPAGE_V3_H



#ifndef UG_TOKENPAGE_H
#define UG_TOKENPAGE_H


#include "ug_basepage.h"
#include "ug_apistruct.h"

class QJsonObject;


//UG_TokenPage
class UG_TokenPage : public UG_BasePage
{
    Q_OBJECT
public:
    UG_TokenPage(QWidget*);
    virtual ~UG_TokenPage() {m_tokens.clear();}

    QString iconPath() const {return QString(":/icons/images/stock.png");}
    QString caption() const {return QString("Tokens");}

    virtual void saveData();
    virtual void loadData();
    virtual void updateDataPage(bool forcibly = false);  //выполняется когда эта страница активируется (всплывает наверх) в stacked_widget
    virtual void startUpdating(quint16);

    bool tokenContains(QString addr) const;

protected:
    QList<UG_TokenInfo>      m_tokens; //контейнер для хранения записей-tokens
    LSearchTableWidgetBox   *m_tableBox;
    QStringList              m_updatedTokens; //адреса токенов для текущей сети, нужен только на время обновления

    void initTable();
    void clearPage();
    void updateTableData();
    void loadAllPoolFiles();
    void loadFromFileData(const QStringList&, const QString&);
    virtual QString dataFile() const;
    void findCurChainTokens();
    void prepareQuery();
    void updateTokenData(const QString&, const QJsonObject&);

public slots:
    virtual void slotJsonReply(int, const QJsonObject&);
    virtual void slotReqBuzyNow()  {}

protected slots:
    virtual void slotTimer();

signals:
    void signalGetTokensFromPoolPage(QHash<QString, QString>&); //key - id token, value - ticker

};


#endif // UG_TOKENPAGE_H

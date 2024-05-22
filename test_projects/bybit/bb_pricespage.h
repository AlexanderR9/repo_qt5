#ifndef BB_PRICESPAGE_H
#define BB_PRICESPAGE_H

#include "bb_basepage.h"
#include "bb_apistruct.h"


class QJsonObject;
class QJsonArray;
class QJsonValue;
class QSettings;


// BB_PricesPage
class BB_PricesPage : public BB_BasePage
{
    Q_OBJECT
public:
    enum DevDays {dd1D=1, dd3D=3, dd1W=7, dd1M=30, dd3M=91, dd6M=182, dd12M=365};

    BB_PricesPage(QWidget*);
    virtual ~BB_PricesPage() {}

    QString iconPath() const {return QString(":/icons/images/r_scale.svg");}
    QString caption() const {return QString("Prices monitor");}
    virtual void load(QSettings&);

    void updateDataPage(bool);


protected:
    LSearchTableWidgetBox     *m_monitTable;
    BB_PricesContainer         m_container;

    void init();
    void loadTickers();
    void loadContainer(); //load all prices to m_container from file
    void updateTablePrices(const QMap<QString, float>&);
    void updateTableDeviations();
    void updateTableDeviationsRow(int, const QStringList&);
    void updateContainer(const QMap<QString, float>&);
    void newPricesReceived(const QJsonArray&);
    void rewriteFile();

public slots:
    void slotJsonReply(int, const QJsonObject&);

};



#endif // BB_PRICESPAGE_H



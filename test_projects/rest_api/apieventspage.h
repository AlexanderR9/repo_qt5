#ifndef APIEVENTSPAGE_H
#define APIEVENTSPAGE_H

#include "apipages.h"
#include "instrument.h"


class QJsonObject;

// APIEventsPage
class APIEventsPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIEventsPage(QWidget*);
    virtual ~APIEventsPage() {}

    QString iconPath() const {return QString(":/icons/images/event.png");}
    QString caption() const {return QString("Events");}

protected:
    QList<EventOperation> m_events;


    void reloadTableByData();

public slots:
    void slotLoadEvents(const QJsonObject&);

signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void signalGetTickerByFigi(const QString&, QString&);

};

#endif // APIEVENTSPAGE_H

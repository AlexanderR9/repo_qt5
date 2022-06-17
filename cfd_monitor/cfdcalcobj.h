#ifndef CFDCALCOBJ_H
#define CFDCALCOBJ_H

#include "lsimpleobj.h"


#include <QTime>
#include <QDate>
#include <QDateTime>

//файл данных имеет вид: построчно перечислены пары: цена - дата и время, пример 45.56(15.06.2022 12:56)
//все значения времени должны идти по возрастанию


//CFDFileRecord  (запись для одной временной точки и соответвующей ей цены)
struct CFDFileRecord
{
    CFDFileRecord() {reset();}
    CFDFileRecord(const QDateTime &dt, const double &p) :date(dt.date()), time(dt.time()), price(p) {}

    QDate date;
    QTime time;
    double price;

    void parseFileLine(const QString&);
    QString toFileLine() const;
    bool invalid() const;
    void reset();
    QDateTime dt() const {return QDateTime(date, time);}

};


//CFDCalcObj
class CFDCalcObj : public LSimpleObject
{
    Q_OBJECT
public:
    CFDCalcObj(QObject*);
    virtual ~CFDCalcObj() {}

protected:
    QList<CFDFileRecord> m_currentData; //контейнер для загрузки всей истории цены для одного тикера

    void addToFile(const QString&, double&, bool &ok); //добавить в файл и в m_currentData новое значение для указанного тикера
    void loadTickerFile(const QString&); //загрузить все данные для указанного тикера в m_currentData
    void appendTickerFile(const QString&, const CFDFileRecord&); //append last record
    QString filePathByTicker(const QString&) const;
    QString changingPriceByPeriod(uint) const; //param n hours ago

public slots:
    void slotNewPrice(QString, double); //добавить очередное значение цены для указанного тикера

signals:
    void signalUpdateCFDTable(const QStringList&);

};


#endif //CFDCALCOBJ_H



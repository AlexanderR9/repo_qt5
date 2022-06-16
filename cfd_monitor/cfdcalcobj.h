#ifndef CFDCALCOBJ_H
#define CFDCALCOBJ_H

#include "lsimpleobj.h"


#include <QTime>

//файл данных имеет вид: сначала идет строка с указанием года - [year : 2022]
//далее идет строка с указанием месяца - [month : 4]
//далее идет количество строк равное количеству дней в этом месяце (или менее если этот день еще не наступил)
//если в какой-то день нет данных значит там должна быть пустая строка
//данные в строке привязаны ко дню(номеру строки идущей за указанием месяца [month : X]) имеют вид 45.56(12:56); 45.11(16:24); 44.88(17:45); и .т.д
//т.е. значение цены(время) и такие пары разделены ';'
//все значения годов, месяцев, времени должны идти по возрастанию


//CFDFileRecord
struct CFDFileRecord
{
    CFDFileRecord() :year(0), month(0), day(0), price(0) {}
    CFDFileRecord(const QTime &t, double &p) :year(0), month(0), day(0), time(t), price(p) {}
    CFDFileRecord(quint16 y, quint8 m, quint8 d) :year(y), month(m), day(d), price(0) {}

    quint16 year;
    quint8 month;
    quint8 day;
    QTime time;
    double price;

    static int getYearFileValue(const QString&);
    static int getMonthFileValue(const QString&);
    static void getPricePointFileValue(const QString&, double&, QTime&);

    bool invalid() const;

};


//CFDCalcObj
class CFDCalcObj : public LSimpleObject
{
    Q_OBJECT
public:
    CFDCalcObj(QObject*);
    virtual ~CFDCalcObj() {}

protected:
    QList<CFDFileRecord> m_currentData;

    void addToFile(const QString&, double&, bool &ok);
    void loadTickerFile(const QString&);
    void saveTickerFile(const QString&);
    QString filePathByTicker(const QString&) const;

public slots:
    void slotNewPrice(QString, double);

signals:
    void signalUpdateCFDTable(const QStringList&);



};


#endif //CFDCALCOBJ_H



#ifndef FXBAR_H
#define FXBAR_H


//#include <QObject>
#include "lsimpleobj.h"
#include <QDateTime>


//FXBar
class FXBar
{
public:
    FXBar();
    FXBar(quint8);
    virtual ~FXBar() {}

    bool invalid() const;
    QString toStr() const;

    //попытка загрузить данные свечи из строки файла, sep_values - разделитель значений
    //пример строки: 2019.10.13,00:00,179.49,183.72,177.63,179.71,1336
    void fromFileLine(const QString&, QString sep_values = ",");

protected:
    double      m_open;
    double      m_close;
    double      m_high;
    double      m_low;
    quint32     m_volume; //количество тиков(цена менялась на какую-то величину) за время существования свечи
    QDateTime   m_time; //begin time of bar

    //произвольная точность, пользователь сам устанавливает, ту которою считает разумной независимо от значений цен
    quint8  m_digist;

    //сброс всех атрибутов свечи
    void reset();


};

//FXBarContainer
class FXBarContainer : public LSimpleObject
{
    Q_OBJECT
public:
    FXBarContainer(QObject *parent) :LSimpleObject(parent), m_fileName(QString()) {reset();}
    FXBarContainer(const QString&, QObject*); //параметр - полный путь файла данных
    virtual ~FXBarContainer() {}

    void tryLoadData(QString sep_values = ","); // загрузка данных в контейнер m_data, sep_values - разделитель значений
    bool invalid() const;
    QString toStr() const;


protected:
    QList<FXBar>    m_data; //загруженные данные из файла
    QString         m_fileName; //имя файла(полный путь) - источника данных, формат имени: couplename_timeframe_digist.csv (пример USDJPY_1440_2.csv)
    int             m_timeframe; //период (определяется автоматически при загрузке данных)
    QString         m_couple; //название инструмента (определяется автоматически при загрузке данных)
    quint8          m_digist; //точность значений цен (определяется автоматически при загрузке данных)


    void reset();
    void checkFileName(); //проверка формата названия файла данных


};


#endif //FXBAR_H



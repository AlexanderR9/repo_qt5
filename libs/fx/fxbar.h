#ifndef FXBAR_H
#define FXBAR_H

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



#endif //FXBAR_H



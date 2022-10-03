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

    inline const double& open() const {return m_open;}
    inline const double& close() const {return m_close;}
    inline const double& high() const {return m_high;}
    inline const double& low() const {return m_low;}
    inline quint32 volume() const {return m_volume;}
    inline const QDateTime& time() const {return m_time;}

    QString strTime() const;

    //попытка загрузить данные свечи из строки файла, sep_values - разделитель значений
    //пример строки: 2019.10.13,00:00,179.49,183.72,177.63,179.71,1336
    void fromFileLine(const QString&, QString sep_values = ",");

protected:
    double      m_open;     //цена открытия
    double      m_close;    //цена закрытия
    double      m_high;     //максимальная цена свечи
    double      m_low;      //минимальная цена свечи
    quint32     m_volume;   //количество тиков(цена менялась на какую-то величину) за время существования свечи
    QDateTime   m_time;     //дата и время соответствующая открытию свечи

    //произвольная точность, пользователь сам устанавливает, ту которою считает разумной независимо от значений цен
    quint8  m_digist;

    //сброс всех атрибутов свечи
    void reset();
};



#endif //FXBAR_H



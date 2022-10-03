#ifndef FXBAR_CONTAINER_H
#define FXBAR_CONTAINER_H

#include "lsimpleobj.h"
#include "fxbar.h"

#include <QList>

class QPointF;

//FXBarContainer
class FXBarContainer : public LSimpleObject
{
    Q_OBJECT
public:
    FXBarContainer(QObject *parent) :LSimpleObject(parent), m_fileName(QString()) {reset();}
    FXBarContainer(const QString&, QObject*); //параметр - полный путь файла данных
    virtual ~FXBarContainer() {clearData();}

    void tryLoadData(QString sep_values = ","); // загрузка данных в контейнер m_data, sep_values - разделитель значений
    void getChartPoints(QList<QPointF>&) const; //выдать набор точек для отображения цен инструмента на графике
    const FXBar* barAt(int) const; //выдать свечу по индексу, если индекс некорректный вернет NULL

    QDateTime firstTime() const;
    QDateTime lastTime() const;


    bool invalid() const;
    QString toStr() const;

    inline QString couple() const {return m_couple;}
    inline int timeframe() const {return m_timeframe;} //значение из множества FXTimeFrame
    inline void setDigist(quint8 d) {m_digist = d;} //точность пункта
    inline quint8 digist() const {return m_digist;}
    inline quint16 barCount() const {return m_data.count();} //количество валидных загруженных свечей
    inline bool dataEmpty() const {return m_data.isEmpty();}



    static QString dataFileFormat() {return QString("csv");}

protected:
    QList<FXBar>    m_data; //загруженные данные из файла, (1-й самый ранний, последний самый поздний)
    QString         m_fileName; //имя файла(полный путь) - источника данных, формат имени: couplename_timeframe.csv (пример USDJPY_1440.csv)
    int             m_timeframe; //период (определяется автоматически при загрузке данных)
    QString         m_couple; //название инструмента (определяется автоматически при загрузке данных)
    quint8          m_digist; //точность значений цен, задается пользователем

    void reset();
    void checkFileName(); //проверка формата названия файла данных
    void clearData(); //очистить полностью контейнер m_data

};


#endif //FXBAR_CONTAINER_H



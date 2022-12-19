#ifndef LXMLPACK_VALUE_H
#define LXMLPACK_VALUE_H


#include <QList>


//class QDomNode;
class QDataStream;


//структура описания 1-го значения элемента пакета.
//изпользуется только одно из двух значений (i_value/d_value) в зависимости от isDouble
struct LXMLPackValue
{
    LXMLPackValue() {reset();}
    LXMLPackValue(const LXMLPackValue &pv) {copy(pv);}

    qint64 i_value;
    double d_value;
    double rand_deviation; //случайное отклонение от значения в %

    //из следующих буленовских переменных значение true может принимать только одна (или не одной)
    bool isDouble; //признак того что значение вещественное
    bool isDiscrete; //признак того что значение дискретное (0/1)
    bool isUnsigned; //признак того что значение беззнаковое целое

    void reset();// сброс всех полей структуры
    void copy(const LXMLPackValue&); // копирование экземпляра структуры
    QString strValue(quint8) const; // строковое представление значения
    QString strDeviation() const; //строковое представление отклонения значения
    void recalcNext(); //пересчитать значение с учетом rand_deviation

    //попытка установить значение пользователя (когда пользователь вводит его в интерфейсе)
    //в случае некорректного строкового значения во второй параметр запишется ошибка, а значение структуры не изменится.
    void setUserValue(const QString&, QString&);

    //попытка установить погрешность пользователя (когда пользователь вводит ее в интерфейсе)
    //в случае некорректного строкового значения во второй параметр запишется ошибка, а значение структуры не изменится.
    void setUserDeviation(const QString&, QString&);

    //преобразовывает целочисленное значение согласно типу данных ноды
    void conversionIntType(int);

    QString toStr() const; //диагностический выхлоп     // {return QString("PackValue: iv=%1  dv=%2 err=%3  type(%4)").arg(i_value).arg(QString::number(d_value, 'f', 4)).arg(QString::number(rand_deviation, 'f', 4)).arg(isDouble?"double":"integer");}


    void writeToStream(int, QDataStream&); //записать m_value в поток данных в зависимости от типа данных элемента
    void readFromStream(int, QDataStream&); //считать m_value из потока данных в зависимости от типа данных элемента

private:
    int r_sign() const {return ((qrand()%100 < 50) ? -1 : 1);} //случайное значение знака отклонения

};


#endif // LXMLPACK_VALUE_H




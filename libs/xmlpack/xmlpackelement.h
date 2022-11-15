#ifndef LXMLPACK_ELEMENT_H
#define LXMLPACK_ELEMENT_H


#include <QList>


class QDomNode;
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
    bool isDouble;

    void reset() {i_value = 0; d_value = 0; rand_deviation = 0; isDouble = false;}
    void copy(const LXMLPackValue &pv) {i_value = pv.i_value; d_value = pv.d_value; rand_deviation = pv.rand_deviation; isDouble = pv.isDouble;}
    QString strValue(quint8 p) const {return (isDouble ? QString::number(d_value, 'f', p) : QString::number(i_value));}
    QString strDeviation() const {return QString::number(rand_deviation, 'f', 2);}
    QString toStr() const {return QString("PackValue: iv=%1  dv=%2 err=%3  type(%4)").arg(i_value).arg(QString::number(d_value, 'f', 4)).arg(QString::number(rand_deviation, 'f', 4)).arg(isDouble?"double":"integer");}
    void next() {if (rand_deviation >= 0.01) {isDouble ? (d_value *= nextFactor()) : (i_value *= nextFactor());}}

private:
    double rf() const {return (rand_deviation/double(1+qrand()%10))/double(100);}
    int r_sign() const {return ((qrand()%100 < 50) ? -1 : 1);}
    double nextFactor() const {return (1+r_sign()*rf());}

};



//элемент структуры пакета, может быть нодой, в которую вложены другие ноды и элементы данных.
//если обьект является нодой(секцией) то m_dataType == petSection.
//может быть массивом, как массивом нод так и массивом элементов.
//рутовая нода всегда должна называться packet.
class LXMLPackElement
{
public:
    LXMLPackElement();
    LXMLPackElement(LXMLPackElement*);
    virtual ~LXMLPackElement() {destroyChilds();}

    void loadNode(const QDomNode&, QString&); //загрузить структуру ноды (самой себя)
    void calcOffset(LXMLPackElement*, quint32&); //определить m_offset для этого элемента
    LXMLPackElement* clone(LXMLPackElement *parent = NULL) const; //клонирование елемента
    void appendChild(LXMLPackElement*);
    void retransformArrChilds(); //размножить все ноды-массивы найденые среди m_childs, выполняется после загрузки пакета
    void writeToStream(QDataStream&); //записать содержимое елемента в поток данных
    void readFromStream(QDataStream&); //читать содержимое елемента из потока данных
    QString strValue(quint8) const; //получить текущее значение элемента в строковом виде
    QString strValueDeviation() const; //получить возможное отклонение значения элемента в строковом виде
    void setNewValue(const QString&, bool &ok); // попытка установить новое значение
    void setNewValueDeviation(const QString&, bool &ok); // попытка установить новое отклонение значения
    void nextRandValue(); //обновить значение m_value с учетом rand_deviation, (если rand_deviation == 0, то значение не измениться)


    bool isNode() const; //элемент является секцией
    bool isTime() const; //элемент является структурой TimeSpec
    bool isRoot() const; //элемент является рутовой нодой (кстати она так же является isNode())
    bool isData() const; //элемент является конечным элементом данных ветки
    bool isArr() const {return (m_arrSize > 1);}
    bool isOff() const {return (m_arrSize == 0);} //элемент отключен в пакете, включая вложенные в него элементы
    bool isDouble() const {return m_value.isDouble;}
    quint32 size() const; //возвращает размер в байтах, включая вложенные элементы
    quint32 offset() const; //возвращает смещение в пакете в байтах

    inline void setCation(const QString &s) {m_caption = s;}
    inline quint16 arrSize() const {return m_arrSize;}
    inline void setArrSize(quint16 n) {m_arrSize = n;}
    inline QString caption() const {return m_caption;}
    inline int dataType() const {return m_dataType;}
    inline void setDataType(int t) {m_dataType = t;}
    inline int childsCount() const {return m_childs.count();}
    inline bool hasChilds() const {return !m_childs.isEmpty();}
    inline const LXMLPackElement* childAt(int i) const {return ((i<0 || i>=childsCount()) ? NULL : m_childs.at(i));}
    inline LXMLPackElement* childAtVar(int i) const {return ((i<0 || i>=childsCount()) ? NULL : m_childs.at(i));}
    inline void setValueInfo(const LXMLPackValue &v) {m_value.copy(v);}
    inline const LXMLPackValue& getValue() const {return m_value;}


    bool invalid() const;

protected:
    int m_dataType; //елемент множества XMLPackElementType
    quint16 m_arrSize; //если > 1, указывает на то что этот елемент/нода является массивом, если m_arrSize == 0, то этот елемент пропускается (как будто его нет в описании)
    quint32 m_offset; //смещение элемента в пакете
    QString m_caption; //название элемента
    LXMLPackValue m_value; //значение элемента, актуально если елемент isData()

    LXMLPackElement *m_parentNode; //указатель на родительскую ноду
    QList<LXMLPackElement*> m_childs; //дети этой ноды, если список пустой, той этот элемент является конечным элементов ветки

    void destroyChilds(); //очистить список m_childs и удалить из память все его элементы
    void loadValueAttrs(const QDomNode&); //загрузить атрибуты значения элемента
    void loadChilds(const QDomNode&); //загрузить детей
    void retransformArrChild(int); //размножить найденный элемент-массив среди детей
    void transformTimeSpec();//сформировать структуру элементов для типа TimeSpec
    void nextTimeValue(); //обновить значение m_value типа timespec

private:
    quint32 sectionSize(const LXMLPackElement*) const; //возвращает размер в байтах секции целиком
    void reset(); //сброс переменных
    void writeValueToStream(QDataStream&); //записать m_value в поток данных в зависимости от типа данных элемента
    void readValueFromStream(QDataStream&); //считать m_value из потока данных в зависимости от типа данных элемента
    void setIntValue(qint64);

};




#endif // LXMLPACK_ELEMENT_H




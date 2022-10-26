#ifndef LXMLPACK_ELEMENT_H
#define LXMLPACK_ELEMENT_H


//#include "lsimpleobj.h"
#include <QList>


//class QDomDocument;
class QDomNode;


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

    void loadNode(const QDomNode&, QString&); //загрузить структуру ноды
    void calcOffset(LXMLPackElement*, quint32&); //определить m_offset для этого элемента
    LXMLPackElement* clone(LXMLPackElement *parent = NULL) const; //клонирование елемента
    void appendChild(LXMLPackElement*);
    void retransformArrChilds(); //размножить все ноды-массивы найденые среди m_childs, выполняется после загрузки пакета


    bool isNode() const; //элемент является секцией
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
    inline void setValueInfo(const LXMLPackValue &v) {m_value.copy(v);}


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

private:
    quint32 sectionSize(const LXMLPackElement*) const; //возвращает размер в байтах секции целиком
    void reset(); //сброс переменных

};

/*
//класс для загрузки и хранения структуры одного xml-пакета
class LXMLPackObj : public LSimpleObject
{
    Q_OBJECT
public:
    LXMLPackObj(const QString&, QObject *parent = NULL); //params: xmlfilename of packet, obj_parent
    virtual ~LXMLPackObj() {delete m_rootNode;}

    void tryLoadPacket(bool&); //загрузить пакет из файла xml (m_fileName)


    inline bool invalid() const {return (m_rootNode == NULL);}
    inline quint32 size() const {return (invalid() ? 0 : m_rootNode->size());} //возвращает размер пакета в байтах
    inline const LXMLPackElement* rootElement() const {return m_rootNode;}

protected:
    LXMLPackElement *m_rootNode;
    QString m_fileName; //xml файл с описанием пакета

    void loadDom(const QDomDocument&, bool&); //создать структуру пакета из считанного QDomDocument
    void recalcOffset(); //пересчитать параметр m_offset для всех нод, выполняется после загрузки пакета
    void retransformArrNodes(); //размножить все ноды-массивы, выполняется после загрузки пакета

protected:


};

*/

#endif // LXMLPACK_ELEMENT_H




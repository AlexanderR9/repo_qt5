#ifndef LXMLPACK_OBJECT_H
#define LXMLPACK_OBJECT_H


#include "lsimpleobj.h"
#include <QList>


class QDomDocument;
class QDomNode;


//структура описания 1-го значения элемента пакета.
//изпользуется только одно из двух значений (i_value/d_value) в зависимости от isDouble
struct LXMLPackValue
{
    LXMLPackValue() {reset();}

    qint64 i_value;
    double d_value;
    double rand_deviation; //случайное отклонение от значения в %
    bool isDouble;

    void reset() {i_value = 0; d_value = 0; rand_deviation = 0; isDouble = false;}
};



//элемент структуры пакета, может быть нодой, в которую вложены другие ноды и элементы данных.
//если обьект является нодой(секцией) то m_dataType == petNone.
//может быть массивом, как массивом нод так и массивом элементов.
//рутовая нода всегда должна называться packet.
class LXMLPackElement
{
public:
    LXMLPackElement();
    LXMLPackElement(LXMLPackElement*);
    virtual ~LXMLPackElement() {reset();}


    bool isNode() const; //элемент является секцией
    bool isRoot() const; //элемент является рутовой нодой (кстати она так же является isNode())
    bool isData() const; //элемент является конечным элементом данных ветки
    bool isArr() const {return (m_arrSize > 1);}
    bool isOff() const {return (m_arrSize == 0);} //элемент отключен в пакете, включая вложенные в него элементы
    bool isDouble() const {return m_value.isDouble;}

    inline void setCation(const QString &s) {m_caption = s;}
    inline quint16 arrSize() const {return m_arrSize;}
    inline QString caption() const {return m_caption;}

    void loadNode(const QDomNode&, QString&); //загрузить структуру ноды
    bool invalid() const;

protected:
    int m_dataType; //елемент множества XMLPackElementType
    quint16 m_arrSize; //если > 1, указывает на то что этот елемент/нода является массивом, если m_arrSize == 0, то этот елемент пропускается (как будто его нет в описании)
    QString m_caption; //название элемента
    LXMLPackValue m_value;

    LXMLPackElement *m_parentNode; //указатель на родительскую ноду
    QList<LXMLPackElement*> m_childs; //дети этой ноды, если список пустой, той этот элемент является конечным элементов ветки

    void reset();
    void loadValueAttrs(const QDomNode&); //загрузить атрибуты значения элемента
    void loadChilds(const QDomNode&); //загрузить детей

};


//класс для загрузки и хранения структуры одного xml-пакета
class LXMLPackObj : public LSimpleObject
{
    Q_OBJECT
public:
    LXMLPackObj(const QString&, QObject *parent = NULL);
    virtual ~LXMLPackObj() {delete m_rootNode;}

    void tryLoadPacket(bool&); //загрузить пакет из файла xml
    inline bool invalid() const {return (m_rootNode == NULL);}
    inline const LXMLPackElement* rootElement() const {return m_rootNode;}

protected:
    LXMLPackElement *m_rootNode;
    QString m_fileName; //xml файл с описанием пакета

    void loadDom(const QDomDocument&, bool&); //создать структуру пакета из считанного QDomDocument
};



#endif // LXMLPACK_OBJECT_H




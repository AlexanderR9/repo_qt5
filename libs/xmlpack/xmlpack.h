#ifndef LXMLPACK_OBJECT_H
#define LXMLPACK_OBJECT_H


#include "lsimpleobj.h"


class QDomDocument;
class LXMLPackElement;
class QByteArray;


//класс для загрузки и хранения структуры одного xml-пакета
class LXMLPackObj : public LSimpleObject
{
    Q_OBJECT
public:
    LXMLPackObj(const QString&, QObject *parent = NULL); //params: xmlfilename of packet, obj_parent
    virtual ~LXMLPackObj() {destroyObj();}

    void tryLoadPacket(bool&); //загрузить пакет из файла xml (m_fileName)
    quint32 size() const; //возвращает размер пакета в байтах
    void setByteOrder(int); //устанавливает порядок байт для записи пакета в поток данных

    inline bool invalid() const {return (m_rootNode == NULL);}
    inline const LXMLPackElement* rootElement() const {return m_rootNode;}
    inline LXMLPackElement* rootElementVar() const {return m_rootNode;}

    //QDataStream operation
    void toByteArray(QByteArray&, bool singleFloatPrecision = false); //запись пакета в массив байт
    void fromByteArray(const QByteArray&, bool &ok, bool singleFloatPrecision = false); //запись массива байт в пакет, если размер BA не равен размеру пакета то ok==false

protected:
    LXMLPackElement *m_rootNode;
    QString m_fileName; //xml файл с описанием пакета
    int m_byteOrder;

    void loadDom(const QDomDocument&, bool&); //создать структуру пакета из считанного QDomDocument
    void recalcOffset(); //пересчитать параметр m_offset для всех нод, выполняется после загрузки пакета
    void retransformArrNodes(); //размножить все ноды-массивы, выполняется после загрузки пакета
    void destroyObj();

};



#endif // LXMLPACK_OBJECT_H




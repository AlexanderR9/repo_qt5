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
    LXMLPackObj(QObject *parent = NULL);
    LXMLPackObj(const QString&, QObject *parent = NULL); //params: xmlfilename of packet, obj_parent
    virtual ~LXMLPackObj() {destroyObj();}

    void tryLoadPacket(bool&); //загрузить пакет из файла xml (m_fileName)
    void tryLoadPacket(const QDomDocument&, bool&); //загрузить пакет из QDomDocument
    quint32 size() const; //возвращает размер пакета в байтах
    void setByteOrder(int); //устанавливает порядок байт для записи пакета в поток данных
    void nextRandValues(); //обновить значения всех элементов пакета с учетом rand_deviation, (если rand_deviation == 0, то значение не измениться)
    QString caption() const; //caption рутовой ноды

    //установить значение 1-й ноды по заданному пути.
    //путь указывается в виде набора уровней вложенности, начинается всегда с 'packet'
    //пример: packet/2/0/11
    //если по заданному пути нет ноды или ее тип не соответствует устанавливаему значению то в параметр bool запишется false
    void setIntValueByPath(QString, qint64, bool&);
    void setDoubleValueByPath(QString, double, bool&);

    //возвращает значение ноды соответствующего типа
    qint64 getIntValueByPath(QString, bool&);
    double getDoubleValueByPath(QString, bool&);

    inline bool invalid() const {return (m_rootNode == NULL);}
    inline const LXMLPackElement* rootElement() const {return m_rootNode;}
    inline LXMLPackElement* rootElementVar() const {return m_rootNode;}
    inline bool kksUsed() const {return m_kksUsed;}

    //QDataStream operation
    void toByteArray(QByteArray&, bool singleFloatPrecision = false); //запись пакета в массив байт
    void fromByteArray(const QByteArray&, bool &ok, bool singleFloatPrecision = false); //запись массива байт в пакет, если размер BA не равен размеру пакета то ok==false

protected:
    LXMLPackElement *m_rootNode;
    QString m_fileName; //xml файл с описанием пакета
    int m_byteOrder;
    bool m_kksUsed; //признак того что в описании пакеты будут использованы ККС, этот признак задается в рутовой ноде атрибутом: use_kks="true"

    void loadDom(const QDomDocument&, bool&); //создать структуру пакета из считанного QDomDocument
    void recalcOffset(); //пересчитать параметр m_offset для всех нод, выполняется после загрузки пакета
    void retransformArrNodes(); //размножить все ноды-массивы, выполняется после загрузки пакета
    void destroyObj();

private:
    QList<quint16> parseXPath(const QString&) const;

};



#endif // LXMLPACK_OBJECT_H




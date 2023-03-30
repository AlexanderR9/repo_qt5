#ifndef LXMLPACK_TYPE_H
#define LXMLPACK_TYPE_H


#include <QString>


//типы данных элементов xml пакета
enum XMLPackElementType {petInt8 = 601, petInt16, petInt32, petInt64, petUint8, petUint16, petUint32, petUint64,
                            petFloat, petDouble, petTimeSpec, petSection, petDiscrete, petInvalid = -1};


//XMLPackStatic (набор статических функций)
class XMLPackStatic
{
public:
    static QString xmlAttrName(int); //строкое название атрибута для заданного типа в описании XML файла.
    static int typeByXmlAttr(QString); //вернет элемент из множества XMLPackElementType по его строковому значению атрибута в описании XML файла.
    static bool isDoubleType(int);
    static bool isIntegerType(int);
    static bool isUnsignedType(int);
    static quint8 sizeOf(int); //вернет размер типа в байтах либо 0


    static QString rootNodeName() {return QString("packet");}
    static QString cationAttrName() {return QString("caption");}
    static QString dataTypeAttrName() {return QString("type");}
    static QString arrSizeAttrName() {return QString("arr_size");}
    static QString defValueAttrName() {return QString("def_value");}
    static QString errValueAttrName() {return QString("err_value");}
    static QString userDataAttrName() {return QString("user_data");}

};


#endif // LXMLPACK_TYPE_H

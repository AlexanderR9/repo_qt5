#include "xmlpacktype.h"

#include <QDebug>

QString XMLPackStatic::xmlAttrName(int t)
{
    switch (t)
    {
        case petInt8:       return QString("int8");
        case petInt16:      return QString("int16");
        case petInt32:      return QString("int32");
        case petInt64:      return QString("int64");

        case petUint8:       return QString("uint8");
        case petUint16:      return QString("uint16");
        case petUint32:      return QString("uint32");
        case petUint64:      return QString("uint64");

        case petFloat:      return QString("float");
        case petDouble:     return QString("double");
        case petSection:     return QString("section");

        default: break;
    }
    return QString();
}
bool XMLPackStatic::isDoubleType(int t)
{
    switch (t)
    {
        case petFloat:
        case petDouble: return true;
        default: break;
    }
    return false;
}
bool XMLPackStatic::isIntegerType(int t)
{
    switch (t)
    {
        case petInt8:
        case petInt16:
        case petInt32:
        case petInt64:
        case petUint8:
        case petUint16:
        case petUint32:
        case petUint64: return true;
        default: break;
    }
    return false;
}
quint8 XMLPackStatic::sizeOf(int t)
{
    switch (t)
    {
        case petInt8:
        case petUint8: return 1;

        case petInt16:
        case petUint16: return 2;

        case petFloat:
        case petInt32:
        case petUint32: return 4;

        case petDouble:
        case petInt64:
        case petUint64: return 8;

        default: break;
    }
    return 0;
}
bool XMLPackStatic::isUnsignedType(int t)
{
    switch (t)
    {
        case petUint8:
        case petUint16:
        case petUint32:
        case petUint64: return true;
        default: break;
    }
    return false;
}
int XMLPackStatic::typeByXmlAttr(QString attr)
{
    if (attr == "int8")      return petInt8;
    if (attr == "int16")     return petInt16;
    if (attr == "int32")     return petInt32;
    if (attr == "int64")     return petInt64;
    if (attr == "uint8")     return petUint8;
    if (attr == "uint16")    return petUint16;
    if (attr == "uint32")    return petUint32;
    if (attr == "uint64")    return petUint64;

    if (attr == "float")     return petFloat;
    if (attr == "double")    return petDouble;

    return petInvalid;
}









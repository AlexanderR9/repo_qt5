/****************************************************************************
** Meta object code from reading C++ file 'modbusobj.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "modbusobj.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'modbusobj.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ModBusObj_t {
    QByteArrayData data[8];
    char stringdata0[134];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ModBusObj_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ModBusObj_t qt_meta_stringdata_ModBusObj = {
    {
QT_MOC_LITERAL(0, 0, 9), // "ModBusObj"
QT_MOC_LITERAL(1, 10, 17), // "slotErrorOccurred"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 20), // "QModbusDevice::Error"
QT_MOC_LITERAL(4, 50, 16), // "slotStateChanged"
QT_MOC_LITERAL(5, 67, 20), // "QModbusDevice::State"
QT_MOC_LITERAL(6, 88, 15), // "slotDataWritten"
QT_MOC_LITERAL(7, 104, 29) // "QModbusDataUnit::RegisterType"

    },
    "ModBusObj\0slotErrorOccurred\0\0"
    "QModbusDevice::Error\0slotStateChanged\0"
    "QModbusDevice::State\0slotDataWritten\0"
    "QModbusDataUnit::RegisterType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ModBusObj[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x09 /* Protected */,
       4,    1,   32,    2, 0x09 /* Protected */,
       6,    3,   35,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int, QMetaType::Int,    2,    2,    2,

       0        // eod
};

void ModBusObj::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ModBusObj *_t = static_cast<ModBusObj *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotErrorOccurred((*reinterpret_cast< QModbusDevice::Error(*)>(_a[1]))); break;
        case 1: _t->slotStateChanged((*reinterpret_cast< QModbusDevice::State(*)>(_a[1]))); break;
        case 2: _t->slotDataWritten((*reinterpret_cast< QModbusDataUnit::RegisterType(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QModbusDataUnit::RegisterType >(); break;
            }
            break;
        }
    }
}

const QMetaObject ModBusObj::staticMetaObject = {
    { &LSimpleObject::staticMetaObject, qt_meta_stringdata_ModBusObj.data,
      qt_meta_data_ModBusObj,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ModBusObj::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ModBusObj::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ModBusObj.stringdata0))
        return static_cast<void*>(const_cast< ModBusObj*>(this));
    return LSimpleObject::qt_metacast(_clname);
}

int ModBusObj::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LSimpleObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'ScenarioLauncher.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/widgets/ScenarioLauncher.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScenarioLauncher.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ScenarioLauncher_t {
    QByteArrayData data[14];
    char stringdata0[195];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ScenarioLauncher_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ScenarioLauncher_t qt_meta_stringdata_ScenarioLauncher = {
    {
QT_MOC_LITERAL(0, 0, 16), // "ScenarioLauncher"
QT_MOC_LITERAL(1, 17, 18), // "onScenarioSelected"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 12), // "onRunClicked"
QT_MOC_LITERAL(4, 50, 13), // "onStopClicked"
QT_MOC_LITERAL(5, 64, 15), // "onBrowseClicked"
QT_MOC_LITERAL(6, 80, 15), // "onProcessOutput"
QT_MOC_LITERAL(7, 96, 17), // "onProcessFinished"
QT_MOC_LITERAL(8, 114, 8), // "exitCode"
QT_MOC_LITERAL(9, 123, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(10, 144, 6), // "status"
QT_MOC_LITERAL(11, 151, 14), // "onProcessError"
QT_MOC_LITERAL(12, 166, 22), // "QProcess::ProcessError"
QT_MOC_LITERAL(13, 189, 5) // "error"

    },
    "ScenarioLauncher\0onScenarioSelected\0"
    "\0onRunClicked\0onStopClicked\0onBrowseClicked\0"
    "onProcessOutput\0onProcessFinished\0"
    "exitCode\0QProcess::ExitStatus\0status\0"
    "onProcessError\0QProcess::ProcessError\0"
    "error"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ScenarioLauncher[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    0,   53,    2, 0x08 /* Private */,
       7,    2,   54,    2, 0x08 /* Private */,
      11,    1,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,    8,   10,
    QMetaType::Void, 0x80000000 | 12,   13,

       0        // eod
};

void ScenarioLauncher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScenarioLauncher *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onScenarioSelected(); break;
        case 1: _t->onRunClicked(); break;
        case 2: _t->onStopClicked(); break;
        case 3: _t->onBrowseClicked(); break;
        case 4: _t->onProcessOutput(); break;
        case 5: _t->onProcessFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 6: _t->onProcessError((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ScenarioLauncher::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ScenarioLauncher.data,
    qt_meta_data_ScenarioLauncher,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ScenarioLauncher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScenarioLauncher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ScenarioLauncher.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ScenarioLauncher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

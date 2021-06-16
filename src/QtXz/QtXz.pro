NAME         = QtXz
TARGET       = $${NAME}
QT           = core
QT          -= gui

load(qt_build_config)
load(qt_module)

CONFIG(static,static|shared) {
# static version does not support Qt Script now
QT          -= script
# XZ Utils requires add a definition of LZMA_API_STATIC
DEFINES     += LZMA_API_STATIC
} else {
QT          += script
}

INCLUDEPATH += $${PWD}/../../include/QtXz
INCLUDEPATH += $${PWD}/../../include/QtXz/xz

HEADERS     += $${PWD}/../../include/QtXz/QtXz
HEADERS     += $${PWD}/../../include/QtXz/qtxz.h

SOURCES     += $${PWD}/qtxz.cpp

CONFIG(static,static|shared) {
} else {
SOURCES     += $${PWD}/ScriptableXz.cpp
}

OTHER_FILES += $${PWD}/../../include/$${NAME}/xz/*
OTHER_FILES += $${PWD}/../../include/$${NAME}/headers.pri

include ($${PWD}/../../doc/Qt/Qt.pri)

win32 {

CONFIG(debug, debug|release) {
LIBS        += -lliblzmad
} else {
LIBS        += -lliblzma
}

}

macx {

CONFIG(debug, debug|release) {
LIBS        += -lliblzmad
} else {
LIBS        += -lliblzma
}

}

unix {

CONFIG(debug, debug|release) {
LIBS        += -lliblzmad
} else {
LIBS        += -lliblzma
}

}

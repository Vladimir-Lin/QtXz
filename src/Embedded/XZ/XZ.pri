INCLUDEPATH += $${PWD}
INCLUDEPATH += $${PWD}/xz

HEADERS     += $${PWD}/qtxz.h

SOURCES     += $${PWD}/qtxz.cpp
SOURCES     += $${PWD}/ScriptableXz.cpp

CONFIG(debug, debug|release) {
LIBS        += -lliblzmad
} else {
LIBS        += -lliblzma
}

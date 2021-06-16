QT             = core
QT            -= gui
QT            += QtXz

CONFIG(debug, debug|release) {
TARGET         = xztoold
} else {
TARGET         = xztool
}

CONFIG        += console

TEMPLATE       = app

SOURCES       += $${PWD}/xztool.cpp

win32 {
RC_FILE        = $${PWD}/xztool.rc
OTHER_FILES   += $${PWD}/xztool.rc
OTHER_FILES   += $${PWD}/*.js
}

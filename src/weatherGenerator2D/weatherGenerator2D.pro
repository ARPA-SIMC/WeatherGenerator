#---------------------------------------------------------------------
#
#   weatherGenerator2D library
#   Spatial weather generator model
#
#   This project is part of ARPAE WG distribution
#
#   Code translated from the MulGets model available online on:
#   https://it.mathworks.com/matlabcentral/fileexchange/47537-multi-site-stochstic-weather-generator--mulgets-
#
#---------------------------------------------------------------------

QT  -= gui
QT  += xml

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/weatherGenerator2D
    } else {
        TARGET = release/weatherGenerator2D
    }
}
win32:{
    TARGET = weatherGenerator2D
}

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../../../agrolib/mathFunctions
INCLUDEPATH += ../../../agrolib/eispack
INCLUDEPATH += ../../../agrolib/crit3dDate
INCLUDEPATH += ../../../agrolib/meteo
INCLUDEPATH += ../../../agrolib/gis
INCLUDEPATH += ../../../agrolib/crit3dDate
INCLUDEPATH += ../../../agrolib/waterTable
INCLUDEPATH += ../weatherGenerator


SOURCES += wg2D.cpp \
    randomset.cpp \
    wg2D_precipitation.cpp \
    wg2D_temperature.cpp \
    wg2doutputmanagement.cpp

HEADERS += wg2D.h

#---------------------------------------------------------------------
#
#   weatherGenerator (1D) library
#   This project is part of ARPAE WG distribution
#
#   Based on Richardson, C. W. and D. A. Wright,
#   WGEN: A model for generating daily weather variables, USDA, 1984
#
#---------------------------------------------------------------------


QT      += sql xml
QT      -= gui


TEMPLATE = lib
CONFIG += staticlib

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/weatherGenerator
    } else {
        TARGET = release/weatherGenerator
    }
}
win32:{
    TARGET = weatherGenerator
}


INCLUDEPATH +=  ../../../agrolib/crit3dDate ../../../agrolib/mathFunctions  ../../../agrolib/utilities \
                ../../../agrolib/meteo ../../../agrolib/gis ../../../agrolib/waterTable

SOURCES += \
    timeUtility.cpp \
    parserXML.cpp \
    wgClimate.cpp \
    fileUtility.cpp \
    weatherGenerator.cpp

HEADERS += \
    timeUtility.h \
    parserXML.h \
    wgClimate.h \
    fileUtility.h \
    weatherGenerator.h

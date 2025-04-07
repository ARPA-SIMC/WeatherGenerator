#-----------------------------------------------------------------------
#
#   WG1D
#   Weather Generator 1D
#
#   generates synthetic daily time series of Tmin, Tmax, Prec (csv files)
#   and possibly Water table depth (if observed data are available)
#   Mode: CLIMATE, SEASONAL FORECAST, SCENARIO
#
#   This project is part of ARPA-SIMC/WeatherGenerator distribution
#
#------------------------------------------------------------------------


QT += core sql xml
QT -= gui

CONFIG += c++11

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/WG1D
    } else {
        TARGET = release/WG1D
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/WG1D
    } else {
        TARGET = release/WG1D
    }
}
win32:{
    TARGET = WG1D
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/meteo ../../agrolib/weatherGenerator ../../agrolib/waterTable

CONFIG(debug, debug|release) {
    LIBS += -L../../agrolib/waterTable/debug -lwaterTable
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/weatherGenerator/debug -lweatherGenerator
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate

} else {
    LIBS += -L../../agrolib/waterTable/release -lwaterTable
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/weatherGenerator/release -lweatherGenerator
    LIBS += -L../../agrolib/meteo/release -lmeteo
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
}


SOURCES += main.cpp \
    wgProject.cpp

HEADERS += \
    wgProject.h


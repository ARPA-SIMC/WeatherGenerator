#-------------------------------------------------------------------
#
#   Test_WG2D_Eraclito
#   Weather Generator (2D) test
#
#   This project is part of CRITERIA3D distribution
#
#-------------------------------------------------------------------
QT  += sql xml
QT  -= gui

TEMPLATE = app
CONFIG += console

DEFINES += _CRT_SECURE_NO_WARNINGS

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/Test_WG2D_Eraclito
    } else {
        TARGET = release/Test_WG2D_Eraclito
    }
}
win32:{
    TARGET = Test_WG2D_Eraclito
}


INCLUDEPATH += ../../agrolib/weatherGenerator ../../agrolib/weatherGenerator2D ../../agrolib/mathFunctions ../../agrolib/eispack \
                ../../agrolib/crit3dDate ../../agrolib/meteo ../../agrolib/gis ../../agrolib/dbMeteoGrid ../../agrolib/utilities


CONFIG(release, debug|release) {
    LIBS += -L../../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/weatherGenerator2D/release -lweatherGenerator2D
    LIBS += -L../../agrolib/weatherGenerator/release -lweatherGenerator
    LIBS += -L../../agrolib/eispack/release -leispack
    LIBS += -L../../agrolib/meteo/release -lmeteo
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
} else {
    LIBS += -L../../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/weatherGenerator2D/debug -lweatherGenerator2D
    LIBS += -L../../agrolib/weatherGenerator/debug -lweatherGenerator
    LIBS += -L../../agrolib/eispack/debug -leispack
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions
}



SOURCES += \
    main.cpp


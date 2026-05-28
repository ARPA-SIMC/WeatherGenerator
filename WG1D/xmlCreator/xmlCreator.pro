QT += core sql xml
QT -= gui

CONFIG += c++11 c++14 c++17 cmdline


INCLUDEPATH +=  ../../../agrolib/crit3dDate ../../../agrolib/mathFunctions ../../../agrolib/utilities

CONFIG(debug, debug|release) {
    LIBS += -L../../../agrolib/utilities/debug -lutilities
    LIBS += -L../../../agrolib/mathFunctions/debug -lmathFunctions
    LIBS += -L../../../agrolib/crit3dDate/debug -lcrit3dDate

} else {
    LIBS += -L../../../agrolib/utilities/release -lutilities
    LIBS += -L../../../agrolib/mathFunctions/release -lmathFunctions
    LIBS += -L../../../agrolib/crit3dDate/release -lcrit3dDate
}

SOURCES += \
        functionsIO.cpp \
        main.cpp \
        main_old.cpp \
        xmlProject.cpp

HEADERS += \
    functionsIO.h \
    xmlProject.h


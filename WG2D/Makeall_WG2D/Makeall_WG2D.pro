TEMPLATE = subdirs

SUBDIRS =   ../../../agrolib/crit3dDate ../../../agrolib/mathFunctions ../../../agrolib/eispack  \
            ../../../agrolib/meteo ../../../agrolib/gis  \
            ../../../agrolib/utilities  ../../../agrolib/dbMeteoGrid  \
            ../../src/weatherGenerator ../../src/weatherGenerator2D  \
            ../TestWG2D

CONFIG += ordered

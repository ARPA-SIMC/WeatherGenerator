#include <QCoreApplication>
#include <QDir>
#include <QString>

#include <iostream>
#include <vector>
#include <iostream>
#include <filesystem>

#include "functionsIO.h"
#include "xmlProject.h"
#include "utilities.h"


// uncomment to execute test
#define TEST


void usage()
{
    std::cout << std::endl << "Usage:" << std::endl
              << "xmlCreator <projectName.ini>" << std::endl;
    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);

    std::cout << "XML scenarios creator  V1.0\n";

    XmlProject myProject;

    QString dataPath, settingsFileName;
    if (! searchDataPath(&dataPath))
        return -1;

    #ifdef TEST
        settingsFileName = dataPath + "TEST_xmlCreator/testXmlCreator.ini";
    #else
        if (argc > 1)
            settingsFileName = argv[1];
        else
        {
            usage();
            return 0;
        }
    #endif


    // read settings
    if (! myProject.readSettings(settingsFileName))
        return -1;


    return 0;
}


/**
 * @file drywell_simple_example.cpp
 * @brief Simplified example using generateAll() method
 */

#include <QApplication>
#include "occtviewer.h"
#include "occtdrywellsystem.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    double ft = 0.3048;



    // Create drywell system
    OcctDrywellSystem* drywell = new OcctDrywellSystem(
        2.0*ft,   // wellRadius
        16*ft,   // chamberDepth
        24*ft,   // aggregateDepth
        20.0f,   // domainRadius
        142*ft,  // depthToGroundwater
        12,      // nr
        12,      // nz_w
        30       // nz_g
        );

    // Generate everything with ONE call!
    drywell->generateAll();

    // Create viewer and display
    OcctViewer viewer;
    viewer.setObjectSet(drywell->createObjectSet());
    viewer.resize(1200, 800);
    viewer.setWindowTitle("Drywell System - Simple Example");
    viewer.show();

    return app.exec();
}



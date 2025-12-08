/**
 * @file occt_drywell_example.cpp
 * @brief Complete example of visualizing a drywell system with OpenCASCADE
 */

#include "occtdrywellsystem.h"
#include "occtgeo3dobjectset.h"
#include "occtcylinderobject.h"
#include "occtviewer.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    qDebug() << "\n=== Creating OCCT Drywell System ===\n";

    // ============================================
    // DRYWELL PARAMETERS
    // ============================================
    float wellRadius = 0.5f;          // 0.5m well radius
    float chamberDepth = 1.0f;        // 1.0m from surface to aggregate
    float aggregateDepth = 2.0f;      // 2.0m aggregate zone thickness
    float domainRadius = 5.0f;        // 5.0m domain radius
    float depthToGroundwater = 10.0f; // 10.0m to groundwater
    int nr = 8;                       // 8 radial layers
    int nz_w = 6;                     // 6 vertical layers in aggregate
    int nz_g = 5;                     // 5 vertical layers below (future use)

    qDebug() << "Drywell Parameters:";
    qDebug() << "  - Well radius:" << wellRadius << "m";
    qDebug() << "  - Chamber depth:" << chamberDepth << "m";
    qDebug() << "  - Aggregate depth:" << aggregateDepth << "m";
    qDebug() << "  - Domain radius:" << domainRadius << "m";
    qDebug() << "  - Radial cells:" << nr;
    qDebug() << "  - Vertical cells:" << nz_w;
    qDebug() << "";

    // ============================================
    // CREATE SCENE
    // ============================================
    OcctGeo3DObjectSet* scene = new OcctGeo3DObjectSet();

    // ============================================
    // 1. WELL CHAMBER (semi-transparent grey cylinder)
    // ============================================
    float chamberHeight = chamberDepth;
    float chamberCenterZ = -chamberHeight / 2.0f;

    OcctCylinderObject* wellChamber = new OcctCylinderObject(wellRadius, chamberHeight);
    wellChamber->setPosition(0.0f, 0.0f, chamberCenterZ);
    wellChamber->setDiffuseColor(QColor(180, 180, 180));  // Light grey
    wellChamber->setAmbientColor(QColor(100, 100, 100));  // Darker grey
    wellChamber->setOpacity(0.6f);
    wellChamber->setShowEdges(true);  // Show edges!
    wellChamber->setEdgeColor(Qt::darkGray);
    scene->addObject("wellChamber", wellChamber);

    qDebug() << "Created well chamber:";
    qDebug() << "  - Radius:" << wellRadius << "m";
    qDebug() << "  - Elevation: 0m to" << -chamberDepth << "m";
    qDebug() << "  - Color: Light grey (semi-transparent with edges)";

    // ============================================
    // 2. AGGREGATE ZONE (tubes using OcctDrywellSystem)
    // ============================================
    OcctDrywellSystem drywell(wellRadius, chamberDepth, aggregateDepth,
                              domainRadius, depthToGroundwater,
                              nr, nz_w, nz_g);

    // Generate all tubes
    drywell.generateAggregateZone();

    // Get the tubes and customize their appearance
    const QVector<OcctTubeObject*>& tubes = drywell.getTubes();
    for (int i = 0; i < tubes.size(); ++i) {
        OcctTubeObject* tube = tubes[i];

        // Calculate layer indices
        int radialLayer = i / nz_w;
        int verticalLayer = i % nz_w;

        // Color gradient: Tan/beige for gravel aggregate
        // Gets slightly darker with depth
        float depthRatio = static_cast<float>(verticalLayer) / (nz_w - 1);
        float radialRatio = static_cast<float>(radialLayer) / (nr - 1);

        QColor aggregateColor;
        aggregateColor.setHsvF(
            0.1f,                           // Hue: tan/beige
            0.35f - depthRatio * 0.1f,      // Saturation: slightly less with depth
            0.85f - depthRatio * 0.2f       // Value: darker with depth
            );

        tube->setDiffuseColor(aggregateColor);
        tube->setAmbientColor(aggregateColor.darker(140));

        // Opacity gradient: inner layers more transparent
        float opacity = 0.35f + radialRatio * 0.45f;
        tube->setOpacity(opacity);

        // Show edges on aggregate tubes
        tube->setShowEdges(true);
        tube->setEdgeColor(QColor(100, 80, 60));  // Brown edges
        tube->setEdgeWidth(1.0f);
    }

    // Add all tubes to the scene - ONE LINE!
    drywell.addToObjectSet(scene);

    qDebug() << "\nCreated aggregate zone:";
    qDebug() << "  - Total tubes:" << drywell.getTubeCount();
    qDebug() << "  - Elevation:" << -chamberDepth << "m to" << -(chamberDepth + aggregateDepth) << "m";
    qDebug() << "  - Color: Tan/beige gravel (gradient by depth)";
    qDebug() << "  - Opacity: 0.35 (inner) to 0.8 (outer)";
    qDebug() << "  - Edges: Shown with brown color";

    // ============================================
    // 3. UPPER SOIL LAYER (around well chamber)
    // ============================================
    OcctTubeObject* upperSoil = new OcctTubeObject(wellRadius, domainRadius, chamberHeight);
    upperSoil->setPosition(0.0f, 0.0f, chamberCenterZ);
    upperSoil->setDiffuseColor(QColor(139, 90, 43));   // Soil brown
    upperSoil->setAmbientColor(QColor(90, 60, 30));    // Darker brown
    upperSoil->setOpacity(0.25f);
    upperSoil->setShowEdges(false);  // No edges on soil
    scene->addObject("upperSoil", upperSoil);

    qDebug() << "\nCreated upper soil layer:";
    qDebug() << "  - Inner radius:" << wellRadius << "m, Outer radius:" << domainRadius << "m";
    qDebug() << "  - Elevation: 0m to" << -chamberDepth << "m";
    qDebug() << "  - Color: Brown (semi-transparent)";

    // ============================================
    // 4. OUTER SOIL AROUND AGGREGATE
    // ============================================
    float aggregateCenterZ = -chamberDepth - aggregateDepth / 2.0f;
    float outerSoilRadius = domainRadius * 1.5f;

    OcctTubeObject* outerSoil = new OcctTubeObject(domainRadius, outerSoilRadius, aggregateDepth);
    outerSoil->setPosition(0.0f, 0.0f, aggregateCenterZ);
    outerSoil->setDiffuseColor(QColor(120, 80, 50));   // Reddish-brown
    outerSoil->setAmbientColor(QColor(80, 50, 30));    // Darker
    outerSoil->setOpacity(0.3f);
    scene->addObject("outerSoil", outerSoil);

    qDebug() << "\nCreated outer soil (around aggregate):";
    qDebug() << "  - Inner radius:" << domainRadius << "m, Outer radius:" << outerSoilRadius << "m";
    qDebug() << "  - Elevation:" << -chamberDepth << "m to" << -(chamberDepth + aggregateDepth) << "m";

    // ============================================
    // 5. LOWER SOIL (below aggregate to groundwater)
    // ============================================
    float belowAggregateDepth = depthToGroundwater - (chamberDepth + aggregateDepth);
    float lowerSoilCenterZ = -(chamberDepth + aggregateDepth) - belowAggregateDepth / 2.0f;

    OcctCylinderObject* lowerSoil = new OcctCylinderObject(outerSoilRadius, belowAggregateDepth);
    lowerSoil->setPosition(0.0f, 0.0f, lowerSoilCenterZ);
    lowerSoil->setDiffuseColor(QColor(110, 70, 45));   // Dark reddish-brown
    lowerSoil->setAmbientColor(QColor(70, 40, 25));    // Even darker
    lowerSoil->setOpacity(0.2f);
    scene->addObject("lowerSoil", lowerSoil);

    qDebug() << "\nCreated lower soil layer:";
    qDebug() << "  - Radius:" << outerSoilRadius << "m";
    qDebug() << "  - Elevation:" << -(chamberDepth + aggregateDepth) << "m to" << -depthToGroundwater << "m";

    // ============================================
    // 6. GROUNDWATER LEVEL (thin disc)
    // ============================================
    OcctCylinderObject* groundwater = new OcctCylinderObject(outerSoilRadius, 0.1f);
    groundwater->setPosition(0.0f, 0.0f, -depthToGroundwater);
    groundwater->setDiffuseColor(QColor(100, 150, 255));  // Light blue
    groundwater->setAmbientColor(QColor(50, 100, 200));   // Darker blue
    groundwater->setOpacity(0.5f);
    groundwater->setShowEdges(true);
    groundwater->setEdgeColor(Qt::blue);
    scene->addObject("groundwater", groundwater);

    qDebug() << "\nCreated groundwater level indicator:";
    qDebug() << "  - Elevation:" << -depthToGroundwater << "m";
    qDebug() << "  - Color: Light blue with edges";

    // ============================================
    // SAVE AND DISPLAY
    // ============================================
    qDebug() << "\n=== Saving Scene ===";
    QString fileName = "occt_drywell_scene.json";
    bool saveSuccess = scene->saveToFile(fileName);
    qDebug() << "Save to" << fileName << ":" << (saveSuccess ? "SUCCESS" : "FAILED");

    qDebug() << "\n=== Opening OCCT Viewer ===";
    OcctViewer viewer;
    viewer.setObjectSet(scene);
    viewer.show();

    qDebug() << "\nScene ready! Click 'Show 3D Objects' to visualize.";
    qDebug() << "\nYou should see:";
    qDebug() << "  ✓ Grey well chamber with edges (0 to -1m)";
    qDebug() << "  ✓ Tan/beige aggregate tubes in concentric layers WITH EDGES (-1 to -3m)";
    qDebug() << "  ✓ Brown soil surrounding the well and aggregate";
    qDebug() << "  ✓ Darker soil below extending to groundwater (-3 to -10m)";
    qDebug() << "  ✓ Blue groundwater level indicator at -10m";
    qDebug() << "\nMouse Controls:";
    qDebug() << "  - Left button + drag: Rotate view";
    qDebug() << "  - Middle button + drag: Pan view";
    qDebug() << "  - Mouse wheel: Zoom in/out";
    qDebug() << "\nThe aggregate zone contains" << drywell.getTubeCount() << "individual tube cells with edges!\n";

    return app.exec();

}

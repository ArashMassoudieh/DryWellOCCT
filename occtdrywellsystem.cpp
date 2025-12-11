/**
 * @file occtdrywellsystem.cpp
 * @brief Implementation of the OcctDrywellSystem class
 */

#include "occtdrywellsystem.h"
#include "occtgeo3dobjectset.h"
#include "occtcylinderobject.h"
#include <QJsonArray>
#include <cmath>

OcctDrywellSystem::OcctDrywellSystem(float wellRadius,
                                     float chamberDepth,
                                     float aggregateDepth,
                                     float domainRadius,
                                     float depthToGroundwater,
                                     int nr,
                                     int nz_w,
                                     int nz_g)
    : m_wellRadius(wellRadius)
    , m_chamberDepth(chamberDepth)
    , m_aggregateDepth(aggregateDepth)
    , m_domainRadius(domainRadius)
    , m_depthToGroundwater(depthToGroundwater)
    , m_nr(nr)
    , m_nz_w(nz_w)
    , m_nz_g(nz_g)
    , m_chamberCylinder(nullptr)
    , m_aggregateWellCylinder(nullptr)
    , m_belowWellCylinder(nullptr)
{
}

OcctDrywellSystem::~OcctDrywellSystem()
{
    clear();
}

void OcctDrywellSystem::generateAggregateZone()
{
    // Clear any existing tubes
    clear();

    // Calculate radial and vertical cell sizes
    float dr = (m_domainRadius - m_wellRadius) / m_nr;  // Radial increment
    float dz = m_aggregateDepth / m_nz_w;               // Vertical increment

    // Reserve space for efficiency
    m_tubes.reserve(m_nr * m_nz_w);

    // Create tubes in nested loops
    // i: radial index (0 to nr-1) - from well to domain boundary
    // j: vertical index (0 to nz_w-1) - from top to bottom of aggregate zone
    for (int i = 0; i < m_nr; ++i) {
        for (int j = 0; j < m_nz_w; ++j) {
            createTube(i, j);
        }
    }
}

void OcctDrywellSystem::generateBelowWellZone()
{
    // Clear any existing below-well tubes
    qDeleteAll(m_belowWellTubes);
    m_belowWellTubes.clear();

    // Calculate radial and vertical cell sizes
    float dr = (m_domainRadius - m_wellRadius) / m_nr;  // Radial increment
    float dz = (m_depthToGroundwater - (m_chamberDepth + m_aggregateDepth)) / m_nz_g;

    // Reserve space for efficiency
    m_belowWellTubes.reserve(m_nr * m_nz_g);

    // Create tubes in nested loops
    // i: radial index (0 to nr-1) - from well to domain boundary
    // j: vertical index (0 to nz_g-1) - from top of zone to groundwater
    for (int i = 0; i < m_nr; ++i) {
        for (int j = 0; j < m_nz_g; ++j) {
            createBelowWellTube(i, j);
        }
    }
}

void OcctDrywellSystem::generateWellCylinders()
{
    // Clean up existing cylinders
    delete m_chamberCylinder;
    delete m_aggregateWellCylinder;
    delete m_belowWellCylinder;

    // 1. Chamber cylinder (from z=0 to z=-chamberDepth)
    // Light gray color for empty chamber - sticks out above tubes
    m_chamberCylinder = new OcctCylinderObject(m_wellRadius, m_chamberDepth);
    m_chamberCylinder->setPosition(0.0f, 0.0f, -m_chamberDepth / 2.0f);
    m_chamberCylinder->setDiffuseColor(QColor(180, 180, 180));  // Medium gray
    m_chamberCylinder->setOpacity(0.7f);  // More visible than before
    m_chamberCylinder->setShowEdges(true);

    // 2. Aggregate zone well cylinder (from z=-chamberDepth to z=-(chamberDepth+aggregateDepth))
    // Orange/brown color matching aggregate zone
    m_aggregateWellCylinder = new OcctCylinderObject(m_wellRadius, m_aggregateDepth);
    float aggZ = -m_chamberDepth - m_aggregateDepth / 2.0f;
    m_aggregateWellCylinder->setPosition(0.0f, 0.0f, aggZ);
    m_aggregateWellCylinder->setDiffuseColor(QColor::fromHsvF(0.08f, 0.7f, 0.75f));  // Match aggregate
    m_aggregateWellCylinder->setOpacity(0.6f);
    m_aggregateWellCylinder->setShowEdges(true);

    // 3. Below-well cylinder (from z=-(chamberDepth+aggregateDepth) to z=-depthToGroundwater)
    // Blue/green color matching soil zone
    float belowWellHeight = m_depthToGroundwater - (m_chamberDepth + m_aggregateDepth);
    m_belowWellCylinder = new OcctCylinderObject(m_wellRadius, belowWellHeight);
    float belowZ = -(m_chamberDepth + m_aggregateDepth) - belowWellHeight / 2.0f;
    m_belowWellCylinder->setPosition(0.0f, 0.0f, belowZ);
    m_belowWellCylinder->setDiffuseColor(QColor::fromHsvF(0.50f, 0.5f, 0.65f));  // Match below-well
    m_belowWellCylinder->setOpacity(0.6f);
    m_belowWellCylinder->setShowEdges(true);
}

void OcctDrywellSystem::generateAll()
{
    // Generate all components in correct order
    generateAggregateZone();
    generateBelowWellZone();
    generateWellCylinders();
}

void OcctDrywellSystem::createTube(int radialIndex, int verticalIndex)
{
    // Calculate radial and vertical increments
    float dr = (m_domainRadius - m_wellRadius) / m_nr;
    float dz = m_aggregateDepth / m_nz_w;

    // Calculate tube dimensions
    float innerRadius = m_wellRadius + radialIndex * dr;
    float outerRadius = m_wellRadius + (radialIndex + 1) * dr;
    float height = dz;

    // Calculate tube position
    // Tubes start at z = -chamberDepth (top of aggregate zone)
    // and extend down to z = -(chamberDepth + aggregateDepth)
    // Each tube is centered at its layer
    float z_top = -m_chamberDepth - verticalIndex * dz;
    float z_center = z_top - dz / 2.0f;

    // Create the tube object
    OcctTubeObject* tube = new OcctTubeObject(innerRadius, outerRadius, height);

    // Position the tube (x=0, y=0, z=center position)
    // OpenCASCADE uses Z-axis as vertical by default
    tube->setPosition(0.0f, 0.0f, z_center);

    // Color scheme for aggregate zone: warm orange/brown tones
    // Each radial layer has a base hue, with slight variation per vertical cell
    float baseHue = 0.08f;  // Orange base
    float hueRange = 0.08f; // Range from orange to brown
    float radialHue = baseHue + (static_cast<float>(radialIndex) / m_nr) * hueRange;

    // Add slight variation within the layer based on vertical position
    float verticalVariation = (static_cast<float>(verticalIndex) / m_nz_w) * 0.03f - 0.015f;
    float finalHue = radialHue + verticalVariation;

    QColor layerColor;
    layerColor.setHsvF(finalHue, 0.7f, 0.75f);  // Warm, saturated colors
    tube->setDiffuseColor(layerColor);

    // Make it transparent
    tube->setOpacity(0.6f);

    // Show edges for better visualization
    tube->setShowEdges(true);

    // Add to collection
    m_tubes.append(tube);
}

void OcctDrywellSystem::createBelowWellTube(int radialIndex, int verticalIndex)
{
    // Calculate radial and vertical increments
    float dr = (m_domainRadius - m_wellRadius) / m_nr;
    float dz = (m_depthToGroundwater - (m_chamberDepth + m_aggregateDepth)) / m_nz_g;

    // Calculate tube dimensions
    float innerRadius = m_wellRadius + radialIndex * dr;
    float outerRadius = m_wellRadius + (radialIndex + 1) * dr;
    float height = dz;

    // Calculate tube position
    // Tubes start at z = -(chamberDepth + aggregateDepth) (top of below-well zone)
    // and extend down to z = -depthToGroundwater
    // Each tube is centered at its layer
    float z_top = -(m_chamberDepth + m_aggregateDepth) - verticalIndex * dz;
    float z_center = z_top - dz / 2.0f;

    // Create the tube object
    OcctTubeObject* tube = new OcctTubeObject(innerRadius, outerRadius, height);

    // Position the tube (x=0, y=0, z=center position)
    tube->setPosition(0.0f, 0.0f, z_center);

    // Color scheme for below-well zone: cool blue/green tones (soil colors)
    // Each radial layer has a base hue, with slight variation per vertical cell
    float baseHue = 0.45f;  // Cyan/green base
    float hueRange = 0.15f; // Range from cyan to green
    float radialHue = baseHue + (static_cast<float>(radialIndex) / m_nr) * hueRange;

    // Add slight variation within the layer based on vertical position
    float verticalVariation = (static_cast<float>(verticalIndex) / m_nz_g) * 0.03f - 0.015f;
    float finalHue = radialHue + verticalVariation;

    QColor layerColor;
    layerColor.setHsvF(finalHue, 0.5f, 0.65f);  // Cooler, less saturated soil colors
    tube->setDiffuseColor(layerColor);

    // Make it transparent
    tube->setOpacity(0.6f);

    // Show edges for better visualization
    tube->setShowEdges(true);

    // Add to below-well collection
    m_belowWellTubes.append(tube);
}

void OcctDrywellSystem::displayInContext(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    // Display well cylinders first (so they appear behind tubes)
    if (m_chamberCylinder) {
        m_chamberCylinder->displayInContext(context);
    }
    if (m_aggregateWellCylinder) {
        m_aggregateWellCylinder->displayInContext(context);
    }
    if (m_belowWellCylinder) {
        m_belowWellCylinder->displayInContext(context);
    }

    // Display all aggregate zone tubes
    for (OcctTubeObject* tube : m_tubes) {
        tube->displayInContext(context);
    }

    // Display all below-well zone tubes
    for (OcctTubeObject* tube : m_belowWellTubes) {
        tube->displayInContext(context);
    }

    context->UpdateCurrentViewer();
}

void OcctDrywellSystem::eraseFromContext(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    // Erase well cylinders
    if (m_chamberCylinder) {
        m_chamberCylinder->eraseFromContext(context);
    }
    if (m_aggregateWellCylinder) {
        m_aggregateWellCylinder->eraseFromContext(context);
    }
    if (m_belowWellCylinder) {
        m_belowWellCylinder->eraseFromContext(context);
    }

    // Erase all aggregate zone tubes
    for (OcctTubeObject* tube : m_tubes) {
        tube->eraseFromContext(context);
    }

    // Erase all below-well zone tubes
    for (OcctTubeObject* tube : m_belowWellTubes) {
        tube->eraseFromContext(context);
    }

    context->UpdateCurrentViewer();
}

OcctGeo3DObjectSet* OcctDrywellSystem::createObjectSet() const
{
    OcctGeo3DObjectSet* objectSet = new OcctGeo3DObjectSet();
    addToObjectSet(objectSet);
    return objectSet;
}

void OcctDrywellSystem::addToObjectSet(OcctGeo3DObjectSet* objectSet) const
{
    if (!objectSet) {
        return;
    }

    // Add well cylinders
    if (m_chamberCylinder) {
        objectSet->addObject("well_chamber", m_chamberCylinder);
    }
    if (m_aggregateWellCylinder) {
        objectSet->addObject("well_aggregate", m_aggregateWellCylinder);
    }
    if (m_belowWellCylinder) {
        objectSet->addObject("well_below", m_belowWellCylinder);
    }

    // Add all aggregate zone tubes to the object set
    for (int i = 0; i < m_tubes.size(); ++i) {
        OcctTubeObject* tube = m_tubes[i];

        // Calculate layer indices
        int radialIndex = i / m_nz_w;
        int verticalIndex = i % m_nz_w;

        // Create unique name for each tube
        QString tubeName = QString("tube_r%1_z%2").arg(radialIndex).arg(verticalIndex);

        // Add tube to object set
        objectSet->addObject(tubeName, tube);
    }

    // Add all below-well zone tubes to the object set
    for (int i = 0; i < m_belowWellTubes.size(); ++i) {
        OcctTubeObject* tube = m_belowWellTubes[i];

        // Calculate layer indices
        int radialIndex = i / m_nz_g;
        int verticalIndex = i % m_nz_g;

        // Create unique name for each below-well tube
        QString tubeName = QString("tube_below_r%1_z%2").arg(radialIndex).arg(verticalIndex);

        // Add tube to object set
        objectSet->addObject(tubeName, tube);
    }
}

const QVector<OcctTubeObject*>& OcctDrywellSystem::getTubes() const
{
    return m_tubes;
}

const QVector<OcctTubeObject*>& OcctDrywellSystem::getBelowWellTubes() const
{
    return m_belowWellTubes;
}

OcctTubeObject* OcctDrywellSystem::getTube(int radialIndex, int verticalIndex) const
{
    if (radialIndex < 0 || radialIndex >= m_nr ||
        verticalIndex < 0 || verticalIndex >= m_nz_w) {
        return nullptr;
    }

    int index = getTubeIndex(radialIndex, verticalIndex);
    if (index >= 0 && index < m_tubes.size()) {
        return m_tubes[index];
    }

    return nullptr;
}

OcctTubeObject* OcctDrywellSystem::getBelowWellTube(int radialIndex, int verticalIndex) const
{
    if (radialIndex < 0 || radialIndex >= m_nr ||
        verticalIndex < 0 || verticalIndex >= m_nz_g) {
        return nullptr;
    }

    int index = getBelowWellTubeIndex(radialIndex, verticalIndex);
    if (index >= 0 && index < m_belowWellTubes.size()) {
        return m_belowWellTubes[index];
    }

    return nullptr;
}

int OcctDrywellSystem::getTubeCount() const
{
    return m_tubes.size() + m_belowWellTubes.size();
}

void OcctDrywellSystem::clear()
{
    qDeleteAll(m_tubes);
    m_tubes.clear();

    qDeleteAll(m_belowWellTubes);
    m_belowWellTubes.clear();

    delete m_chamberCylinder;
    m_chamberCylinder = nullptr;

    delete m_aggregateWellCylinder;
    m_aggregateWellCylinder = nullptr;

    delete m_belowWellCylinder;
    m_belowWellCylinder = nullptr;
}

float OcctDrywellSystem::getRadialCellSize() const
{
    return (m_domainRadius - m_wellRadius) / m_nr;
}

float OcctDrywellSystem::getVerticalCellSize() const
{
    return m_aggregateDepth / m_nz_w;
}

float OcctDrywellSystem::getBelowWellVerticalCellSize() const
{
    return (m_depthToGroundwater - (m_chamberDepth + m_aggregateDepth)) / m_nz_g;
}

int OcctDrywellSystem::getTubeIndex(int radialIndex, int verticalIndex) const
{
    // Tubes are stored in row-major order: for each radial layer, all vertical cells
    return radialIndex * m_nz_w + verticalIndex;
}

int OcctDrywellSystem::getBelowWellTubeIndex(int radialIndex, int verticalIndex) const
{
    // Below-well tubes are stored in row-major order: for each radial layer, all vertical cells
    return radialIndex * m_nz_g + verticalIndex;
}

QJsonObject OcctDrywellSystem::toJson() const
{
    QJsonObject json;

    // System parameters
    json["wellRadius"] = m_wellRadius;
    json["chamberDepth"] = m_chamberDepth;
    json["aggregateDepth"] = m_aggregateDepth;
    json["domainRadius"] = m_domainRadius;
    json["depthToGroundwater"] = m_depthToGroundwater;
    json["nr"] = m_nr;
    json["nz_w"] = m_nz_w;
    json["nz_g"] = m_nz_g;

    // Calculated parameters
    json["radialCellSize"] = getRadialCellSize();
    json["verticalCellSize"] = getVerticalCellSize();
    json["tubeCount"] = getTubeCount();

    // Export aggregate zone tubes
    QJsonArray tubesArray;
    for (const OcctTubeObject* tube : m_tubes) {
        tubesArray.append(tube->toJson());
    }
    json["tubes"] = tubesArray;

    // Export below-well zone tubes
    QJsonArray belowWellTubesArray;
    for (const OcctTubeObject* tube : m_belowWellTubes) {
        belowWellTubesArray.append(tube->toJson());
    }
    json["belowWellTubes"] = belowWellTubesArray;

    return json;
}

bool OcctDrywellSystem::fromJson(const QJsonObject& json)
{
    // Clear existing tubes
    clear();

    // Load system parameters
    if (!json.contains("wellRadius") || !json.contains("chamberDepth") ||
        !json.contains("aggregateDepth") || !json.contains("domainRadius") ||
        !json.contains("depthToGroundwater") || !json.contains("nr") ||
        !json.contains("nz_w") || !json.contains("nz_g")) {
        return false;
    }

    m_wellRadius = json["wellRadius"].toDouble();
    m_chamberDepth = json["chamberDepth"].toDouble();
    m_aggregateDepth = json["aggregateDepth"].toDouble();
    m_domainRadius = json["domainRadius"].toDouble();
    m_depthToGroundwater = json["depthToGroundwater"].toDouble();
    m_nr = json["nr"].toInt();
    m_nz_w = json["nz_w"].toInt();
    m_nz_g = json["nz_g"].toInt();

    // Load aggregate zone tubes if they exist
    if (json.contains("tubes")) {
        QJsonArray tubesArray = json["tubes"].toArray();
        m_tubes.reserve(tubesArray.size());

        for (const QJsonValue& tubeValue : tubesArray) {
            QJsonObject tubeJson = tubeValue.toObject();
            OcctTubeObject* tube = new OcctTubeObject();
            if (tube->fromJson(tubeJson)) {
                m_tubes.append(tube);
            } else {
                delete tube;
            }
        }
    }

    // Load below-well zone tubes if they exist
    if (json.contains("belowWellTubes")) {
        QJsonArray belowWellTubesArray = json["belowWellTubes"].toArray();
        m_belowWellTubes.reserve(belowWellTubesArray.size());

        for (const QJsonValue& tubeValue : belowWellTubesArray) {
            QJsonObject tubeJson = tubeValue.toObject();
            OcctTubeObject* tube = new OcctTubeObject();
            if (tube->fromJson(tubeJson)) {
                m_belowWellTubes.append(tube);
            } else {
                delete tube;
            }
        }
    }

    return true;
}

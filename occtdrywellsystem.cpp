/**
 * @file occtdrywellsystem.cpp
 * @brief Implementation of the OcctDrywellSystem class
 */

#include "occtdrywellsystem.h"
#include "occtgeo3dobjectset.h"
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

    // Optional: Set different colors based on layer for visualization
    // You can customize this based on your needs
    float colorGradient = static_cast<float>(radialIndex) / m_nr;
    QColor layerColor;
    layerColor.setHsvF(0.1f + colorGradient * 0.3f, 0.6f, 0.8f);
    tube->setDiffuseColor(layerColor);

    // Add to collection
    m_tubes.append(tube);
}

void OcctDrywellSystem::displayInContext(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    // Display all tubes
    for (OcctTubeObject* tube : m_tubes) {
        tube->displayInContext(context);
    }

    context->UpdateCurrentViewer();
}

void OcctDrywellSystem::eraseFromContext(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    // Erase all tubes
    for (OcctTubeObject* tube : m_tubes) {
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

    // Add all tubes to the object set
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
}

const QVector<OcctTubeObject*>& OcctDrywellSystem::getTubes() const
{
    return m_tubes;
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

int OcctDrywellSystem::getTubeCount() const
{
    return m_tubes.size();
}

void OcctDrywellSystem::clear()
{
    qDeleteAll(m_tubes);
    m_tubes.clear();
}

float OcctDrywellSystem::getRadialCellSize() const
{
    return (m_domainRadius - m_wellRadius) / m_nr;
}

float OcctDrywellSystem::getVerticalCellSize() const
{
    return m_aggregateDepth / m_nz_w;
}

int OcctDrywellSystem::getTubeIndex(int radialIndex, int verticalIndex) const
{
    // Tubes are stored in row-major order: for each radial layer, all vertical cells
    return radialIndex * m_nz_w + verticalIndex;
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
    json["tubeCount"] = m_tubes.size();

    // Export all tubes
    QJsonArray tubesArray;
    for (const OcctTubeObject* tube : m_tubes) {
        tubesArray.append(tube->toJson());
    }
    json["tubes"] = tubesArray;

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

    // Load tubes if they exist
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

    return true;
}

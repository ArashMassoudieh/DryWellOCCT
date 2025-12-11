/**
 * @file occtdrywellsystem.h
 * @brief Header file for the OcctDrywellSystem class
 */

#ifndef OCCTDRYWELLSYSTEM_H
#define OCCTDRYWELLSYSTEM_H

#include <QVector>
#include <QJsonObject>
#include <AIS_InteractiveContext.hxx>
#include "occttubeobject.h"

// Forward declarations
class OcctGeo3DObjectSet;
class OcctCylinderObject;

/**
 * @class OcctDrywellSystem
 * @brief A class that creates and manages a 3D representation of a drywell system using OpenCASCADE
 *
 * The OcctDrywellSystem creates a collection of OcctTubeObjects arranged in a cylindrical
 * grid pattern to represent the aggregate zone of a drywell infiltration system.
 */
class OcctDrywellSystem
{
public:
    /**
     * @brief Constructor with system parameters
     * @param wellRadius Radius of the well (R_w)
     * @param chamberDepth Depth from surface to top of aggregate zone (D_c)
     * @param aggregateDepth Thickness of the aggregate zone (D_a)
     * @param domainRadius Outer radius of the domain (R_d)
     * @param depthToGroundwater Depth from surface to groundwater table
     * @param nr Number of cells in radial direction
     * @param nz_w Number of cells in vertical direction adjacent to well
     * @param nz_g Number of cells in vertical direction below well
     */
    OcctDrywellSystem(float wellRadius,
                      float chamberDepth,
                      float aggregateDepth,
                      float domainRadius,
                      float depthToGroundwater,
                      int nr,
                      int nz_w,
                      int nz_g);

    /**
     * @brief Destructor - cleans up all created tube objects
     */
    ~OcctDrywellSystem();

    /**
     * @brief Generates the tube objects for the aggregate zone
     *
     * Creates nr * nz_w tubes arranged in cylindrical layers.
     * Each tube represents a computational cell in the drywell model.
     */
    void generateAggregateZone();

    /**
     * @brief Generates the tube objects for the zone below the well down to groundwater
     *
     * Creates nr * nz_g tubes arranged in cylindrical layers below the aggregate zone.
     * Tubes extend from -(chamberDepth + aggregateDepth) down to -depthToGroundwater.
     */
    void generateBelowWellZone();

    /**
     * @brief Generates the well cylinders
     *
     * Creates three cylinders representing the well shaft:
     * 1. Chamber cylinder: from z=0 to z=-chamberDepth
     * 2. Aggregate zone cylinder: from z=-chamberDepth to z=-(chamberDepth+aggregateDepth)
     * 3. Below-well cylinder: from z=-(chamberDepth+aggregateDepth) to z=-depthToGroundwater
     */
    void generateWellCylinders();

    /**
     * @brief Generates all components of the drywell system
     *
     * Convenience method that calls:
     * - generateAggregateZone()
     * - generateBelowWellZone()
     * - generateWellCylinders()
     *
     * Use this instead of calling each method separately.
     */
    void generateAll();

    /**
     * @brief Displays all tubes in the given AIS context
     * @param context The AIS interactive context
     */
    void displayInContext(const Handle(AIS_InteractiveContext)& context);

    /**
     * @brief Erases all tubes from the given AIS context
     * @param context The AIS interactive context
     */
    void eraseFromContext(const Handle(AIS_InteractiveContext)& context);

    /**
     * @brief Creates an OcctGeo3DObjectSet containing all tubes
     *
     * Creates a new OcctGeo3DObjectSet and adds all generated tubes to it.
     * Tubes are named as "tube_r{i}_z{j}" where i is radial index and j is vertical index.
     *
     * @return Pointer to newly created OcctGeo3DObjectSet with all tubes added
     * @note Caller is responsible for deleting the returned object set
     * @note Tubes must be generated first using generateAggregateZone()
     */
    OcctGeo3DObjectSet* createObjectSet() const;

    /**
     * @brief Adds all tubes to an existing OcctGeo3DObjectSet
     *
     * Adds all generated tubes to the provided object set.
     * Tubes are named as "tube_r{i}_z{j}" where i is radial index and j is vertical index.
     *
     * @param objectSet Existing OcctGeo3DObjectSet to add tubes to
     * @note Tubes must be generated first using generateAggregateZone()
     */
    void addToObjectSet(OcctGeo3DObjectSet* objectSet) const;

    /**
     * @brief Gets all tube objects in the aggregate zone
     * @return Vector of pointers to OcctTubeObject instances
     */
    const QVector<OcctTubeObject*>& getTubes() const;

    /**
     * @brief Gets a specific tube by radial and vertical indices
     * @param radialIndex Index in radial direction (0 to nr-1)
     * @param verticalIndex Index in vertical direction (0 to nz_w-1)
     * @return Pointer to the tube, or nullptr if indices are invalid
     */
    OcctTubeObject* getTube(int radialIndex, int verticalIndex) const;

    /**
     * @brief Gets all tube objects in the below-well zone
     * @return Vector of pointers to OcctTubeObject instances
     */
    const QVector<OcctTubeObject*>& getBelowWellTubes() const;

    /**
     * @brief Gets a specific below-well tube by radial and vertical indices
     * @param radialIndex Index in radial direction (0 to nr-1)
     * @param verticalIndex Index in vertical direction (0 to nz_g-1)
     * @return Pointer to the tube, or nullptr if indices are invalid
     */
    OcctTubeObject* getBelowWellTube(int radialIndex, int verticalIndex) const;

    /**
     * @brief Gets the number of tubes created
     * @return Total number of tubes (aggregate + below-well)
     */
    int getTubeCount() const;

    /**
     * @brief Clears all created tubes
     */
    void clear();

    /**
     * @brief Exports the system configuration to JSON
     * @return QJsonObject containing system parameters and all tubes
     */
    QJsonObject toJson() const;

    /**
     * @brief Imports the system configuration from JSON
     * @param json QJsonObject containing system data
     * @return true if successful, false otherwise
     */
    bool fromJson(const QJsonObject& json);

    // Getters for system parameters
    float getWellRadius() const { return m_wellRadius; }
    float getChamberDepth() const { return m_chamberDepth; }
    float getAggregateDepth() const { return m_aggregateDepth; }
    float getDomainRadius() const { return m_domainRadius; }
    float getDepthToGroundwater() const { return m_depthToGroundwater; }
    int getNr() const { return m_nr; }
    int getNzW() const { return m_nz_w; }
    int getNzG() const { return m_nz_g; }

    // Calculated parameters
    float getRadialCellSize() const;
    float getVerticalCellSize() const;
    float getBelowWellVerticalCellSize() const;

private:
    // System parameters
    float m_wellRadius;           // R_w
    float m_chamberDepth;         // D_c
    float m_aggregateDepth;       // D_a
    float m_domainRadius;         // R_d
    float m_depthToGroundwater;   // D_gw
    int m_nr;                     // Number of radial cells
    int m_nz_w;                   // Number of vertical cells in aggregate zone
    int m_nz_g;                   // Number of vertical cells below aggregate

    // Generated tubes
    QVector<OcctTubeObject*> m_tubes;
    QVector<OcctTubeObject*> m_belowWellTubes;

    // Well cylinders
    OcctCylinderObject* m_chamberCylinder;        // 0 to -chamberDepth
    OcctCylinderObject* m_aggregateWellCylinder;  // -chamberDepth to -(chamberDepth+aggregateDepth)
    OcctCylinderObject* m_belowWellCylinder;      // -(chamberDepth+aggregateDepth) to -depthToGroundwater

    // Helper methods
    void createTube(int radialIndex, int verticalIndex);
    void createBelowWellTube(int radialIndex, int verticalIndex);
    int getTubeIndex(int radialIndex, int verticalIndex) const;
    int getBelowWellTubeIndex(int radialIndex, int verticalIndex) const;
};

#endif // OCCTDRYWELLSYSTEM_H

/**
 * @file occtcylinderobject.h
 * @brief Header file for the OcctCylinderObject class
 */

#ifndef OCCTCYLINDEROBJECT_H
#define OCCTCYLINDEROBJECT_H

#include "occtgeo3dobject.h"

/**
 * @class OcctCylinderObject
 * @brief A 3D cylinder object implementation using OpenCASCADE
 *
 * The OcctCylinderObject class provides a concrete implementation of a 3D cylinder
 * geometry using OpenCASCADE. It inherits all transform and material properties
 * from OcctGeo3DObject and implements the cylinder-specific shape creation.
 *
 * The cylinder is oriented along the Z-axis by default, with its center at the origin.
 * The cylinder's dimensions can be customized through radius and length (height).
 */
class OcctCylinderObject : public OcctGeo3DObject
{
public:
    /**
     * @brief Default constructor
     * Creates a cylinder with default parameters (radius=1.0, length=2.0)
     */
    explicit OcctCylinderObject();

    /**
     * @brief Parameterized constructor
     * @param radius The radius of the cylinder (must be positive)
     * @param length The length (height) of the cylinder (must be positive)
     */
    explicit OcctCylinderObject(float radius, float length);

    /**
     * @brief Virtual destructor
     */
    virtual ~OcctCylinderObject();

    // Cylinder-specific properties
    float getRadius() const;
    void setRadius(float radius);

    float getLength() const;
    void setLength(float length);

    void setDimensions(float radius, float length);

    // JSON Serialization
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject& json) override;
    QString getObjectType() const override;

protected:
    /**
     * @brief Creates the cylinder shape using OpenCASCADE
     * @return TopoDS_Shape containing the cylinder geometry
     */
    TopoDS_Shape createShape() override;

private:
    float m_radius;
    float m_length;
};

#endif // OCCTCYLINDEROBJECT_H

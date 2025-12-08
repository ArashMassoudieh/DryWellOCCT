/**
 * @file occttubeobject.h
 * @brief Header file for the OcctTubeObject class
 */

#ifndef OCCTTUBEOBJECT_H
#define OCCTTUBEOBJECT_H

#include "occtgeo3dobject.h"

/**
 * @class OcctTubeObject
 * @brief A 3D hollow cylinder (tube) object using OpenCASCADE
 *
 * The OcctTubeObject class creates a tube/pipe shape - essentially a cylinder
 * with another cylinder removed from its center. It's defined by inner radius,
 * outer radius, and height. Created using Boolean operations in OpenCASCADE.
 */
class OcctTubeObject : public OcctGeo3DObject
{
public:
    /**
     * @brief Default constructor
     * Creates a tube with default parameters
     */
    explicit OcctTubeObject();

    /**
     * @brief Parameterized constructor
     * @param innerRadius Inner radius of the tube
     * @param outerRadius Outer radius of the tube
     * @param height Height of the tube
     */
    explicit OcctTubeObject(float innerRadius, float outerRadius, float height);

    /**
     * @brief Virtual destructor
     */
    virtual ~OcctTubeObject();

    // Tube-specific properties
    float getInnerRadius() const;
    void setInnerRadius(float radius);

    float getOuterRadius() const;
    void setOuterRadius(float radius);

    float getHeight() const;
    void setHeight(float height);

    void setDimensions(float innerRadius, float outerRadius, float height);

    // JSON Serialization
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject& json) override;
    QString getObjectType() const override;

protected:
    /**
     * @brief Creates the tube shape using OpenCASCADE
     * Uses BRepPrimAPI_MakeCylinder and BRepAlgoAPI_Cut to create hollow cylinder
     * @return TopoDS_Shape containing the tube geometry
     */
    TopoDS_Shape createShape() override;

private:
    float m_innerRadius;
    float m_outerRadius;
    float m_height;
};

#endif // OCCTTUBEOBJECT_H

#ifndef OCCTGEO3DOBJECT_H
#define OCCTGEO3DOBJECT_H

#include <QVector3D>
#include <QColor>
#include <QJsonObject>
#include <QMap>
#include <functional>

// OpenCASCADE includes
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax1.hxx>
#include <gp_Vec.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>

/**
 * @class OcctGeo3DObject
 * @brief Base class for 3D geometric objects using OpenCASCADE
 *
 * This class provides a base for creating 3D objects with OpenCASCADE.
 * It handles transformations (position, rotation, scale), materials,
 * colors, transparency, and edge display.
 */
class OcctGeo3DObject
{
public:
    explicit OcctGeo3DObject();
    virtual ~OcctGeo3DObject();

    // Transform properties
    QVector3D getPosition() const;
    void setPosition(const QVector3D& position);
    void setPosition(float x, float y, float z);

    QVector3D getRotation() const;
    void setRotation(const QVector3D& rotation);
    void setRotation(float x, float y, float z);

    QVector3D getScale() const;
    void setScale(const QVector3D& scale);
    void setScale(float uniformScale);
    void setScale(float x, float y, float z);

    // Material properties
    QColor getDiffuseColor() const;
    void setDiffuseColor(const QColor& color);

    QColor getAmbientColor() const;
    void setAmbientColor(const QColor& color);

    QColor getSpecularColor() const;
    void setSpecularColor(const QColor& color);

    float getShininess() const;
    void setShininess(float shininess);

    float getOpacity() const;
    void setOpacity(float opacity);

    // Visibility
    bool isVisible() const;
    void setVisible(bool visible);

    // Edge display
    bool isShowEdges() const;
    void setShowEdges(bool show);

    QColor getEdgeColor() const;
    void setEdgeColor(const QColor& color);

    float getEdgeWidth() const;
    void setEdgeWidth(float width);

    // OpenCASCADE AIS object creation and management
    virtual Handle(AIS_Shape) createAISObject();
    Handle(AIS_Shape) getAISShape() const;

    void displayInContext(const Handle(AIS_InteractiveContext)& context);
    void eraseFromContext(const Handle(AIS_InteractiveContext)& context);
    void redisplay(const Handle(AIS_InteractiveContext)& context);

    // JSON Serialization
    virtual QJsonObject toJson() const = 0;
    virtual bool fromJson(const QJsonObject& json) = 0;
    virtual QString getObjectType() const = 0;

    /**
     * @brief Creates an OcctGeo3DObject from JSON data
     *
     * Factory method that creates the appropriate object type based on the
     * "type" field in the JSON data and deserializes it.
     *
     * @param json QJsonObject containing the serialized object data
     * @return Pointer to the created object, or nullptr if creation failed
     */
    static OcctGeo3DObject* createFromJson(const QJsonObject& json);

    // Factory function type
    typedef std::function<OcctGeo3DObject*()> ObjectFactory;

    /**
     * @brief Registers a factory function for an object type
     *
     * @param typeName Type name identifier (e.g., "Cylinder")
     * @param factory Factory function that creates the object
     */
    static void registerObjectType(const QString& typeName, ObjectFactory factory);

protected:
    // Pure virtual method for creating shape - must be implemented by derived classes
    virtual TopoDS_Shape createShape() = 0;

    // Update methods - called when properties change
    virtual void updateTransform();
    virtual void updateMaterial();
    virtual void updateEdgeDisplay();
    virtual void updateVisibility(const Handle(AIS_InteractiveContext)& context);

    // Helper method to apply all updates
    void updateAISObject(const Handle(AIS_InteractiveContext)& context = nullptr);

    // Access to the shape for derived classes
    TopoDS_Shape getShape() const;

private:
    // Transform data
    QVector3D m_position;
    QVector3D m_rotation;  // Rotation in degrees around X, Y, Z axes
    QVector3D m_scale;

    // Material data
    QColor m_diffuseColor;
    QColor m_ambientColor;
    QColor m_specularColor;
    float m_shininess;
    float m_opacity;

    // Visibility
    bool m_visible;

    // Edge display
    bool m_showEdges;
    QColor m_edgeColor;
    float m_edgeWidth;

    // OpenCASCADE objects
    Handle(AIS_Shape) m_aisShape;
    TopoDS_Shape m_shape;

    // Helper methods
    gp_Trsf computeTransformation() const;
    void applyTransformationToShape();
};

#endif // OCCTGEO3DOBJECT_H

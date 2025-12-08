#include "occtgeo3dobject.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

OcctGeo3DObject::OcctGeo3DObject()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_diffuseColor(QColor(102, 84, 35))   // Brown color
    , m_ambientColor(QColor(68, 51, 17))    // Darker brown
    , m_specularColor(QColor(255, 255, 255)) // White
    , m_shininess(50.0f)
    , m_opacity(1.0f)
    , m_visible(true)
    , m_showEdges(false)
    , m_edgeColor(Qt::black)
    , m_edgeWidth(1.0f)
    , m_aisShape(nullptr)
{
}

OcctGeo3DObject::~OcctGeo3DObject()
{
    // AIS objects are managed by handles, automatic cleanup
}

// ============================================
// Transform Properties
// ============================================

QVector3D OcctGeo3DObject::getPosition() const
{
    return m_position;
}

void OcctGeo3DObject::setPosition(const QVector3D& position)
{
    m_position = position;
    updateTransform();
}

void OcctGeo3DObject::setPosition(float x, float y, float z)
{
    setPosition(QVector3D(x, y, z));
}

QVector3D OcctGeo3DObject::getRotation() const
{
    return m_rotation;
}

void OcctGeo3DObject::setRotation(const QVector3D& rotation)
{
    m_rotation = rotation;
    updateTransform();
}

void OcctGeo3DObject::setRotation(float x, float y, float z)
{
    setRotation(QVector3D(x, y, z));
}

QVector3D OcctGeo3DObject::getScale() const
{
    return m_scale;
}

void OcctGeo3DObject::setScale(const QVector3D& scale)
{
    m_scale = scale;
    // Note: OCCT doesn't support non-uniform scaling on shapes easily
    // We need to recreate the shape with scaling applied
    if (!m_aisShape.IsNull()) {
        // Recreate the shape with new scale
        m_shape = createShape();
        applyTransformationToShape();
        m_aisShape->Set(m_shape);
    }
}

void OcctGeo3DObject::setScale(float uniformScale)
{
    setScale(QVector3D(uniformScale, uniformScale, uniformScale));
}

void OcctGeo3DObject::setScale(float x, float y, float z)
{
    setScale(QVector3D(x, y, z));
}

// ============================================
// Material Properties
// ============================================

QColor OcctGeo3DObject::getDiffuseColor() const
{
    return m_diffuseColor;
}

void OcctGeo3DObject::setDiffuseColor(const QColor& color)
{
    m_diffuseColor = color;
    updateMaterial();
}

QColor OcctGeo3DObject::getAmbientColor() const
{
    return m_ambientColor;
}

void OcctGeo3DObject::setAmbientColor(const QColor& color)
{
    m_ambientColor = color;
    updateMaterial();
}

QColor OcctGeo3DObject::getSpecularColor() const
{
    return m_specularColor;
}

void OcctGeo3DObject::setSpecularColor(const QColor& color)
{
    m_specularColor = color;
    updateMaterial();
}

float OcctGeo3DObject::getShininess() const
{
    return m_shininess;
}

void OcctGeo3DObject::setShininess(float shininess)
{
    m_shininess = shininess;
    updateMaterial();
}

float OcctGeo3DObject::getOpacity() const
{
    return m_opacity;
}

void OcctGeo3DObject::setOpacity(float opacity)
{
    m_opacity = qBound(0.0f, opacity, 1.0f);
    updateMaterial();
}

// ============================================
// Visibility
// ============================================

bool OcctGeo3DObject::isVisible() const
{
    return m_visible;
}

void OcctGeo3DObject::setVisible(bool visible)
{
    m_visible = visible;
    // Note: Actual visibility change requires context
    // Call updateVisibility() if context is available
}

// ============================================
// Edge Display
// ============================================

bool OcctGeo3DObject::isShowEdges() const
{
    return m_showEdges;
}

void OcctGeo3DObject::setShowEdges(bool show)
{
    m_showEdges = show;
    updateEdgeDisplay();
}

QColor OcctGeo3DObject::getEdgeColor() const
{
    return m_edgeColor;
}

void OcctGeo3DObject::setEdgeColor(const QColor& color)
{
    m_edgeColor = color;
    updateEdgeDisplay();
}

float OcctGeo3DObject::getEdgeWidth() const
{
    return m_edgeWidth;
}

void OcctGeo3DObject::setEdgeWidth(float width)
{
    m_edgeWidth = width;
    updateEdgeDisplay();
}

// ============================================
// OpenCASCADE AIS Object Management
// ============================================

Handle(AIS_Shape) OcctGeo3DObject::createAISObject()
{
    if (m_aisShape.IsNull()) {
        // Create the shape
        m_shape = createShape();

        if (!m_shape.IsNull()) {
            // Apply transformations to the shape
            applyTransformationToShape();

            // Create AIS shape
            m_aisShape = new AIS_Shape(m_shape);

            // Apply material properties
            updateMaterial();

            // Apply edge display
            updateEdgeDisplay();
        }
    }

    return m_aisShape;
}

Handle(AIS_Shape) OcctGeo3DObject::getAISShape() const
{
    return m_aisShape;
}

void OcctGeo3DObject::displayInContext(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    // Create AIS object if not already created
    if (m_aisShape.IsNull()) {
        createAISObject();
    }

    if (!m_aisShape.IsNull()) {
        if (m_visible) {
            context->Display(m_aisShape, Standard_False);
        }
    }
}

void OcctGeo3DObject::eraseFromContext(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull() || m_aisShape.IsNull()) {
        return;
    }

    context->Erase(m_aisShape, Standard_False);
}

void OcctGeo3DObject::redisplay(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull() || m_aisShape.IsNull()) {
        return;
    }

    context->Redisplay(m_aisShape, Standard_False);
}

// ============================================
// Update Methods
// ============================================

void OcctGeo3DObject::updateTransform()
{
    if (!m_aisShape.IsNull()) {
        // For OCCT, we apply transformation to the shape itself
        // Recreate shape with new transformation
        m_shape = createShape();
        applyTransformationToShape();
        m_aisShape->Set(m_shape);
    }
}

void OcctGeo3DObject::updateMaterial()
{
    if (!m_aisShape.IsNull()) {
        // Set diffuse color
        Quantity_Color occtColor(
            m_diffuseColor.redF(),
            m_diffuseColor.greenF(),
            m_diffuseColor.blueF(),
            Quantity_TOC_RGB
            );
        m_aisShape->SetColor(occtColor);

        // Set transparency (OCCT uses transparency, which is 1 - opacity)
        Standard_Real transparency = 1.0 - m_opacity;
        if (transparency > 0.001) {
            m_aisShape->SetTransparency(transparency);
        } else {
            m_aisShape->UnsetTransparency();
        }

        // Set material
        m_aisShape->SetMaterial(Graphic3d_NOM_PLASTIC);

        // Note: OCCT doesn't have separate ambient/specular controls like Qt3D
        // These are controlled by the material type
        // You can create custom materials if needed
    }
}

void OcctGeo3DObject::updateEdgeDisplay()
{
    if (!m_aisShape.IsNull()) {
        Handle(Prs3d_Drawer) drawer = m_aisShape->Attributes();

        if (m_showEdges) {
            // Enable face boundary (edge) display
            drawer->SetFaceBoundaryDraw(Standard_True);

            // Create edge color
            Quantity_Color edgeColor(
                m_edgeColor.redF(),
                m_edgeColor.greenF(),
                m_edgeColor.blueF(),
                Quantity_TOC_RGB
                );

            // Create line aspect for edges
            Handle(Prs3d_LineAspect) lineAspect = new Prs3d_LineAspect(
                edgeColor,
                Aspect_TOL_SOLID,
                m_edgeWidth
                );

            drawer->SetFaceBoundaryAspect(lineAspect);
        } else {
            // Disable face boundary display
            drawer->SetFaceBoundaryDraw(Standard_False);
        }
    }
}

void OcctGeo3DObject::updateVisibility(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull() || m_aisShape.IsNull()) {
        return;
    }

    if (m_visible) {
        context->Display(m_aisShape, Standard_False);
    } else {
        context->Erase(m_aisShape, Standard_False);
    }
}

void OcctGeo3DObject::updateAISObject(const Handle(AIS_InteractiveContext)& context)
{
    if (!m_aisShape.IsNull()) {
        updateTransform();
        updateMaterial();
        updateEdgeDisplay();

        if (!context.IsNull()) {
            context->Redisplay(m_aisShape, Standard_False);
        }
    }
}

// ============================================
// Helper Methods
// ============================================

gp_Trsf OcctGeo3DObject::computeTransformation() const
{
    gp_Trsf transform;

    // Apply translation
    gp_Vec translation(m_position.x(), m_position.y(), m_position.z());
    transform.SetTranslation(translation);

    // Apply rotations (in degrees, convert to radians)
    // Order: Z, Y, X (can be changed based on your needs)
    if (m_rotation.z() != 0.0f) {
        gp_Trsf rotZ;
        rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)),
                         m_rotation.z() * M_PI / 180.0);
        transform = transform * rotZ;
    }

    if (m_rotation.y() != 0.0f) {
        gp_Trsf rotY;
        rotY.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)),
                         m_rotation.y() * M_PI / 180.0);
        transform = transform * rotY;
    }

    if (m_rotation.x() != 0.0f) {
        gp_Trsf rotX;
        rotX.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)),
                         m_rotation.x() * M_PI / 180.0);
        transform = transform * rotX;
    }

    // Apply uniform scaling if not (1,1,1)
    // Note: Non-uniform scaling requires recreating geometry
    if (m_scale.x() == m_scale.y() && m_scale.y() == m_scale.z() && m_scale.x() != 1.0f) {
        gp_Trsf scale;
        scale.SetScale(gp_Pnt(0, 0, 0), m_scale.x());
        transform = transform * scale;
    }

    return transform;
}

void OcctGeo3DObject::applyTransformationToShape()
{
    if (!m_shape.IsNull()) {
        gp_Trsf transform = computeTransformation();

        // Only apply if transformation is not identity
        bool isIdentity = (transform.Form() == gp_Identity);
        if (!isIdentity) {
            BRepBuilderAPI_Transform transformer(m_shape, transform, Standard_True);
            m_shape = transformer.Shape();
        }
    }
}

TopoDS_Shape OcctGeo3DObject::getShape() const
{
    return m_shape;
}

// ============================================
// Factory Registration (Static)
// ============================================

static QMap<QString, OcctGeo3DObject::ObjectFactory> s_occtObjectFactories;

void OcctGeo3DObject::registerObjectType(const QString& typeName, ObjectFactory factory)
{
    s_occtObjectFactories[typeName] = factory;
}

OcctGeo3DObject* OcctGeo3DObject::createFromJson(const QJsonObject& json)
{
    if (!json.contains("type")) {
        return nullptr;
    }

    QString objectType = json["type"].toString();

    auto it = s_occtObjectFactories.find(objectType);
    if (it == s_occtObjectFactories.end()) {
        return nullptr; // Unknown object type
    }

    OcctGeo3DObject* object = it.value()(); // Call factory function
    if (object && object->fromJson(json)) {
        return object;
    } else {
        delete object;
        return nullptr;
    }
}

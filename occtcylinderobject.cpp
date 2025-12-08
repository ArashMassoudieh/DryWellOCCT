#include "occtcylinderobject.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>
#include <QJsonObject>

OcctCylinderObject::OcctCylinderObject()
    : OcctGeo3DObject()
    , m_radius(1.0f)
    , m_length(2.0f)
{
}

OcctCylinderObject::OcctCylinderObject(float radius, float length)
    : OcctGeo3DObject()
    , m_radius(radius)
    , m_length(length)
{
}

OcctCylinderObject::~OcctCylinderObject()
{
}

float OcctCylinderObject::getRadius() const
{
    return m_radius;
}

void OcctCylinderObject::setRadius(float radius)
{
    if (m_radius != radius) {
        m_radius = radius;
        // Recreate the shape if AIS object exists
        if (!getAISShape().IsNull()) {
            createAISObject();
        }
    }
}

float OcctCylinderObject::getLength() const
{
    return m_length;
}

void OcctCylinderObject::setLength(float length)
{
    if (m_length != length) {
        m_length = length;
        // Recreate the shape if AIS object exists
        if (!getAISShape().IsNull()) {
            createAISObject();
        }
    }
}

void OcctCylinderObject::setDimensions(float radius, float length)
{
    bool changed = false;
    if (m_radius != radius) {
        m_radius = radius;
        changed = true;
    }
    if (m_length != length) {
        m_length = length;
        changed = true;
    }

    if (changed && !getAISShape().IsNull()) {
        createAISObject();
    }
}

TopoDS_Shape OcctCylinderObject::createShape()
{
    // Create cylinder along Z-axis
    // Center is at origin, extends from -length/2 to +length/2
    gp_Ax2 axis(gp_Pnt(0, 0, -m_length/2.0), gp_Dir(0, 0, 1));

    TopoDS_Shape shape = BRepPrimAPI_MakeCylinder(axis, m_radius, m_length).Shape();

    return shape;
}

QJsonObject OcctCylinderObject::toJson() const
{
    QJsonObject json;
    json["type"] = getObjectType();

    // Transform
    QJsonObject transform;
    QVector3D pos = getPosition();
    QVector3D rot = getRotation();
    QVector3D scale = getScale();

    transform["position"] = QJsonObject{{"x", pos.x()}, {"y", pos.y()}, {"z", pos.z()}};
    transform["rotation"] = QJsonObject{{"x", rot.x()}, {"y", rot.y()}, {"z", rot.z()}};
    transform["scale"] = QJsonObject{{"x", scale.x()}, {"y", scale.y()}, {"z", scale.z()}};
    json["transform"] = transform;

    // Material
    QJsonObject material;
    QColor diffuse = getDiffuseColor();
    QColor ambient = getAmbientColor();
    QColor specular = getSpecularColor();

    material["diffuse"] = QJsonObject{{"r", diffuse.red()}, {"g", diffuse.green()}, {"b", diffuse.blue()}, {"a", diffuse.alpha()}};
    material["ambient"] = QJsonObject{{"r", ambient.red()}, {"g", ambient.green()}, {"b", ambient.blue()}, {"a", ambient.alpha()}};
    material["specular"] = QJsonObject{{"r", specular.red()}, {"g", specular.green()}, {"b", specular.blue()}, {"a", specular.alpha()}};
    material["shininess"] = getShininess();
    json["material"] = material;

    json["visible"] = isVisible();
    json["opacity"] = getOpacity();

    // Edge properties
    json["showEdges"] = isShowEdges();
    QColor edgeColor = getEdgeColor();
    json["edgeColor"] = QJsonObject{{"r", edgeColor.red()}, {"g", edgeColor.green()}, {"b", edgeColor.blue()}};
    json["edgeWidth"] = getEdgeWidth();

    // Cylinder properties
    QJsonObject cylinder;
    cylinder["radius"] = m_radius;
    cylinder["length"] = m_length;
    json["cylinder"] = cylinder;

    return json;
}

bool OcctCylinderObject::fromJson(const QJsonObject& json)
{
    if (json["type"].toString() != getObjectType()) {
        return false;
    }

    // Load transform
    if (json.contains("transform")) {
        QJsonObject transform = json["transform"].toObject();
        if (transform.contains("position")) {
            QJsonObject pos = transform["position"].toObject();
            setPosition(pos["x"].toDouble(), pos["y"].toDouble(), pos["z"].toDouble());
        }
        if (transform.contains("rotation")) {
            QJsonObject rot = transform["rotation"].toObject();
            setRotation(rot["x"].toDouble(), rot["y"].toDouble(), rot["z"].toDouble());
        }
        if (transform.contains("scale")) {
            QJsonObject scale = transform["scale"].toObject();
            setScale(scale["x"].toDouble(), scale["y"].toDouble(), scale["z"].toDouble());
        }
    }

    // Load material
    if (json.contains("material")) {
        QJsonObject material = json["material"].toObject();
        if (material.contains("diffuse")) {
            QJsonObject diffuse = material["diffuse"].toObject();
            setDiffuseColor(QColor(diffuse["r"].toInt(), diffuse["g"].toInt(), diffuse["b"].toInt(), diffuse["a"].toInt()));
        }
        if (material.contains("ambient")) {
            QJsonObject ambient = material["ambient"].toObject();
            setAmbientColor(QColor(ambient["r"].toInt(), ambient["g"].toInt(), ambient["b"].toInt(), ambient["a"].toInt()));
        }
        if (material.contains("specular")) {
            QJsonObject specular = material["specular"].toObject();
            setSpecularColor(QColor(specular["r"].toInt(), specular["g"].toInt(), specular["b"].toInt(), specular["a"].toInt()));
        }
        if (material.contains("shininess")) {
            setShininess(material["shininess"].toDouble());
        }
    }

    if (json.contains("visible")) {
        setVisible(json["visible"].toBool());
    }

    if (json.contains("opacity")) {
        setOpacity(json["opacity"].toDouble());
    }

    // Load edge properties
    if (json.contains("showEdges")) {
        setShowEdges(json["showEdges"].toBool());
    }
    if (json.contains("edgeColor")) {
        QJsonObject edgeColor = json["edgeColor"].toObject();
        setEdgeColor(QColor(edgeColor["r"].toInt(), edgeColor["g"].toInt(), edgeColor["b"].toInt()));
    }
    if (json.contains("edgeWidth")) {
        setEdgeWidth(json["edgeWidth"].toDouble());
    }

    // Load cylinder properties
    if (json.contains("cylinder")) {
        QJsonObject cylinder = json["cylinder"].toObject();
        if (cylinder.contains("radius") && cylinder.contains("length")) {
            setDimensions(cylinder["radius"].toDouble(), cylinder["length"].toDouble());
        }
    }

    return true;
}

QString OcctCylinderObject::getObjectType() const
{
    return "Cylinder";
}

// Static registration
static bool s_occtCylinderRegistered = []() {
    OcctGeo3DObject::registerObjectType("Cylinder", []() -> OcctGeo3DObject* {
        return new OcctCylinderObject();
    });
    return true;
}();

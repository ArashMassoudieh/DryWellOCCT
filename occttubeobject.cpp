#include "occttubeobject.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <gp_Ax2.hxx>
#include <QJsonObject>

OcctTubeObject::OcctTubeObject()
    : OcctGeo3DObject()
    , m_innerRadius(0.5f)
    , m_outerRadius(1.0f)
    , m_height(2.0f)
{
}

OcctTubeObject::OcctTubeObject(float innerRadius, float outerRadius, float height)
    : OcctGeo3DObject()
    , m_innerRadius(innerRadius)
    , m_outerRadius(outerRadius)
    , m_height(height)
{
}

OcctTubeObject::~OcctTubeObject()
{
}

float OcctTubeObject::getInnerRadius() const
{
    return m_innerRadius;
}

void OcctTubeObject::setInnerRadius(float radius)
{
    if (m_innerRadius != radius) {
        m_innerRadius = radius;
        if (!getAISShape().IsNull()) {
            createAISObject();
        }
    }
}

float OcctTubeObject::getOuterRadius() const
{
    return m_outerRadius;
}

void OcctTubeObject::setOuterRadius(float radius)
{
    if (m_outerRadius != radius) {
        m_outerRadius = radius;
        if (!getAISShape().IsNull()) {
            createAISObject();
        }
    }
}

float OcctTubeObject::getHeight() const
{
    return m_height;
}

void OcctTubeObject::setHeight(float height)
{
    if (m_height != height) {
        m_height = height;
        if (!getAISShape().IsNull()) {
            createAISObject();
        }
    }
}

void OcctTubeObject::setDimensions(float innerRadius, float outerRadius, float height)
{
    bool changed = false;
    if (m_innerRadius != innerRadius) {
        m_innerRadius = innerRadius;
        changed = true;
    }
    if (m_outerRadius != outerRadius) {
        m_outerRadius = outerRadius;
        changed = true;
    }
    if (m_height != height) {
        m_height = height;
        changed = true;
    }

    if (changed && !getAISShape().IsNull()) {
        createAISObject();
    }
}

TopoDS_Shape OcctTubeObject::createShape()
{
    // Create axis along Z direction, centered at origin
    gp_Ax2 axis(gp_Pnt(0, 0, -m_height/2.0), gp_Dir(0, 0, 1));

    // Create outer cylinder
    TopoDS_Shape outerCylinder = BRepPrimAPI_MakeCylinder(axis, m_outerRadius, m_height).Shape();

    // Create inner cylinder (to be subtracted)
    TopoDS_Shape innerCylinder = BRepPrimAPI_MakeCylinder(axis, m_innerRadius, m_height).Shape();

    // Perform Boolean cut operation to create tube
    TopoDS_Shape tube = BRepAlgoAPI_Cut(outerCylinder, innerCylinder).Shape();

    return tube;
}

QJsonObject OcctTubeObject::toJson() const
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

    // Tube properties
    QJsonObject tube;
    tube["innerRadius"] = m_innerRadius;
    tube["outerRadius"] = m_outerRadius;
    tube["height"] = m_height;
    json["tube"] = tube;

    return json;
}

bool OcctTubeObject::fromJson(const QJsonObject& json)
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

    // Load tube properties
    if (json.contains("tube")) {
        QJsonObject tube = json["tube"].toObject();
        if (tube.contains("innerRadius") && tube.contains("outerRadius") && tube.contains("height")) {
            setDimensions(tube["innerRadius"].toDouble(),
                          tube["outerRadius"].toDouble(),
                          tube["height"].toDouble());
        }
    }

    return true;
}

QString OcctTubeObject::getObjectType() const
{
    return "Tube";
}

// Static registration
static bool s_occtTubeRegistered = []() {
    OcctGeo3DObject::registerObjectType("Tube", []() -> OcctGeo3DObject* {
        return new OcctTubeObject();
    });
    return true;
}();

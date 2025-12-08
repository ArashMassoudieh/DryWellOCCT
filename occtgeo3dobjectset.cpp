#include "occtgeo3dobjectset.h"
#include "occtgeo3dobject.h"

#include <QJsonDocument>
#include <QFile>
#include <QIODevice>

OcctGeo3DObjectSet::OcctGeo3DObjectSet()
    : m_ownsObjects(true)
{
}

OcctGeo3DObjectSet::~OcctGeo3DObjectSet()
{
    clear();
}

void OcctGeo3DObjectSet::addObject(const QString& name, OcctGeo3DObject* object)
{
    if (!object) {
        return;
    }

    // If an object with this name already exists, remove it first
    if (m_objects.contains(name)) {
        removeObject(name);
    }

    m_objects.insert(name, object);
}

bool OcctGeo3DObjectSet::removeObject(const QString& name)
{
    auto it = m_objects.find(name);
    if (it != m_objects.end()) {
        if (m_ownsObjects && it.value()) {
            delete it.value();
        }
        m_objects.erase(it);
        return true;
    }
    return false;
}

void OcctGeo3DObjectSet::clear()
{
    if (m_ownsObjects) {
        for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
            delete it.value();
        }
    }
    m_objects.clear();
}

OcctGeo3DObject* OcctGeo3DObjectSet::getObject(const QString& name) const
{
    auto it = m_objects.find(name);
    return (it != m_objects.end()) ? it.value() : nullptr;
}

bool OcctGeo3DObjectSet::contains(const QString& name) const
{
    return m_objects.contains(name);
}

QStringList OcctGeo3DObjectSet::getObjectNames() const
{
    return m_objects.keys();
}

int OcctGeo3DObjectSet::count() const
{
    return m_objects.size();
}

bool OcctGeo3DObjectSet::isEmpty() const
{
    return m_objects.isEmpty();
}

QMap<QString, OcctGeo3DObject*>::iterator OcctGeo3DObjectSet::begin()
{
    return m_objects.begin();
}

QMap<QString, OcctGeo3DObject*>::iterator OcctGeo3DObjectSet::end()
{
    return m_objects.end();
}

QMap<QString, OcctGeo3DObject*>::const_iterator OcctGeo3DObjectSet::begin() const
{
    return m_objects.begin();
}

QMap<QString, OcctGeo3DObject*>::const_iterator OcctGeo3DObjectSet::end() const
{
    return m_objects.end();
}

QMap<QString, OcctGeo3DObject*>::const_iterator OcctGeo3DObjectSet::constBegin() const
{
    return m_objects.constBegin();
}

QMap<QString, OcctGeo3DObject*>::const_iterator OcctGeo3DObjectSet::constEnd() const
{
    return m_objects.constEnd();
}

void OcctGeo3DObjectSet::displayAll(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    for (auto it = m_objects.constBegin(); it != m_objects.constEnd(); ++it) {
        if (it.value()) {
            it.value()->displayInContext(context);
        }
    }

    context->UpdateCurrentViewer();
}

void OcctGeo3DObjectSet::eraseAll(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    for (auto it = m_objects.constBegin(); it != m_objects.constEnd(); ++it) {
        if (it.value()) {
            it.value()->eraseFromContext(context);
        }
    }

    context->UpdateCurrentViewer();
}

void OcctGeo3DObjectSet::redisplayAll(const Handle(AIS_InteractiveContext)& context)
{
    if (context.IsNull()) {
        return;
    }

    for (auto it = m_objects.constBegin(); it != m_objects.constEnd(); ++it) {
        if (it.value()) {
            it.value()->redisplay(context);
        }
    }

    context->UpdateCurrentViewer();
}

void OcctGeo3DObjectSet::updateViewer(const Handle(AIS_InteractiveContext)& context)
{
    if (!context.IsNull()) {
        context->UpdateCurrentViewer();
    }
}

void OcctGeo3DObjectSet::setAllVisible(bool visible)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setVisible(visible);
        }
    }
}

void OcctGeo3DObjectSet::setObjectVisible(const QString& name, bool visible)
{
    OcctGeo3DObject* obj = getObject(name);
    if (obj) {
        obj->setVisible(visible);
    }
}

void OcctGeo3DObjectSet::setAllDiffuseColor(const QColor& color)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setDiffuseColor(color);
        }
    }
}

void OcctGeo3DObjectSet::setAllScale(float uniformScale)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setScale(uniformScale);
        }
    }
}

void OcctGeo3DObjectSet::setAllScale(const QVector3D& scale)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setScale(scale);
        }
    }
}

void OcctGeo3DObjectSet::setAllShowEdges(bool show)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setShowEdges(show);
        }
    }
}

void OcctGeo3DObjectSet::setAllEdgeColor(const QColor& color)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setEdgeColor(color);
        }
    }
}

void OcctGeo3DObjectSet::setAllEdgeWidth(float width)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setEdgeWidth(width);
        }
    }
}

void OcctGeo3DObjectSet::setAllOpacity(float opacity)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it.value()) {
            it.value()->setOpacity(opacity);
        }
    }
}

const QMap<QString, OcctGeo3DObject*>& OcctGeo3DObjectSet::getObjectMap() const
{
    return m_objects;
}

QJsonObject OcctGeo3DObjectSet::toJson() const
{
    QJsonObject json;
    json["version"] = "1.0";
    json["objectCount"] = m_objects.size();

    QJsonObject objectsJson;
    for (auto it = m_objects.constBegin(); it != m_objects.constEnd(); ++it) {
        if (it.value()) {
            objectsJson[it.key()] = it.value()->toJson();
        }
    }

    json["objects"] = objectsJson;
    return json;
}

bool OcctGeo3DObjectSet::fromJson(const QJsonObject& json)
{
    // Clear existing objects
    clear();

    // Check version
    if (!json.contains("version")) {
        return false;
    }

    if (!json.contains("objects") || !json["objects"].isObject()) {
        return false;
    }

    QJsonObject objectsJson = json["objects"].toObject();
    for (auto it = objectsJson.begin(); it != objectsJson.end(); ++it) {
        QString name = it.key();

        if (!it.value().isObject()) {
            continue;
        }

        QJsonObject data = it.value().toObject();

        // Use factory method to create object
        OcctGeo3DObject* object = OcctGeo3DObject::createFromJson(data);
        if (object) {
            addObject(name, object);
        }
    }

    return true;
}

bool OcctGeo3DObjectSet::saveToFile(const QString& filePath) const
{
    QJsonObject json = toJson();
    QJsonDocument doc(json);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << filePath;
        return false;
    }

    qint64 bytesWritten = file.write(doc.toJson());
    file.close();

    if (bytesWritten == -1) {
        qWarning() << "Error writing to file:" << filePath;
        return false;
    }

    return true;
}

bool OcctGeo3DObjectSet::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file for reading:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    return fromJson(doc.object());
}

/**
 * @file occtgeo3dobjectset.h
 * @brief Header file for the OcctGeo3DObjectSet class
 */

#ifndef OCCTGEO3DOBJECTSET_H
#define OCCTGEO3DOBJECTSET_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QVector3D>
#include <QJsonObject>

#include <AIS_InteractiveContext.hxx>
#include <TopoDS_Compound.hxx>
#include <QString>

class OcctGeo3DObject;

/**
 * @class OcctGeo3DObjectSet
 * @brief A collection class for managing multiple OcctGeo3DObject instances
 *
 * The OcctGeo3DObjectSet class provides a convenient way to manage a collection of
 * OpenCASCADE-based 3D objects using a QMap with string keys. It offers functionality
 * for adding, removing, and manipulating multiple 3D objects as a group, with
 * OpenCASCADE AIS context integration for rendering.
 *
 * The class automatically manages memory for the objects it contains and provides both
 * individual object access and bulk operations on all objects in the set.
 */
class OcctGeo3DObjectSet
{
public:
    /**
     * @brief Default constructor
     */
    explicit OcctGeo3DObjectSet();

    /**
     * @brief Destructor
     */
    ~OcctGeo3DObjectSet();

    // Object management
    void addObject(const QString& name, OcctGeo3DObject* object);
    bool removeObject(const QString& name);
    void clear();

    // Object access
    OcctGeo3DObject* getObject(const QString& name) const;
    bool contains(const QString& name) const;
    QStringList getObjectNames() const;
    int count() const;
    bool isEmpty() const;

    // Iteration support
    QMap<QString, OcctGeo3DObject*>::iterator begin();
    QMap<QString, OcctGeo3DObject*>::iterator end();
    QMap<QString, OcctGeo3DObject*>::const_iterator begin() const;
    QMap<QString, OcctGeo3DObject*>::const_iterator end() const;
    QMap<QString, OcctGeo3DObject*>::const_iterator constBegin() const;
    QMap<QString, OcctGeo3DObject*>::const_iterator constEnd() const;

    // OpenCASCADE AIS context integration

    /**
     * @brief Displays all objects in the given AIS context
     * @param context The AIS interactive context
     */
    void displayAll(const Handle(AIS_InteractiveContext)& context);

    /**
     * @brief Erases all objects from the given AIS context
     * @param context The AIS interactive context
     */
    void eraseAll(const Handle(AIS_InteractiveContext)& context);

    /**
     * @brief Redisplays all objects in the given AIS context
     * @param context The AIS interactive context
     */
    void redisplayAll(const Handle(AIS_InteractiveContext)& context);

    /**
     * @brief Updates the viewer after changes
     * @param context The AIS interactive context
     */
    void updateViewer(const Handle(AIS_InteractiveContext)& context);

    // Visibility control
    void setAllVisible(bool visible);
    void setObjectVisible(const QString& name, bool visible);

    // Bulk operations
    void setAllDiffuseColor(const QColor& color);
    void setAllScale(float uniformScale);
    void setAllScale(const QVector3D& scale);
    void setAllShowEdges(bool show);
    void setAllEdgeColor(const QColor& color);
    void setAllEdgeWidth(float width);
    void setAllOpacity(float opacity);

    // Direct map access
    const QMap<QString, OcctGeo3DObject*>& getObjectMap() const;

    // JSON Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    // File I/O
    bool saveToFile(const QString& filePath) const;
    bool loadFromFile(const QString& filePath);

    /**
     * @brief Exports all objects to STEP file format
     * @param filename Path to output STEP file (e.g., "output.step")
     * @return true if successful, false otherwise
     */
    bool exportToSTEP(const QString& filename) const;

    /**
     * @brief Gets all shapes combined into a compound
     * @return TopoDS_Compound containing all object shapes
     */
    TopoDS_Compound getAllShapesCompound() const;

private:
    QMap<QString, OcctGeo3DObject*> m_objects;
    bool m_ownsObjects;
};

#endif // OCCTGEO3DOBJECTSET_H

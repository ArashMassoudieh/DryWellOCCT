/**
 * @file occtviewer_qwidget.h
 * @brief OCCT Viewer using QWidget (not QOpenGLWidget) - WORKING VERSION
 *
 * Based on ultra_minimal which works perfectly.
 */

#ifndef OCCTVIEWER_QWIDGET_H
#define OCCTVIEWER_QWIDGET_H

#include <QWidget>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <Aspect_DisplayConnection.hxx>

class OcctGeo3DObjectSet;
class QPushButton;
class QLabel;

/**
 * @class OcctViewerWidget
 * @brief Widget for rendering OpenCASCADE 3D view (QWidget-based, not OpenGL)
 */
class OcctViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OcctViewerWidget(QWidget* parent = nullptr);
    ~OcctViewerWidget();

    Handle(AIS_InteractiveContext) getContext() const;
    Handle(V3d_View) getView() const;
    void fitAll();

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    QPaintEngine* paintEngine() const override;

private:
    void initializeOcct();

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(Aspect_DisplayConnection) m_displayConnection;

    QPoint m_lastPos;
    bool m_isRotating;
    bool m_isPanning;
    bool m_initialized;
};

/**
 * @class OcctViewer
 * @brief Main viewer window for displaying OpenCASCADE objects
 */
class OcctViewer : public QWidget
{
    Q_OBJECT

public:
    explicit OcctViewer(QWidget* parent = nullptr);
    ~OcctViewer();

    void setObjectSet(OcctGeo3DObjectSet* objectSet);
    OcctGeo3DObjectSet* getObjectSet() const;

    Handle(AIS_InteractiveContext) getContext() const;

private slots:
    void showObjects();
    void fitAll();
    void saveImage();
    void exportToSTEP();

private:
    void setupUI();

    OcctGeo3DObjectSet* m_objectSet;
    OcctViewerWidget* m_viewerWidget;
};

#endif // OCCTVIEWER_QWIDGET_H

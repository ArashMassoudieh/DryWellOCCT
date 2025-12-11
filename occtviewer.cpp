/**
 * @file occtviewer_qwidget.cpp
 * @brief OCCT Viewer implementation using QWidget - WORKING VERSION
 */

#include "occtviewer.h"
#include "occtgeo3dobjectset.h"
#include "occtcylinderobject.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include <OpenGl_GraphicDriver.hxx>
#include <TCollection_AsciiString.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#elif defined(__APPLE__)
#include <Cocoa_Window.hxx>
#else
#include <Xw_Window.hxx>
#include <X11/Xlib.h>
#endif

#include <Quantity_Color.hxx>
#include <Graphic3d_Camera.hxx>
#include <AIS_ViewCube.hxx>

// ============================================================================
// OcctViewerWidget Implementation (QWidget-based)
// ============================================================================

OcctViewerWidget::OcctViewerWidget(QWidget* parent)
    : QWidget(parent)
    , m_isRotating(false)
    , m_isPanning(false)
    , m_initialized(false)
{
    // These attributes are CRITICAL for native OCCT rendering
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

OcctViewerWidget::~OcctViewerWidget()
{
}

QPaintEngine* OcctViewerWidget::paintEngine() const
{
    // Return nullptr to disable Qt's painting system
    // OCCT handles all rendering directly
    return nullptr;
}

void OcctViewerWidget::initializeOcct()
{
    if (m_initialized) {
        return;
    }

    // Create display connection
    m_displayConnection = new Aspect_DisplayConnection();

    // Create graphic driver
    Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(m_displayConnection);

    // Create viewer
    m_viewer = new V3d_Viewer(graphicDriver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    // Create view
    m_view = m_viewer->CreateView();

    // Create window handle
#ifdef _WIN32
    Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)winId());
#elif defined(__APPLE__)
    Handle(Cocoa_Window) wind = new Cocoa_Window((NSView*)winId());
#else
    Handle(Xw_Window) wind = new Xw_Window(m_displayConnection, (::Window)winId());
#endif

    m_view->SetWindow(wind);
    if (!wind->IsMapped()) {
        wind->Map();
    }

    // Set background
    m_view->SetBackgroundColor(Quantity_NOC_GRAY30);

    // Create interactive context
    m_context = new AIS_InteractiveContext(m_viewer);
    m_context->SetDisplayMode(AIS_Shaded, Standard_True);

    // Set up camera
    m_view->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);

    // Set initial view orientation
    m_view->SetProj(V3d_XposYposZpos);
    m_view->FitAll(0.01, Standard_False);

    m_initialized = true;
}

void OcctViewerWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (!m_initialized) {
        initializeOcct();
    }
}

void OcctViewerWidget::paintEvent(QPaintEvent* event)
{
    if (!m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OcctViewerWidget::resizeEvent(QResizeEvent* event)
{
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
    }
}

Handle(AIS_InteractiveContext) OcctViewerWidget::getContext() const
{
    return m_context;
}

Handle(V3d_View) OcctViewerWidget::getView() const
{
    return m_view;
}

void OcctViewerWidget::fitAll()
{
    if (!m_view.IsNull()) {
        m_view->FitAll(0.01, Standard_False);
        m_view->ZFitAll();
        update();
    }
}

void OcctViewerWidget::mousePressEvent(QMouseEvent* event)
{
    m_lastPos = event->pos();

    if (event->button() == Qt::LeftButton) {
        m_isRotating = true;
    } else if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
    }
}

void OcctViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isRotating = false;
    } else if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
    }
}

void OcctViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }

    QPoint currPos = event->pos();
    int dx = currPos.x() - m_lastPos.x();
    int dy = currPos.y() - m_lastPos.y();

    if (m_isRotating) {
        m_view->StartRotation(m_lastPos.x(), m_lastPos.y());
        m_view->Rotation(currPos.x(), currPos.y());
        update();
    } else if (m_isPanning) {
        m_view->Pan(dx, -dy);
        update();
    }

    m_lastPos = currPos;
}

void OcctViewerWidget::wheelEvent(QWheelEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }

    int delta = event->angleDelta().y();
    if (delta > 0) {
        m_view->SetZoom(1.1, Standard_True);
    } else {
        m_view->SetZoom(0.9, Standard_True);
    }

    update();
}

// ============================================================================
// OcctViewer Implementation
// ============================================================================

OcctViewer::OcctViewer(QWidget* parent)
    : QWidget(parent)
    , m_objectSet(nullptr)
    , m_viewerWidget(nullptr)
{
    setupUI();
}

OcctViewer::~OcctViewer()
{
    // m_objectSet is owned by caller, don't delete
}

void OcctViewer::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);  // Minimal margins
    mainLayout->setSpacing(2);  // Minimal spacing

    // Create viewer widget
    m_viewerWidget = new OcctViewerWidget(this);
    mainLayout->addWidget(m_viewerWidget);

    // Create compact button panel
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(4, 2, 4, 2);  // Compact margins
    buttonLayout->setSpacing(4);  // Minimal spacing

    QPushButton* showButton = new QPushButton("Show Objects", this);
    showButton->setMaximumWidth(100);  // Limit width
    showButton->setMaximumHeight(24);  // Compact height
    connect(showButton, &QPushButton::clicked, this, &OcctViewer::showObjects);
    buttonLayout->addWidget(showButton);

    QPushButton* fitButton = new QPushButton("Fit All", this);
    fitButton->setMaximumWidth(80);  // Limit width
    fitButton->setMaximumHeight(24);  // Compact height
    connect(fitButton, &QPushButton::clicked, this, &OcctViewer::fitAll);
    buttonLayout->addWidget(fitButton);

    QPushButton* saveButton = new QPushButton("Save Image", this);
    saveButton->setMaximumWidth(90);  // Limit width
    saveButton->setMaximumHeight(24);  // Compact height
    connect(saveButton, &QPushButton::clicked, this, &OcctViewer::saveImage);
    buttonLayout->addWidget(saveButton);

    QPushButton* exportButton = new QPushButton("Export STEP", this);
    exportButton->setMaximumWidth(90);  // Limit width
    exportButton->setMaximumHeight(24);  // Compact height
    connect(exportButton, &QPushButton::clicked, this, &OcctViewer::exportToSTEP);
    buttonLayout->addWidget(exportButton);

    buttonLayout->addStretch();

    QLabel* infoLabel = new QLabel("L: Rotate | M: Pan | Wheel: Zoom", this);
    QFont smallFont = infoLabel->font();
    smallFont.setPointSize(8);  // Smaller font
    infoLabel->setFont(smallFont);
    infoLabel->setMaximumHeight(24);
    buttonLayout->addWidget(infoLabel);

    mainLayout->addLayout(buttonLayout);
}

void OcctViewer::setObjectSet(OcctGeo3DObjectSet* objectSet)
{
    m_objectSet = objectSet;
}

OcctGeo3DObjectSet* OcctViewer::getObjectSet() const
{
    return m_objectSet;
}

Handle(AIS_InteractiveContext) OcctViewer::getContext() const
{
    if (!m_viewerWidget) {
        return Handle(AIS_InteractiveContext)();
    }
    return m_viewerWidget->getContext();
}

void OcctViewer::showObjects()
{
    if (!m_viewerWidget) {
        return;
    }

    Handle(AIS_InteractiveContext) context = m_viewerWidget->getContext();
    if (context.IsNull()) {
        return;
    }

    // Clear existing objects
    context->RemoveAll(Standard_False);

    // Display objects from set
    if (!m_objectSet || m_objectSet->isEmpty()) {
        // Create demo objects
        OcctGeo3DObjectSet* demoSet = new OcctGeo3DObjectSet();

        // Create cylinder 1
        OcctCylinderObject* cylinder1 = new OcctCylinderObject(1.0f, 2.0f);
        cylinder1->setPosition(-3.0f, 0.0f, 0.0f);
        cylinder1->setDiffuseColor(QColor(102, 84, 35));  // Brown
        cylinder1->setShowEdges(true);
        demoSet->addObject("cylinder1", cylinder1);

        // Create cylinder 2
        OcctCylinderObject* cylinder2 = new OcctCylinderObject(0.8f, 3.0f);
        cylinder2->setPosition(0.0f, 0.0f, 0.0f);
        cylinder2->setDiffuseColor(QColor(50, 120, 200));  // Blue
        cylinder2->setShowEdges(true);
        demoSet->addObject("cylinder2", cylinder2);

        // Create cylinder 3
        OcctCylinderObject* cylinder3 = new OcctCylinderObject(0.6f, 1.5f);
        cylinder3->setPosition(3.0f, 0.0f, 0.0f);
        cylinder3->setDiffuseColor(QColor(200, 50, 50));  // Red
        cylinder3->setShowEdges(true);
        cylinder3->setOpacity(0.7f);
        demoSet->addObject("cylinder3", cylinder3);

        // Display demo objects
        demoSet->displayAll(context);

        // Clean up demo set
        delete demoSet;
    } else {
        // Display provided object set
        m_objectSet->displayAll(context);
    }

    // Update viewer and fit all
    context->UpdateCurrentViewer();
    fitAll();
    m_viewerWidget->update();
}

void OcctViewer::fitAll()
{
    if (m_viewerWidget) {
        m_viewerWidget->fitAll();
    }
}

void OcctViewer::saveImage()
{
    if (!m_viewerWidget) {
        return;
    }

    // Get the view
    Handle(V3d_View) view = m_viewerWidget->getView();
    if (view.IsNull()) {
        QMessageBox::warning(this, "Save Image", "No view available to save.");
        return;
    }

    // Open file dialog
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Image",
        QDir::homePath() + "/view.png",
        "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;BMP Image (*.bmp);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    // Ensure file has extension
    if (!fileName.contains('.')) {
        fileName += ".png";
    }

    // Convert QString to TCollection_AsciiString for OCCT
    TCollection_AsciiString occtFileName(fileName.toUtf8().constData());

    // Dump the view to file
    Standard_Boolean result = view->Dump(occtFileName.ToCString());

    if (result) {
        QMessageBox::information(this, "Save Image",
                                 QString("Image saved successfully to:\n%1").arg(fileName));
    } else {
        QMessageBox::critical(this, "Save Image",
                              QString("Failed to save image to:\n%1").arg(fileName));
    }
}

void OcctViewer::exportToSTEP()
{
    if (!m_objectSet || m_objectSet->isEmpty()) {
        QMessageBox::warning(this, "Export STEP", "No objects to export. Please display objects first.");
        return;
    }

    // Open file dialog
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export to STEP",
        QDir::homePath() + "/drywell_system.step",
        "STEP Files (*.step *.stp);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    // Ensure file has extension
    if (!fileName.contains('.')) {
        fileName += ".step";
    }

    // Export using the object set's exportToSTEP method
    bool success = m_objectSet->exportToSTEP(fileName);

    if (success) {
        QMessageBox::information(this, "Export STEP",
                                 QString("Successfully exported to STEP file:\n%1\n\nYou can now open this file in AutoCAD, SolidWorks, FreeCAD, or other CAD programs.").arg(fileName));
    } else {
        QMessageBox::critical(this, "Export STEP",
                              QString("Failed to export to STEP file:\n%1").arg(fileName));
    }
}

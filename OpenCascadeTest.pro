QT += core widgets 3dcore 3drender 3dextras openglwidgets

CONFIG += c++17

SOURCES += main.cpp \
    cylinderobject.cpp \
    faceobject.cpp \
    geo3dobject.cpp \
    geo3dobjectset.cpp \
    occtcylinderobject.cpp \
    occtdrywellsystem.cpp \
    occtgeo3dobject.cpp \
    occtgeo3dobjectset.cpp \
    occttubeobject.cpp \
    occtviewer.cpp \
    qt3dviewer.cpp

TARGET = qt3d_cylinder_viewer

HEADERS += \
    cylinderobject.h \
    faceobject.h \
    geo3dobject.h \
    geo3dobjectset.h \
    occtcylinderobject.h \
    occtdrywellsystem.h \
    occtgeo3dobject.h \
    occtgeo3dobjectset.h \
    occttubeobject.h \
    occtviewer.h \
    qt3dviewer.h

# ========================================
# OpenCASCADE Configuration
# ========================================

# Platform-specific paths
unix:!macx {
    # Linux - adjust paths based on your installation
    OCCT_ROOT = /usr/local
    # Or if installed via package manager:
    # OCCT_ROOT = /usr

    INCLUDEPATH += $$OCCT_ROOT/include/opencascade
    LIBS += -L$$OCCT_ROOT/lib
}

macx {
    # macOS - adjust paths based on your installation
    OCCT_ROOT = /usr/local

    INCLUDEPATH += $$OCCT_ROOT/include/opencascade
    LIBS += -L$$OCCT_ROOT/lib
}

win32 {
    # Windows - adjust paths based on your installation
    OCCT_ROOT = C:/OpenCASCADE-7.7.0

    INCLUDEPATH += $$OCCT_ROOT/inc
    LIBS += -L$$OCCT_ROOT/win64/vc14/lib
}

# Required OpenCASCADE libraries
LIBS += \
    -lTKernel \
    -lTKMath \
    -lTKG2d \
    -lTKG3d \
    -lTKGeomBase \
    -lTKBRep \
    -lTKGeomAlgo \
    -lTKTopAlgo \
    -lTKPrim \
    -lTKBO \
    -lTKBool \
    -lTKShHealing \
    -lTKMesh \
    -lTKService \
    -lTKV3d \
    -lTKOpenGl \
    -lTKFillet \
    -lTKOffset

# For AIS (visualization)
LIBS += \
    -lTKV3d \
    -lTKService

# Additional libraries if using STEP/IGES import/export
# LIBS += -lTKXSBase -lTKSTEP -lTKIGES



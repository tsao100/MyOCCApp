QT += core gui widgets opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# Remove x11extras as it's deprecated and not needed
# Instead use platform-specific includes

TARGET = MyOCCApp
TEMPLATE = app

SOURCES += main.cpp \
           MainWindow.cpp \
           CadView.cpp

HEADERS += MainWindow.h \
           CadView.h

# --- OCCT 8.0.0 paths ---
OCCT_DIR = /usr/local/occt

INCLUDEPATH += $$OCCT_DIR/include/opencascade

LIBS += -L$$OCCT_DIR/lib -Wl,-rpath,$$OCCT_DIR/lib

# Essential OCCT libraries for OCCT 8.0.0
LIBS += -lTKernel \
        -lTKMath \
        -lTKG2d \
        -lTKG3d \
        -lTKGeomBase \
        -lTKBRep \
        -lTKTopAlgo \
        -lTKPrim \
        -lTKV3d \
        -lTKOpenGl \
        -lTKService \
        -lTKLCAF \
        -lTKCAF \
        -lTKCDF \
        -lTKVCAF \
        -lTKMesh \
        -lTKHLR \
        -lTKBO \
        -lTKBool \
        -lTKOffset \
        -lTKFillet \
        -lTKXSBase

# X11 libraries for Linux
unix:!macx {
    LIBS += -lX11 -lXext
}

# Compiler flags
QMAKE_CXXFLAGS += -std=c++11

# Define for Linux builds
unix:!macx {
    DEFINES += __linux__
}

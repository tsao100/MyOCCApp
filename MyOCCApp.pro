QT += core gui widgets opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TEMPLATE = app
TARGET = MyOCCApp

SOURCES += main.cpp \
           MainWindow.cpp \
           CadView.cpp

HEADERS += MainWindow.h \
           CadView.h

# -------------------------
# Platform-specific setup
# -------------------------

unix:!macx {
    message("Building for Ubuntu (Qt5 + OCCT 8.0.0)")

    # --- OCCT 8.0.0 paths ---
    OCCT_DIR = /usr/local/occt

    INCLUDEPATH += $$OCCT_DIR/include/opencascade
    LIBS += -L$$OCCT_DIR/lib -Wl,-rpath,$$OCCT_DIR/lib

    # Essential OCCT libraries
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

    # Link X11 (required for OpenGL context)
    LIBS += -lX11 -lXext

    DEFINES += __linux__
    QMAKE_CXXFLAGS += -std=c++11
}

win32 {
    message("Building for Windows 10 (Qt6 + OCCT 7.8.0 MSVC2022)")

    # --- OCCT 7.8.0 paths ---
    OCC_INC = D:/Git/OCCT/OCCT-install/inc
    OCC_LIB = D:/Git/OCCT/OCCT-install/win64/vc14/lib
    OCC_BIN = D:/Git/OCCT/OCCT-install/win64/vc14/bin

    INCLUDEPATH += $$OCC_INC
    LIBS += -L$$OCC_LIB

    # Add PATH for DLLs during execution
    QMAKE_POST_LINK += $$quote(cmd /C "set PATH=$$OCC_BIN;%PATH% && echo Added OCCT bin to PATH")

    # OCCT core libs (adjusted for 7.8.0)
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

    DEFINES += _WIN32
    QMAKE_CXXFLAGS += /std:c++17
    QMAKE_CXXFLAGS += /wd4005
}

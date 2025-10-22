#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#else
    // Force Qt to use XCB before QApplication is created
    const char* session = std::getenv("XDG_SESSION_TYPE");

    if (session && std::strcmp(session, "wayland") == 0) {
        qDebug("Detected Wayland session → forcing xcb");
        qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    }
#endif
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
